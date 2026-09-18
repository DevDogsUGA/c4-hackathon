/**
 * Connect Four bot starter (Node.js, stdlib only — no npm install needed).
 *
 * Edit ONLY the `chooseMove` function below. Everything else (HTTP server,
 * CORS headers, JSON parsing) is already done for you.
 *
 * Run it:
 *   node server.js
 * Then test it:
 *   curl -X POST http://localhost:8000/move \
 *     -H "Content-Type: application/json" \
 *     -d '{"you":1,"board":[[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0]],"moves":[],"game":{"match_id":"local","game_number":1,"clock_remaining_ms":10000}}'
 */

const http = require("node:http");

// ---------------------------------------------------------------------------
// EDIT THIS FUNCTION. Everything else in this file you can ignore.
//
// board:  an array of 8 columns, each an array of 8 rows.
//         board[col][row] — col 0 is the LEFT column, row 0 is the BOTTOM row.
//         0 = empty, 1 = player 1's piece, 2 = player 2's piece.
// you:    1 or 2 — which player you are this game.
//
// Return: a number 0-7, the column you want to drop a piece into.
//         It MUST be a legal (non-full) column — see `legalMoves` below.
// ---------------------------------------------------------------------------
function chooseMove(board, you) {
  const moves = legalMoves(board);
  return moves[Math.floor(Math.random() * moves.length)];
}

// --- Everything below this line is plumbing. You shouldn't need to edit it. -

function legalMoves(board) {
  // Columns that aren't full yet (top row is still empty).
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
        const { board, you } = request;

        const column = chooseMove(board, you);

        if (!Number.isInteger(column) || !legalMoves(board).includes(column)) {
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
