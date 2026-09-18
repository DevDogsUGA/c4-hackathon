"""
Connect Four bot starter (Python, stdlib only — no pip install needed).

Edit ONLY the `choose_move` function below. Everything else (HTTP server,
CORS headers, JSON parsing) is already done for you.

Run it:
    python3 server.py
Then test it:
    curl -X POST http://localhost:8000/move \
      -H "Content-Type: application/json" \
      -d '{"you":1,"board":[[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0]],"moves":[],"game":{"match_id":"local","game_number":1,"clock_remaining_ms":10000}}'
"""

import json
import os
import random
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer


# ---------------------------------------------------------------------------
# EDIT THIS FUNCTION. Everything else in this file you can ignore.
#
# board:  a list of 8 columns, each a list of 8 rows.
#         board[col][row] — col 0 is the LEFT column, row 0 is the BOTTOM row.
#         0 = empty, 1 = player 1's piece, 2 = player 2's piece.
# you:    1 or 2 — which player you are this game.
#
# Return: an int 0-7, the column you want to drop a piece into.
#         It MUST be a legal (non-full) column — see `legal_moves` below.
# ---------------------------------------------------------------------------
def choose_move(board, you):
    return random.choice(legal_moves(board))


# --- Everything below this line is plumbing. You shouldn't need to edit it. -

def legal_moves(board):
    """Columns that aren't full yet (top row is still empty)."""
    return [col for col, column in enumerate(board) if column[-1] == 0]


CORS_HEADERS = {
    "Access-Control-Allow-Origin": "*",
    "Access-Control-Allow-Methods": "GET, POST, OPTIONS",
    "Access-Control-Allow-Headers": "Content-Type",
    # Chrome Private Network Access: lets a hosted https:// page call
    # http://localhost during local testing.
    "Access-Control-Allow-Private-Network": "true",
}


class BotHandler(BaseHTTPRequestHandler):
    def _send(self, status, body_dict=None):
        payload = b"" if body_dict is None else json.dumps(body_dict).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(payload)))
        for key, value in CORS_HEADERS.items():
            self.send_header(key, value)
        self.end_headers()
        if payload:
            self.wfile.write(payload)

    def do_OPTIONS(self):
        self._send(204)

    def do_GET(self):
        if self.path == "/health":
            self._send(200, {"status": "ok"})
        else:
            self._send(404)

    def do_POST(self):
        if self.path != "/move":
            self._send(404)
            return

        try:
            length = int(self.headers.get("Content-Length", 0))
            raw = self.rfile.read(length)
            request = json.loads(raw)
            board = request["board"]
            you = request["you"]

            column = choose_move(board, you)

            if not isinstance(column, int) or column not in legal_moves(board):
                raise ValueError(f"choose_move returned an illegal column: {column!r}")

            self._send(200, {"column": column})
        except Exception as exc:  # noqa: BLE001 - surface any bug as a 500, don't hang
            self._send(500, {"error": str(exc)})

    def log_message(self, format, *args):  # noqa: A002 - quiet default logging
        pass


def main():
    port = int(os.environ.get("PORT", 8000))
    server = ThreadingHTTPServer(("0.0.0.0", port), BotHandler)
    print(f"Bot listening on http://0.0.0.0:{port}")
    server.serve_forever()


if __name__ == "__main__":
    main()
