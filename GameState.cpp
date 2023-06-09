#include "GameState.h";

GameState::GameState() {
  begin();
}

void GameState::begin() {
  turn = 1;
  player = X;

  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      grid[x][y] = UNKNOWN;
    }
  }
}

int8_t GameState::processMove(uint8_t x, uint8_t y) {
  // Do nothing if the cell isn't empty
  if (grid[x][y] != UNKNOWN) return INVALID_MOVE;

  grid[x][y] = player;

    // Check for victory
  if (grid[0][y] == grid[1][y] && grid[1][y] == grid[2][y]) {
    return VICTORY;
  }
  if (grid[x][0] == grid[x][1] && grid[x][1] == grid[x][2]) {
    return VICTORY;
  }
  if (x == y) { // We're on the off diagonal
    if (grid[0][0] == grid[1][1] && grid[1][1] == grid[2][2]) {
      return VICTORY;
    }
  }
  if (x + y == 2) { // We're on the main diagonal
    if (grid[0][2] == grid[1][1] && grid[1][1] == grid[2][0]) {
      return VICTORY;
    }
  }

  if (turn == 9) return STALEMATE;

  player = player == X ? O : X;
  turn++;
  return CONTINUE;
}

// Technically incorrect on the first turn, but that's a non-issue
uint8_t GameState::lastPlayer() {
  return player == X ? O : X;
}

uint8_t GameState::currentPlayer() {
  return player;
}