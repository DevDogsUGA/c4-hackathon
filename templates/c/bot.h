/* Shared declarations for bot.c. Included by server.c; you shouldn't need
 * to edit this file (though it's fine to add helpers to it if you want). */

#ifndef BOT_H
#define BOT_H

#define COLS 8
#define ROWS 8

/* Your move function. See bot.c for the full contract. */
int choose_move(const int board[COLS][ROWS], int you);

/* Fills out_cols with the indices of columns that aren't full yet (top row
 * is still empty) and returns how many there are. out_cols must have room
 * for COLS ints. */
int legal_moves(const int board[COLS][ROWS], int out_cols[COLS]);

#endif
