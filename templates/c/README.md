# Connect Four bot — C starter

Zero package-manager dependencies. The only third-party code is
[cJSON](https://github.com/DaveGamble/cJSON) (MIT licensed) v1.7.19, vendored
directly in `vendor/` — nothing to fetch or install.

Uses POSIX sockets, so it runs on Linux, macOS, or WSL. Windows users:
build and run it under WSL, or just use Docker.

## Prerequisites

A C compiler (gcc or clang) and `make`.

## Quick start

```bash
make
./bot
```

The server listens on port 8000 (or `$PORT` if set). In another terminal:

```bash
curl -X POST http://localhost:8000/move \
  -H "Content-Type: application/json" \
  -d '{"you":1,"board":[[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0]],"moves":[],"game":{"match_id":"local","game_number":1,"clock_remaining_ms":10000}}'
```

You should get back something like `{"column": 4}`.

`make run` builds and runs it in one step. `make clean` removes the build
output.

## What to edit

Open `bot.c` and change the `choose_move(board, you)` function. That's it.
`server.c` is the HTTP server (raw POSIX sockets, CORS headers, JSON parsing
via cJSON); it calls your function once per turn, and you shouldn't need to
touch it.

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

`.github/workflows/smoke.yml` builds your Docker image, starts it, POSTs the
same sample request as above, and fails the build if the response isn't a
legal column. Push to GitHub and check the Actions tab.
