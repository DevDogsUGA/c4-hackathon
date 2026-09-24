/*
 * HTTP server for your Connect Four bot (C++17).
 *
 * You shouldn't need to edit this file. Write your bot in bot.cpp: this
 * server handles HTTP, CORS headers, and JSON parsing, then calls
 * `choose_move` from bot.cpp once per turn.
 *
 * Vendored dependencies (both MIT-licensed, single-header, see vendor/):
 *   - cpp-httplib v0.57.1 (https://github.com/yhirose/cpp-httplib)
 *   - nlohmann/json v3.12.0 (https://github.com/nlohmann/json)
 *
 * Run it:
 *   make && ./bot
 * Then test it:
 *   curl -X POST http://localhost:8000/move \
 *     -H "Content-Type: application/json" \
 *     -d '{"you":1,"board":[[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0]],"moves":[],"game":{"match_id":"local","game_number":1,"clock_remaining_ms":5000}}'
 */

#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "bot.hpp"
#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

namespace {

// Columns that aren't full yet. Kept separate (and static/anonymous-namespace
// scoped) from bot.cpp so the server's safety check still works however you
// change your bot, even if you delete or rewrite `legal_moves`.
std::vector<int> legal_columns(const std::vector<std::vector<int>>& board) {
  std::vector<int> cols;
  for (size_t col = 0; col < board.size(); ++col) {
    if (board[col].back() == 0) {
      cols.push_back(static_cast<int>(col));
    }
  }
  return cols;
}

void send_json(httplib::Response& res, int status, const json& body) {
  res.status = status;
  res.body = body.dump();
}

}  // namespace

int main() {
  // Exit promptly on `docker stop` / Ctrl-C (as PID 1 in a container, signals
  // without a handler are ignored and Docker waits 10s before killing the bot).
  std::signal(SIGTERM, [](int) { std::_Exit(0); });
  std::signal(SIGINT, [](int) { std::_Exit(0); });

  httplib::Server server;

  // Every response (200, 204, 404, 500, ...) gets these headers. This runs
  // after routing, error handling, and exception handling, so it applies
  // unconditionally -- including to 404s and to exceptions turned into 500s.
  server.set_post_routing_handler([](const httplib::Request&, httplib::Response& res) {
    res.headers.erase("Content-Type");
    res.set_header("Content-Type", "application/json");
    res.headers.erase("Access-Control-Allow-Origin");
    res.set_header("Access-Control-Allow-Origin", "*");
    res.headers.erase("Access-Control-Allow-Methods");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.headers.erase("Access-Control-Allow-Headers");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    // Chrome Private Network Access: lets a hosted https:// page call
    // http://localhost during local testing.
    res.headers.erase("Access-Control-Allow-Private-Network");
    res.set_header("Access-Control-Allow-Private-Network", "true");
  });

  // Belt-and-suspenders: turn any exception that escapes a handler into a
  // 500 instead of a dropped connection. The /move handler below already
  // catches its own exceptions to report a useful message, but this covers
  // anything unexpected.
  server.set_exception_handler([](const httplib::Request&, httplib::Response& res,
                                   const std::exception_ptr& ep) {
    std::string message = "internal error";
    try {
      if (ep) std::rethrow_exception(ep);
    } catch (const std::exception& e) {
      message = e.what();
    } catch (...) {
    }
    send_json(res, 500, {{"error", message}});
  });

  server.Options(".*", [](const httplib::Request&, httplib::Response& res) {
    res.status = 204;
  });

  server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
    send_json(res, 200, {{"status", "ok"}});
  });

  server.Post("/move", [](const httplib::Request& req, httplib::Response& res) {
    try {
      json request = json::parse(req.body);
      std::vector<std::vector<int>> board =
          request.at("board").get<std::vector<std::vector<int>>>();
      int you = request.at("you").get<int>();

      MoveInfo info;
      if (request.contains("moves")) {
        info.moves = request.at("moves").get<std::vector<int>>();
      }
      if (request.contains("game")) {
        const json& game = request.at("game");
        info.match_id = game.value("match_id", "");
        info.game_number = game.value("game_number", 0);
        info.clock_remaining_ms = game.value("clock_remaining_ms", 0LL);
      }

      int column = choose_move(board, you, info);

      std::vector<int> legal = legal_columns(board);
      bool is_legal = std::find(legal.begin(), legal.end(), column) != legal.end();
      if (!is_legal) {
        throw std::runtime_error("choose_move returned an illegal column: " +
                                  std::to_string(column));
      }

      send_json(res, 200, {{"column", column}});
    } catch (const std::exception& e) {
      send_json(res, 500, {{"error", std::string(e.what())}});
    }
  });

  int port = 8000;
  if (const char* env_port = std::getenv("PORT")) {
    port = std::atoi(env_port);
  }

  std::cout << "Bot listening on http://0.0.0.0:" << port << std::endl;
  server.listen("0.0.0.0", port);

  return 0;
}
