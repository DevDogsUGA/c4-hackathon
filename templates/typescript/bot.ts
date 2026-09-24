/**
 * Your Connect Four bot. This is the only file you need to edit.
 *
 * The HTTP server lives in server.ts. It calls `chooseMove` once per turn
 * with the current board and sends your answer back to the arena.
 */

import type { Board, Player } from "./types.ts";

// board:  an array of 8 columns, each an array of 8 rows.
//         board[col][row]: col 0 is the LEFT column, row 0 is the BOTTOM row.
//         0 = empty, 1 = player 1's piece, 2 = player 2's piece.
// you:    1 or 2, which player you are this game.
//
// Return: a number 0-7, the column you want to drop a piece into.
//         It MUST be a legal (non-full) column; see `legalMoves` below.
//
// Every call gets the full game state, so don't rely on variables that
// persist between calls: the arena may restart your bot mid-game.
export function chooseMove(board: Board, you: Player): number {
  const moves = legalMoves(board);
  return moves[Math.floor(Math.random() * moves.length)];
}

export function legalMoves(board: Board): number[] {
  // Columns that aren't full yet (top row is still empty).
  return board
    .map((column, col) => (column[column.length - 1] === 0 ? col : -1))
    .filter((col) => col !== -1);
}
