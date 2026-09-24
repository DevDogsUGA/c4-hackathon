/*
 * HTTP server for your Connect Four bot (C, POSIX sockets + vendored cJSON
 * 1.7.19 for JSON; no other dependencies).
 *
 * You shouldn't need to edit this file. Write your bot in bot.c: this server
 * handles HTTP, CORS headers, and JSON parsing, then calls choose_move from
 * bot.c once per turn.
 *
 * This is a minimal, single-connection-at-a-time HTTP/1.1 server written by
 * hand over raw sockets (there's no small, permissively-licensed HTTP
 * library for C worth vendoring). It only implements what the bot contract
 * needs: OPTIONS, GET, and POST with a JSON body.
 *
 * Build and run it:
 *   make
 *   ./bot
 * Then test it:
 *   curl -X POST http://localhost:8000/move \
 *     -H "Content-Type: application/json" \
 *     -d '{"you":1,"board":[[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0]],"moves":[],"game":{"match_id":"local","game_number":1,"clock_remaining_ms":10000}}'
 *
 * Requires a POSIX sockets environment: Linux, macOS, or WSL on Windows.
 */

#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "bot.h"
#include "vendor/cJSON.h"

#define MAX_HEADER_SIZE (64 * 1024)
#define MAX_BODY_SIZE (64 * 1024)
#define RECV_TIMEOUT_SEC 5
#define BACKLOG 16

static const char *CORS_HEADERS =
    "Access-Control-Allow-Origin: *\r\n"
    "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
    "Access-Control-Allow-Headers: Content-Type\r\n"
    /* Chrome Private Network Access: lets a hosted https:// page call
     * http://localhost during local testing. */
    "Access-Control-Allow-Private-Network: true\r\n";

/* Columns that aren't full yet. A static copy separate from bot.c's
 * legal_moves so the server's safety check still works however a student
 * changes (or breaks) their bot. */
static int server_legal_moves(const int board[COLS][ROWS], int out_cols[COLS]) {
    int n = 0;
    for (int col = 0; col < COLS; col++) {
        if (board[col][ROWS - 1] == 0) {
            out_cols[n++] = col;
        }
    }
    return n;
}

static int is_legal_column(const int board[COLS][ROWS], int col) {
    if (col < 0 || col >= COLS) {
        return 0;
    }
    int cols[COLS];
    int n = server_legal_moves(board, cols);
    for (int i = 0; i < n; i++) {
        if (cols[i] == col) {
            return 1;
        }
    }
    return 0;
}

/* Sends `len` bytes from buf, looping over send() to handle partial writes. */
static int send_all(int fd, const char *buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, buf + sent, len - sent, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        sent += (size_t)n;
    }
    return 0;
}

static const char *status_text(int status) {
    switch (status) {
        case 200: return "OK";
        case 204: return "No Content";
        case 404: return "Not Found";
        case 413: return "Payload Too Large";
        case 500: return "Internal Server Error";
        default: return "Unknown";
    }
}

/* Sends a full HTTP response: status line, CORS/JSON headers, and body.
 * body may be NULL for an empty body (204/404 with nothing to say). Every
 * response -- success or failure -- gets the same header set and
 * Connection: close, so the client (including Node's fetch/undici on the
 * arena side) never tries to reuse this socket. */
static void send_response(int fd, int status, const char *body) {
    size_t body_len = body ? strlen(body) : 0;
    char head[1024];
    int head_len = snprintf(
        head, sizeof(head),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "%s"
        "\r\n",
        status, status_text(status), body_len, CORS_HEADERS);

    if (head_len < 0) {
        return;
    }
    if (send_all(fd, head, (size_t)head_len) != 0) {
        return;
    }
    if (body_len > 0) {
        send_all(fd, body, body_len);
    }
}

static void send_error(int fd, int status, const char *message) {
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "error", message);
    char *json = cJSON_PrintUnformatted(obj);
    send_response(fd, status, json);
    free(json);
    cJSON_Delete(obj);
}

/* Reads a full HTTP request off `fd`: headers (up to MAX_HEADER_SIZE, ending
 * at "\r\n\r\n") followed by exactly Content-Length body bytes, which may
 * arrive in the same read as the headers, or trickle in afterward across
 * several reads (or after a delay -- this loop just keeps calling recv until
 * it has everything or the socket times out/closes).
 *
 * On success, fills *method, *path (into caller-owned buffers) and mallocs
 * *body (may be length 0; still NUL-terminated). Returns 0 on success, or a
 * negative code on failure: -1 malformed/short request, -2 request too
 * large. *err is set to a human-readable message on failure. */
static int read_request(int fd, char *method, size_t method_cap, char *path,
                         size_t path_cap, char **body_out, size_t *body_len_out,
                         const char **err) {
    char *header_buf = malloc(MAX_HEADER_SIZE + 1);
    size_t header_have = 0;
    size_t header_end = 0; /* index just past the blank line, once found */
    int found = 0;

    if (!header_buf) {
        *err = "out of memory";
        return -1;
    }

    while (!found) {
        if (header_have >= MAX_HEADER_SIZE) {
            free(header_buf);
            *err = "request headers too large";
            return -2;
        }
        ssize_t n = recv(fd, header_buf + header_have, MAX_HEADER_SIZE - header_have, 0);
        if (n < 0) {
            free(header_buf);
            *err = (errno == EAGAIN || errno == EWOULDBLOCK) ? "timed out waiting for request"
                                                               : "read error";
            return -1;
        }
        if (n == 0) {
            free(header_buf);
            *err = "connection closed before headers were complete";
            return -1;
        }
        header_have += (size_t)n;

        /* Look for the blank line ending the headers. */
        for (size_t i = 0; i + 4 <= header_have; i++) {
            if (memcmp(header_buf + i, "\r\n\r\n", 4) == 0) {
                header_end = i + 4;
                found = 1;
                break;
            }
        }
    }

    header_buf[header_have] = '\0';

    /* Parse the request line: "METHOD /path HTTP/1.1\r\n" */
    char *line_end = strstr(header_buf, "\r\n");
    if (!line_end) {
        free(header_buf);
        *err = "malformed request line";
        return -1;
    }
    char method_buf[16] = {0};
    char path_buf[1024] = {0};
    if (sscanf(header_buf, "%15s %1023s", method_buf, path_buf) != 2) {
        free(header_buf);
        *err = "malformed request line";
        return -1;
    }
    /* method/path are caller-owned buffers of at least these sizes (see the
     * call site); snprintf here just copies without tripping the compiler's
     * strncpy truncation warning. */
    snprintf(method, method_cap, "%s", method_buf);
    snprintf(path, path_cap, "%s", path_buf);

    /* Find Content-Length, case-insensitively, among the header lines. */
    size_t content_length = 0;
    char *cursor = header_buf;
    while (cursor < header_buf + header_end) {
        char *eol = strstr(cursor, "\r\n");
        if (!eol || (size_t)(eol - header_buf) >= header_end) {
            break;
        }
        if (eol > cursor && strncasecmp(cursor, "content-length:", 15) == 0) {
            content_length = (size_t)strtoul(cursor + 15, NULL, 10);
        }
        cursor = eol + 2;
    }

    if (content_length > MAX_BODY_SIZE) {
        free(header_buf);
        *err = "request body too large";
        return -2;
    }

    char *body = malloc(content_length + 1);
    if (!body) {
        free(header_buf);
        *err = "out of memory";
        return -1;
    }

    /* Any body bytes that arrived in the same read as the headers are
     * sitting right after header_end in header_buf. */
    size_t prefetched = header_have - header_end;
    if (prefetched > content_length) {
        prefetched = content_length;
    }
    memcpy(body, header_buf + header_end, prefetched);
    free(header_buf);

    size_t body_have = prefetched;
    while (body_have < content_length) {
        ssize_t n = recv(fd, body + body_have, content_length - body_have, 0);
        if (n < 0) {
            free(body);
            *err = (errno == EAGAIN || errno == EWOULDBLOCK) ? "timed out waiting for request body"
                                                               : "read error";
            return -1;
        }
        if (n == 0) {
            free(body);
            *err = "connection closed before body was complete";
            return -1;
        }
        body_have += (size_t)n;
    }
    body[content_length] = '\0';

    *body_out = body;
    *body_len_out = content_length;
    return 0;
}

/* Parses `board` (must be exactly 8 arrays of 8 ints) and `you` out of the
 * request JSON. Returns 0 on success (filling *board and *you), or -1 with
 * *err set to a message describing the problem. Other request fields
 * (moves, game, ...) are ignored -- the bot doesn't need them here. */
static int parse_move_request(const char *body, size_t body_len, int board[COLS][ROWS],
                               int *you, cJSON **root_out, const char **err) {
    cJSON *root = cJSON_ParseWithLength(body, body_len);
    if (!root) {
        *err = "invalid JSON";
        return -1;
    }

    cJSON *board_json = cJSON_GetObjectItemCaseSensitive(root, "board");
    cJSON *you_json = cJSON_GetObjectItemCaseSensitive(root, "you");

    if (!cJSON_IsArray(board_json) || cJSON_GetArraySize(board_json) != COLS) {
        *err = "board must be an array of 8 columns";
        cJSON_Delete(root);
        return -1;
    }
    if (!cJSON_IsNumber(you_json)) {
        *err = "you must be a number";
        cJSON_Delete(root);
        return -1;
    }

    int col = 0;
    cJSON *column_json;
    cJSON_ArrayForEach(column_json, board_json) {
        if (!cJSON_IsArray(column_json) || cJSON_GetArraySize(column_json) != ROWS) {
            *err = "each board column must be an array of 8 ints";
            cJSON_Delete(root);
            return -1;
        }
        int row = 0;
        cJSON *cell_json;
        cJSON_ArrayForEach(cell_json, column_json) {
            if (!cJSON_IsNumber(cell_json)) {
                *err = "board cells must be numbers";
                cJSON_Delete(root);
                return -1;
            }
            board[col][row] = cell_json->valueint;
            row++;
        }
        col++;
    }

    *you = you_json->valueint;
    *root_out = root;
    return 0;
}

static void handle_move(int fd, const char *body, size_t body_len) {
    int board[COLS][ROWS];
    int you;
    cJSON *root = NULL;
    const char *err = NULL;

    if (parse_move_request(body, body_len, board, &you, &root, &err) != 0) {
        send_error(fd, 500, err);
        return;
    }

    int column = choose_move(board, you);

    if (!is_legal_column(board, column)) {
        char message[128];
        snprintf(message, sizeof(message), "choose_move returned an illegal column: %d", column);
        send_error(fd, 500, message);
        cJSON_Delete(root);
        return;
    }

    cJSON *resp = cJSON_CreateObject();
    cJSON_AddNumberToObject(resp, "column", column);
    char *json = cJSON_PrintUnformatted(resp);
    send_response(fd, 200, json);
    free(json);
    cJSON_Delete(resp);
    cJSON_Delete(root);
}

static void handle_connection(int fd) {
    struct timeval timeout;
    timeout.tv_sec = RECV_TIMEOUT_SEC;
    timeout.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    char method[16];
    char path[1024];
    char *body = NULL;
    size_t body_len = 0;
    const char *err = NULL;

    int rc = read_request(fd, method, sizeof(method), path, sizeof(path), &body, &body_len, &err);
    if (rc != 0) {
        send_error(fd, rc == -2 ? 413 : 500, err ? err : "bad request");
        return;
    }

    if (strcasecmp(method, "OPTIONS") == 0) {
        send_response(fd, 204, NULL);
    } else if (strcmp(method, "GET") == 0 && strcmp(path, "/health") == 0) {
        send_response(fd, 200, "{\"status\":\"ok\"}");
    } else if (strcmp(method, "POST") == 0 && strcmp(path, "/move") == 0) {
        handle_move(fd, body, body_len);
    } else {
        send_response(fd, 404, NULL);
    }

    free(body);
}

/* Exit promptly on `docker stop` / Ctrl-C (as PID 1 in a container, signals
 * without a handler are ignored and Docker waits 10s before killing the bot). */
static void on_stop_signal(int sig) {
    (void)sig;
    _exit(0);
}

int main(void) {
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, on_stop_signal);
    signal(SIGINT, on_stop_signal);
    srand((unsigned)(time(NULL) ^ getpid()));

    const char *port_env = getenv("PORT");
    int port = port_env ? atoi(port_env) : 8000;
    if (port <= 0) {
        port = 8000;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        return 1;
    }

    printf("Bot listening on http://0.0.0.0:%d\n", port);
    fflush(stdout);

    for (;;) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR) {
                continue;
            }
            continue;
        }
        handle_connection(client_fd);
        close(client_fd);
    }

    return 0;
}
