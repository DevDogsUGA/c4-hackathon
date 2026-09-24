/**
 * HTTP server for your Connect Four bot (Node.js stdlib only; tsx runs the
 * TypeScript directly, no build step needed).
 *
 * You shouldn't need to edit this file. Write your bot in bot.ts: this server
 * handles HTTP, CORS headers, and JSON parsing, then calls `chooseMove` from
 * bot.ts once per turn.
 *
 * Run it:
 *   npm start
 * Then test it:
 *   curl -X POST http://localhost:8000/move \
 *     -H "Content-Type: application/json" \
 *     -d '{"you":1,"board":[[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0]],"moves":[],"game":{"match_id":"local","game_number":1,"clock_remaining_ms":5000}}'
 */

import http from "node:http";
import { chooseMove } from "./bot.ts";
import type { Board, Cell, MoveInfo } from "./types.ts";

// Columns that aren't full yet. Kept separate from bot.ts so the server's
// safety check still works however you change your bot.
function legalColumns(board: Board): number[] {
  return board
    .map((column, col) => (column[column.length - 1] === 0 ? col : -1))
    .filter((col) => col !== -1);
}

// TypeScript types don't exist at runtime and don't check JSON coming off
// the wire, so we validate the request shape by hand before trusting it.
function isCell(value: unknown): value is Cell {
  return value === 0 || value === 1 || value === 2;
}

function isBoard(value: unknown): value is Board {
  return (
    Array.isArray(value) &&
    value.every((column) => Array.isArray(column) && column.every(isCell))
  );
}

interface MoveRequest {
  board: Board;
  you: 1 | 2;
  moves?: number[];
  game?: { match_id?: string; game_number?: number; clock_remaining_ms?: number };
}

function isRequest(value: unknown): value is MoveRequest {
  if (typeof value !== "object" || value === null) return false;
  const request = value as Record<string, unknown>;
  return (
    isBoard(request.board) &&
    (request.you === 1 || request.you === 2)
  );
}

// Builds the third argument to chooseMove from the raw request fields.
function buildMoveInfo(request: MoveRequest): MoveInfo {
  return {
    moves: request.moves ?? [],
    matchId: request.game?.match_id ?? "",
    gameNumber: request.game?.game_number ?? 0,
    clockRemainingMs: request.game?.clock_remaining_ms ?? 0,
  };
}

const CORS_HEADERS = {
  "Access-Control-Allow-Origin": "*",
  "Access-Control-Allow-Methods": "GET, POST, OPTIONS",
  "Access-Control-Allow-Headers": "Content-Type",
  // Chrome Private Network Access: lets a hosted https:// page call
  // http://localhost during local testing.
  "Access-Control-Allow-Private-Network": "true",
};

function send(res: http.ServerResponse, status: number, bodyObj?: unknown): void {
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
        const parsed: unknown = JSON.parse(body);

        if (!isRequest(parsed)) {
          throw new Error("request must have a `board` and a `you` of 1 or 2");
        }

        const { board, you } = parsed;
        const info = buildMoveInfo(parsed);
        const column = chooseMove(board, you, info);

        if (!Number.isInteger(column) || !legalColumns(board).includes(column)) {
          throw new Error(`chooseMove returned an illegal column: ${column}`);
        }

        send(res, 200, { column });
      } catch (err) {
        send(res, 500, { error: String(err instanceof Error ? err.message : err) });
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
for (const signal of ["SIGTERM", "SIGINT"] as const) {
  process.on(signal, () => process.exit(0));
}
