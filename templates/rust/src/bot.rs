//! Your Connect Four bot. This is the only file you need to edit.
//!
//! The HTTP server lives in server.rs. It calls `choose_move` once per turn
//! with the current board and sends your answer back to the arena.

use rand::seq::SliceRandom;

// board:  a list of 8 columns, each a list of 8 rows.
//         board[col][row]: col 0 is the LEFT column, row 0 is the BOTTOM row.
//         0 = empty, 1 = player 1's piece, 2 = player 2's piece.
// you:    1 or 2, which player you are this game.
//
// Return: a usize 0-7, the column you want to drop a piece into.
//         It MUST be a legal (non-full) column; see `legal_moves` below.
//
// Every call gets the full game state, so don't rely on variables that
// persist between calls: the arena may restart your bot mid-game.
pub fn choose_move(board: &[Vec<u8>], _you: u8) -> usize {
    let moves = legal_moves(board);
    *moves.choose(&mut rand::thread_rng()).unwrap()
}

/// Columns that aren't full yet (top row is still empty).
pub fn legal_moves(board: &[Vec<u8>]) -> Vec<usize> {
    board
        .iter()
        .enumerate()
        .filter(|(_, column)| *column.last().unwrap() == 0)
        .map(|(col, _)| col)
        .collect()
}
