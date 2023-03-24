#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <cstdint>

/* We represent the board as a 2x2 grid, with (0, 0) being the bottom-left cell.
 * Each cell can contain one of three values: SIDE_UNKNOWN, SIDE_X and SIDE_O */

#define SIDE_UNKNOWN 0
#define SIDE_X 1
#define SIDE_O 2

#define RESULT_INVALID_MOVE -1
#define RESULT_CONTINUE 0
#define RESULT_VICTORY 1
#define RESULT_STALEMATE 2

class GameState {
  public:
    GameState();
    int8_t processMove(uint8_t x, uint8_t y);
    uint8_t lastPlayer();
    uint8_t currentPlayer();
  
  private:
    uint8_t player;
    uint8_t grid[3][3];
    uint8_t turn;
    void begin();
    uint8_t getWinner();
    bool isStalemate();
};

#endif