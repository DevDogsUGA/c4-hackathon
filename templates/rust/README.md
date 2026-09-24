# Connect Four bot — Rust starter

Three small crates: [`tiny_http`](https://crates.io/crates/tiny_http) for the
HTTP server, [`serde`](https://crates.io/crates/serde) +
[`serde_json`](https://crates.io/crates/serde_json) for JSON, and
[`rand`](https://crates.io/crates/rand) for the default bot's random move.
No `axum`/`tokio` — keeps the dependency tree (and build time) small.

## Prerequisites

Install Rust via [rustup](https://rustup.rs/) if you don't already have it.

## Quick start

```bash
cargo run --release
```

The first build downloads and compiles the crates above (and their
dependencies), so it'll take a minute; after that, rebuilds are fast. Use
`--release`: an unoptimized (`cargo run` without `--release`) build is much
slower, and think-time matters once your bot is searching the board instead
of picking randomly.

The server listens on port 8000 (or `$PORT` if set). In another terminal:

```bash
curl -X POST http://localhost:8000/move \
  -H "Content-Type: application/json" \
  -d '{"you":1,"board":[[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0]],"moves":[],"game":{"match_id":"local","game_number":1,"clock_remaining_ms":10000}}'
```

You should get back something like `{"column": 4}`.

## What to edit

Open `src/bot.rs` and change the `choose_move(board, you)` function. That's
it. `src/server.rs` is the HTTP server (CORS headers, JSON parsing, error
handling, panic recovery); it calls your function once per turn, and you
shouldn't need to touch it.

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
