// Your Connect Four bot. This is the only file you need to edit (along with
// the function declarations in bot.hpp, if you add helpers of your own).
//
// The HTTP server lives in server.cpp. It calls `choose_move` once per turn
// with the current board and sends your answer back to the arena.
#include "bot.hpp"

#include <random>

int choose_move(const std::vector<std::vector<int>>& board, int you) {
  (void)you;  // the default bot plays randomly and doesn't care who it is

  std::vector<int> moves = legal_moves(board);

  static std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<size_t> dist(0, moves.size() - 1);
  return moves[dist(rng)];
}

std::vector<int> legal_moves(const std::vector<std::vector<int>>& board) {
  // Columns that aren't full yet (top row is still empty).
  std::vector<int> moves;
  for (size_t col = 0; col < board.size(); ++col) {
    if (board[col].back() == 0) {
      moves.push_back(static_cast<int>(col));
    }
  }
  return moves;
}
