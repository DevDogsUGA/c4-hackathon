"""
HTTP server for your Connect Four bot (Python, stdlib only; no pip install needed).

You shouldn't need to edit this file. Write your bot in bot.py: this server
handles HTTP, CORS headers, and JSON parsing, then calls `choose_move` from
bot.py once per turn.

Run it:
    python3 server.py
Then test it:
    curl -X POST http://localhost:8000/move \
      -H "Content-Type: application/json" \
      -d '{"you":1,"board":[[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0]],"moves":[],"game":{"match_id":"local","game_number":1,"clock_remaining_ms":5000}}'
"""

import json
import os
import signal
import sys
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from types import SimpleNamespace

from bot import choose_move


def legal_columns(board):
    """Columns that aren't full yet. Kept separate from bot.py so the server's
    safety check still works however you change your bot."""
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
            game = request.get("game", {})
            info = SimpleNamespace(
                moves=request.get("moves", []),
                match_id=game.get("match_id"),
                game_number=game.get("game_number"),
                clock_remaining_ms=game.get("clock_remaining_ms"),
            )

            column = choose_move(board, you, info)

            if not isinstance(column, int) or column not in legal_columns(board):
                raise ValueError(f"choose_move returned an illegal column: {column!r}")

            self._send(200, {"column": column})
        except Exception as exc:  # noqa: BLE001 - surface any bug as a 500, don't hang
            self._send(500, {"error": str(exc)})

    def log_message(self, format, *args):  # noqa: A002 - quiet default logging
        pass


def main():
    port = int(os.environ.get("PORT", 8000))
    server = ThreadingHTTPServer(("0.0.0.0", port), BotHandler)
    # Exit promptly on `docker stop` (as PID 1 in a container, SIGTERM is
    # otherwise ignored and Docker waits 10s before killing the bot).
    signal.signal(signal.SIGTERM, lambda *_: sys.exit(0))
    print(f"Bot listening on http://0.0.0.0:{port}", flush=True)
    server.serve_forever()


if __name__ == "__main__":
    main()
