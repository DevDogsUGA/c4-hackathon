//! Your Connect Four bot. This is the only file you need to edit.
//!
//! The HTTP server lives in server.rs. It calls `choose_move` once per turn
//! with the current board and sends your answer back to the arena.

use rand::seq::SliceRandom;

/// Extra per-move context, passed as `choose_move`'s third argument. Safe
/// to ignore -- the default bot doesn't use it.
///
/// `#[allow(dead_code)]`: this is a binary crate, so rustc can't see that
/// server.rs constructs every field -- it only complains that the default
/// bot doesn't read them back out. Once your bot reads `info.something`,
/// this warning goes away on its own; the allow just keeps a fresh,
/// unedited clone of this template warning-free.
#[allow(dead_code)]
pub struct MoveInfo {
    /// The full move history so far, as column numbers.
    pub moves: Vec<usize>,
    pub match_id: String,
    pub game_number: u32,
    /// Your remaining think-time budget for THIS GAME (not this move) --
    /// see the root README's "Time control" section.
    pub clock_remaining_ms: u64,
}

// board:  a list of 8 columns, each a list of 8 rows.
//         board[col][row]: col 0 is the LEFT column, row 0 is the BOTTOM row.
//         0 = empty, 1 = player 1's piece, 2 = player 2's piece.
// you:    1 or 2, which player you are this game.
// info:   extra context, safe to ignore. See `MoveInfo` above.
//
// Return: a usize 0-7, the column you want to drop a piece into.
//         It MUST be a legal (non-full) column; see `legal_moves` below.
//
// Every call gets the full game state, so don't rely on variables that
// persist between calls: the arena may restart your bot mid-game.
pub fn choose_move(board: &[Vec<u8>], _you: u8, _info: &MoveInfo) -> usize {
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
