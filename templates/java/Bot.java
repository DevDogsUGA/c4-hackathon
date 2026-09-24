import java.util.ArrayList;
import java.util.List;
import java.util.Random;

/**
 * Your Connect Four bot. This is the only file you need to edit.
 *
 * The HTTP server lives in Server.java. It calls `chooseMove` once per turn
 * with the current board and sends your answer back to the arena.
 */
public class Bot {
    private static final Random RANDOM = new Random();

    // board:  an 8x8 array of columns, each an array of 8 rows.
    //         board[col][row]: col 0 is the LEFT column, row 0 is the BOTTOM row.
    //         0 = empty, 1 = player 1's piece, 2 = player 2's piece.
    // you:    1 or 2, which player you are this game.
    //
    // Return: an int 0-7, the column you want to drop a piece into.
    //         It MUST be a legal (non-full) column; see `legalMoves` below.
    //
    // Every call gets the full game state, so don't rely on variables that
    // persist between calls: the arena may restart your bot mid-game.
    public static int chooseMove(int[][] board, int you) {
        List<Integer> moves = legalMoves(board);
        return moves.get(RANDOM.nextInt(moves.size()));
    }

    public static List<Integer> legalMoves(int[][] board) {
        // Columns that aren't full yet (top row is still empty).
        List<Integer> moves = new ArrayList<>();
        for (int col = 0; col < board.length; col++) {
            int[] column = board[col];
            if (column[column.length - 1] == 0) {
                moves.add(col);
            }
        }
        return moves;
    }
}
