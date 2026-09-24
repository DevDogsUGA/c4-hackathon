/* Shared declarations for bot.c. Included by server.c; you shouldn't need
 * to edit this file (though it's fine to add helpers to it if you want). */

#ifndef BOT_H
#define BOT_H

#define COLS 8
#define ROWS 8

/* Extra per-move context, passed as choose_move's third argument. Safe to
 * ignore -- the default bot doesn't use it.
 *   moves               the full move history so far (move_count columns)
 *   clock_remaining_ms  your remaining think-time budget for THIS GAME
 *                       (not this move) -- see the root README's
 *                       "Time control" section
 * Owned by the server; valid only for the duration of the choose_move call
 * that receives it. Don't store the pointer past that call. */
typedef struct {
    const int *moves;
    int move_count;
    const char *match_id;
    int game_number;
    long long clock_remaining_ms;
} MoveInfo;

/* Your move function. See bot.c for the full contract. */
int choose_move(const int board[COLS][ROWS], int you, const MoveInfo *info);

/* Fills out_cols with the indices of columns that aren't full yet (top row
 * is still empty) and returns how many there are. out_cols must have room
 * for COLS ints. */
int legal_moves(const int board[COLS][ROWS], int out_cols[COLS]);

#endif
