// Shared types for the board and players. Imported by both bot.ts and
// server.ts so the shapes stay in sync.

// 0 = empty, 1 = player 1's piece, 2 = player 2's piece.
export type Cell = 0 | 1 | 2;

// board[col][row]: col 0 is the LEFT column, row 0 is the BOTTOM row.
export type Board = Cell[][];

// Which player you are this game.
export type Player = 1 | 2;

// Extra per-move context, passed as chooseMove's third argument. Safe to
// ignore -- the default bot doesn't use it.
export interface MoveInfo {
  // The full move history so far, as column numbers.
  moves: number[];
  matchId: string;
  gameNumber: number;
  // Your remaining think-time budget for THIS GAME (not this move) -- see
  // the root README's "Time control" section.
  clockRemainingMs: number;
}
