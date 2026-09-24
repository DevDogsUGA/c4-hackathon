// Your Connect Four bot. This is the only file you need to edit.
//
// The HTTP server lives in server.go. It calls chooseMove once per turn
// with the current board and sends your answer back to the arena.
package main

import "math/rand"

// chooseMove picks your next move.
//
// board is a slice of 8 columns, each a slice of 8 rows: board[col][row].
// Column 0 is the LEFT column, row 0 is the BOTTOM row.
// 0 = empty, 1 = player 1's piece, 2 = player 2's piece.
//
// you is 1 or 2: which player you are this game.
//
// Return an int 0-7, the column you want to drop a piece into. It MUST be a
// legal (non-full) column; see legalMoves below.
//
// Every call gets the full game state, so don't rely on variables that
// persist between calls: the arena may restart your bot mid-game.
func chooseMove(board [][]int, you int) int {
	moves := legalMoves(board)
	return moves[rand.Intn(len(moves))]
}

// legalMoves returns the columns that aren't full yet (top row is still empty).
func legalMoves(board [][]int) []int {
	var moves []int
	for col, column := range board {
		if column[len(column)-1] == 0 {
			moves = append(moves, col)
		}
	}
	return moves
}
