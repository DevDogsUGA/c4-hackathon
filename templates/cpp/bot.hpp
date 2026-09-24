// Declarations for your Connect Four bot. This is the only file pair you need
// to edit (this header + bot.cpp).
//
// The HTTP server lives in server.cpp. It calls `choose_move` once per turn
// with the current board and sends your answer back to the arena.
#pragma once

#include <vector>

// board:  a vector of 8 columns, each a vector of 8 rows.
//         board[col][row]: col 0 is the LEFT column, row 0 is the BOTTOM row.
//         0 = empty, 1 = player 1's piece, 2 = player 2's piece.
// you:    1 or 2, which player you are this game.
//
// Return: an int 0-7, the column you want to drop a piece into.
//         It MUST be a legal (non-full) column; see `legal_moves` below.
//
// Every call gets the full game state, so don't rely on variables that
// persist between calls: the arena may restart your bot mid-game.
int choose_move(const std::vector<std::vector<int>>& board, int you);

// Columns that aren't full yet (top row is still empty).
std::vector<int> legal_moves(const std::vector<std::vector<int>>& board);
