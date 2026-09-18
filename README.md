# ACM UGA Connect Four Hackathon

You have **60 minutes** to build a bot that plays Connect Four on an **8×8 board**
(four in a row to win — same rules as classic Connect Four, just a bigger board).
Then the bots play each other in a live tournament.

## 1. Get started

Pick a starter template and copy it into your own repo:

- [`templates/python/`](templates/python/) — Python, standard library only
- [`templates/node/`](templates/node/) — Node.js, standard library only

Both are **zero-dependency**: nothing to `pip install` or `npm install`. Each
template has its own README with a quick-start command. The only thing you need
to write is one function: `choose_move(board, you)` (Python) or
`chooseMove(board, you)` (Node). Everything else — the HTTP server, CORS
headers, JSON parsing — is already done for you.

## 2. The bot contract

Your bot is a stateless HTTP server. The arena sends it a `POST /move` request
before every move you make, and expects a column number back.

### `POST /move`

Request:

```json
{
  "you": 1,
  "board": [[0,0,0,0,0,0,0,0], [0,0,0,0,0,0,0,0], "... 8 columns total"],
  "moves": [3, 4, 3],
  "game": {
    "match_id": "rr-007",
    "game_number": 2,
    "clock_remaining_ms": 8420
  }
}
```

Response (must be `200`):

```json
{ "column": 4 }
```

- `you` — `1` or `2`. Which player you are this game.
- `board` — `board[col][row]`. **Column 0 is the left edge. Row 0 is the
  bottom row.** `0` = empty, `1` = player 1's piece, `2` = player 2's piece.
- `moves` — the full move history so far, as a list of column numbers, in
  case that's a more convenient format for your logic.
- `game.clock_remaining_ms` — how much of your think-time budget you have
  left this game (see [Time control](#3-time-control) below).
- Your response's `column` must be `0`–`7` and must not already be full
  (i.e. it must be a **legal move**).

### Board encoding, visually

```
row 7  .  .  .  .  .  .  .  .   <- top
row 6  .  .  .  .  .  .  .  .
row 5  .  .  .  .  .  .  .  .
row 4  .  .  .  .  .  .  .  .
row 3  .  .  .  .  .  .  .  .
row 2  .  .  .  .  .  .  .  .
row 1  .  .  .  .  .  .  .  .
row 0  .  .  .  .  .  .  .  .   <- bottom (pieces fall here first)
      col0 col1 col2 col3 col4 col5 col6 col7
```

`board[3]` is the array of 8 values in column 3, bottom to top. A column is
full — and therefore illegal to play — when its top slot (`board[col][7]`)
is non-zero.

### `GET /health`

Must return `200`. The arena polls this to know your bot is ready before a
match starts.

### CORS

Your bot must send these headers on every response (the templates already do
this):

```
Access-Control-Allow-Origin: *
Access-Control-Allow-Methods: GET, POST, OPTIONS
Access-Control-Allow-Headers: Content-Type
Access-Control-Allow-Private-Network: true
```

The last one matters if you test your bot against the hosted testground page:
Chrome blocks a public `https://` page from calling `http://localhost` unless
you send it (this is the browser's "Private Network Access" check).

## 3. Time control

Each bot gets a **10-second chess clock per game**, shared across all your
`/move` calls in that game — not 10 seconds per move. The arena measures wall
clock from when it sends the request to when it fully receives your response.

| Event | Consequence |
|---|---|
| Your clock hits zero | You forfeit that **game** |
| Invalid move (full/out-of-range column, malformed JSON, non-200 response) | You forfeit that **game** |
| Your container crashes (including OOM) | The arena restarts it and re-sends the move — but the restart time is billed to your clock. Crash-looping will drain your clock. |
| Startup at the beginning of a match | Free — doesn't count against your clock (the arena waits on `/health`) |
| Not healthy within 30s of match start | You forfeit the **match** |
| Board fills up (draw) | Counts for neither player |

**Practical takeaway:** your bot needs to be fast and it needs to be robust.
A crash is not free — it costs you clock time on the restart.

## 4. Test your bot

Run it locally (see your template's README), then either:

- `curl` it directly, as shown in each template's README, or
- Use the hosted browser testground: **`<TESTGROUND_URL_PLACEHOLDER>`** — enter
  `http://localhost:8000` (or wherever your bot is listening) and watch it
  play live against practice bots of varying strength (`random`, `greedy`,
  `minimax`). The testground shows you the raw request/response JSON of the
  last move, which is handy for debugging.

## 5. Submit

1. Push your bot to your own **public GitHub repo**, including the
   `Dockerfile` from the template (the arena builds and runs your bot in
   Docker — test it locally with `docker build` / `docker run` before you
   submit).
2. Check the **Actions** tab on your repo — the template ships a CI smoke
   test that builds your image, starts it, and POSTs a sample `/move`
   request. Green means the arena will be able to run your bot.
3. Submit your team name and repo URL via the **submission form**:
   **`<SUBMISSION_FORM_URL_PLACEHOLDER>`**

Submissions are re-validated at **T-30, T-15, and T-5 minutes** before the
tournament starts — failures are announced to the room, so you'll have a
chance to fix a broken submission. Bring a USB drive as a fallback in case
your GitHub push fails at the last minute.

## 6. Tournament format

- Round-robin: everyone plays everyone, **best of 3** games per match,
  alternating who moves first (a coin flip decides who goes first in game 1).
- Round-robin standings seed a single-elimination bracket.
- Everything you see on the projector is a replay of already-finished games —
  the actual matches run headless, instantly, beforehand.

Good luck — write `choose_move`, test it, ship it.
