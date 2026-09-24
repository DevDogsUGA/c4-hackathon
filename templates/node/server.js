/**
 * HTTP server for your Connect Four bot (Node.js, stdlib only; no npm install needed).
 *
 * You shouldn't need to edit this file. Write your bot in bot.js: this server
 * handles HTTP, CORS headers, and JSON parsing, then calls `chooseMove` from
 * bot.js once per turn.
 *
 * Run it:
 *   node server.js
 * Then test it:
 *   curl -X POST http://localhost:8000/move \
 *     -H "Content-Type: application/json" \
 *     -d '{"you":1,"board":[[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0]],"moves":[],"game":{"match_id":"local","game_number":1,"clock_remaining_ms":5000}}'
 */

const http = require("node:http");
const { chooseMove } = require("./bot.js");

// Columns that aren't full yet. Kept separate from bot.js so the server's
// safety check still works however you change your bot.
function legalColumns(board) {
  return board
    .map((column, col) => (column[column.length - 1] === 0 ? col : -1))
    .filter((col) => col !== -1);
}

const CORS_HEADERS = {
  "Access-Control-Allow-Origin": "*",
  "Access-Control-Allow-Methods": "GET, POST, OPTIONS",
  "Access-Control-Allow-Headers": "Content-Type",
  // Chrome Private Network Access: lets a hosted https:// page call
  // http://localhost during local testing.
  "Access-Control-Allow-Private-Network": "true",
};

function send(res, status, bodyObj) {
  const payload = bodyObj === undefined ? "" : JSON.stringify(bodyObj);
  res.writeHead(status, {
    "Content-Type": "application/json",
    "Content-Length": Buffer.byteLength(payload),
    ...CORS_HEADERS,
  });
  res.end(payload);
}

const server = http.createServer((req, res) => {
  if (req.method === "OPTIONS") {
    send(res, 204);
    return;
  }

  if (req.method === "GET" && req.url === "/health") {
    send(res, 200, { status: "ok" });
    return;
  }

  if (req.method === "POST" && req.url === "/move") {
    let body = "";
    req.on("data", (chunk) => {
      body += chunk;
    });
    req.on("end", () => {
      try {
        const request = JSON.parse(body);
        const { board, you, moves, game } = request;
        const info = {
          moves: moves || [],
          matchId: game && game.match_id,
          gameNumber: game && game.game_number,
          clockRemainingMs: game && game.clock_remaining_ms,
        };

        const column = chooseMove(board, you, info);

        if (!Number.isInteger(column) || !legalColumns(board).includes(column)) {
          throw new Error(`chooseMove returned an illegal column: ${column}`);
        }

        send(res, 200, { column });
      } catch (err) {
        send(res, 500, { error: String(err && err.message ? err.message : err) });
      }
    });
    return;
  }

  send(res, 404);
});

const port = Number(process.env.PORT) || 8000;
server.listen(port, "0.0.0.0", () => {
  console.log(`Bot listening on http://0.0.0.0:${port}`);
});

// Exit promptly on `docker stop` / Ctrl-C (as PID 1 in a container, Node
// ignores these signals by default and Docker waits 10s before killing it).
for (const signal of ["SIGTERM", "SIGINT"]) {
  process.on(signal, () => process.exit(0));
}
