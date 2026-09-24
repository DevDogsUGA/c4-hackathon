# Connect Four bot — C++ starter

Two vendored MIT-licensed single-header libraries, no package manager needed:
[cpp-httplib](https://github.com/yhirose/cpp-httplib) v0.57.1 and
[nlohmann/json](https://github.com/nlohmann/json) v3.12.0, both checked into
`vendor/`.

## Prerequisites

A C++17 compiler and `make` (e.g. `g++` or `clang++`, plus `pthread` support).
On Windows, use WSL or Docker — this template hasn't been tested with MSVC or
a native Windows toolchain.

## Quick start

```bash
make
./bot
```

The server listens on port 8000 (or `$PORT` if set). In another terminal:

```bash
curl -X POST http://localhost:8000/move \
  -H "Content-Type: application/json" \
  -d '{"you":1,"board":[[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0]],"moves":[],"game":{"match_id":"local","game_number":1,"clock_remaining_ms":5000}}'
```

You should get back something like `{"column": 4}`.

`make run` builds and runs in one step. `make clean` removes the built
binary.

## What to edit

Open `bot.cpp` (and its declarations in `bot.hpp`) and change the
`choose_move(board, you, info)` function. That's it. `server.cpp` is the HTTP
server (CORS headers, JSON parsing, error handling); it calls your function
once per turn, and you shouldn't need to touch it.

- `board[col][row]` — column 0 is the left edge, row 0 is the bottom.
- `0` = empty, `1` = player 1's piece, `2` = player 2's piece.
- Return the column (0–7) you want to drop into. It must not be full.
- `info` (the `MoveInfo` struct, defined in `bot.hpp`) is extra context you
  can ignore: `moves` (the move history), `match_id`, `game_number`, and
  `clock_remaining_ms` (your remaining think-time budget for **this game**,
  not this move).

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
