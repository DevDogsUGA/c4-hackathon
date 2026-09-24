/**
 * Your Connect Four bot. This is the only file you need to edit.
 *
 * The HTTP server lives in server.js. It calls `chooseMove` once per turn
 * with the current board and sends your answer back to the arena.
 */

// board:  an array of 8 columns, each an array of 8 rows.
//         board[col][row]: col 0 is the LEFT column, row 0 is the BOTTOM row.
//         0 = empty, 1 = player 1's piece, 2 = player 2's piece.
// you:    1 or 2, which player you are this game.
// info:   extra context, safe to ignore. An object with:
//           .moves               the full move history (array of columns)
//           .matchId             string
//           .gameNumber          number
//           .clockRemainingMs    your remaining think-time budget for THIS
//                                 GAME (not this move) -- see the root README
//
// Return: a number 0-7, the column you want to drop a piece into.
//         It MUST be a legal (non-full) column; see `legalMoves` below.
//
// Every call gets the full game state, so don't rely on variables that
// persist between calls: the arena may restart your bot mid-game.
function chooseMove(board, you, info) {
  const moves = legalMoves(board);
  return moves[Math.floor(Math.random() * moves.length)];
}

function legalMoves(board) {
  // Columns that aren't full yet (top row is still empty).
  return board
    .map((column, col) => (column[column.length - 1] === 0 ? col : -1))
    .filter((col) => col !== -1);
}

module.exports = { chooseMove };
