// Your Connect Four bot. This is the only file you need to edit.
//
// The HTTP server lives in Server.cs. It calls `Bot.ChooseMove` once per turn
// with the current board and sends your answer back to the arena.

namespace C4Bot;

// Extra per-move context, passed as ChooseMove's third argument. Safe to
// ignore -- the default bot doesn't use it.
//   Moves             the full move history so far, as column numbers
//   ClockRemainingMs  your remaining think-time budget for THIS GAME (not
//                     this move) -- see the root README's "Time control"
//                     section
public record MoveInfo(List<int> Moves, string MatchId, int GameNumber, long ClockRemainingMs);

public static class Bot
{
    // board:  an array of 8 columns, each an array of 8 rows.
    //         board[col][row]: col 0 is the LEFT column, row 0 is the BOTTOM row.
    //         0 = empty, 1 = player 1's piece, 2 = player 2's piece.
    // you:    1 or 2, which player you are this game.
    // info:   extra context, safe to ignore. See MoveInfo above.
    //
    // Return: an int 0-7, the column you want to drop a piece into.
    //         It MUST be a legal (non-full) column; see `LegalMoves` below.
    //
    // Every call gets the full game state, so don't rely on state persisting
    // between calls (e.g. static fields): the arena may restart your bot
    // mid-game.
    public static int ChooseMove(int[][] board, int you, MoveInfo info)
    {
        var moves = LegalMoves(board);
        return moves[Random.Shared.Next(moves.Count)];
    }

    // Columns that aren't full yet (top row is still empty).
    public static List<int> LegalMoves(int[][] board)
    {
        var moves = new List<int>();
        for (var col = 0; col < board.Length; col++)
        {
            if (board[col][^1] == 0)
            {
                moves.Add(col);
            }
        }
        return moves;
    }
}
