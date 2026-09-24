# Connect Four bot — TypeScript starter

Runs your TypeScript directly with [`tsx`](https://tsx.is/) — no build step.

## Quick start

```bash
npm install
npm start
```

The server listens on port 8000 (or `$PORT` if set). In another terminal:

```bash
curl -X POST http://localhost:8000/move \
  -H "Content-Type: application/json" \
  -d '{"you":1,"board":[[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0]],"moves":[],"game":{"match_id":"local","game_number":1,"clock_remaining_ms":10000}}'
```

You should get back something like `{"column": 4}`.

`npm run typecheck` to check types (tsx runs your code without type checking).
On Node 23.6+ you can also run `node server.ts` directly.

## What to edit

Open `bot.ts` and change the `chooseMove(board, you)` function. That's it.
`server.ts` is the HTTP server (CORS headers, JSON parsing, error handling); it
calls your function once per turn, and you shouldn't need to touch it.

- `board[col][row]` — column 0 is the left edge, row 0 is the bottom.
- `0` = empty, `1` = player 1's piece, `2` = player 2's piece.
- Return the column (0–7) you want to drop into. It must not be full.

See the root [`README.md`](../../README.md) for the full contract (request/response
shape, clock rules, failure rules).

## Docker

This is how the arena will actually run your bot, so it's worth testing:

```bash
docker build -t my-bot .
docker run -p 8000:8000 my-bot
```

## CI smoke test

`.github/workflows/smoke.yml` type-checks your bot, then builds your Docker
image, starts it, POSTs the same sample request as above, and fails the build
if the response isn't a legal column. Push to GitHub and check the Actions
tab.
