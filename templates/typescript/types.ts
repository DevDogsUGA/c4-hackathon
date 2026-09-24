// Shared types for the board and players. Imported by both bot.ts and
// server.ts so the shapes stay in sync.

// 0 = empty, 1 = player 1's piece, 2 = player 2's piece.
export type Cell = 0 | 1 | 2;

// board[col][row]: col 0 is the LEFT column, row 0 is the BOTTOM row.
export type Board = Cell[][];

// Which player you are this game.
export type Player = 1 | 2;
