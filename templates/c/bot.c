/*
 * Your Connect Four bot. This is the only file you need to edit.
 *
 * The HTTP server lives in server.c. It calls choose_move once per turn
 * with the current board and sends your answer back to the arena.
 */

#include <stdlib.h>

#include "bot.h"

/* board: an 8x8 array of columns, each an array of 8 rows.
 *        board[col][row]: col 0 is the LEFT column, row 0 is the BOTTOM row.
 *        0 = empty, 1 = player 1's piece, 2 = player 2's piece.
 * you:   1 or 2, which player you are this game.
 *
 * info:  extra context, safe to ignore. See MoveInfo in bot.h.
 *
 * Return: an int 0-7, the column you want to drop a piece into.
 *         It MUST be a legal (non-full) column; see legal_moves below.
 *
 * Every call gets the full game state, so don't rely on state persisting
 * between calls: no static or global variables to remember what happened
 * last turn -- the arena may restart your bot mid-game.
 */
int choose_move(const int board[COLS][ROWS], int you, const MoveInfo *info) {
    (void)you;
    (void)info;

    int moves[COLS];
    int n = legal_moves(board, moves);

    return moves[rand() % n];
}

/* Columns that aren't full yet (top row is still empty). */
int legal_moves(const int board[COLS][ROWS], int out_cols[COLS]) {
    int n = 0;
    for (int col = 0; col < COLS; col++) {
        if (board[col][ROWS - 1] == 0) {
            out_cols[n++] = col;
        }
    }
    return n;
}
