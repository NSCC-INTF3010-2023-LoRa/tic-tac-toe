#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <cstdint>

/* We represent the board as a 2x2 grid, with (0, 0) being the bottom-left cell.
 * Each cell can contain one of three values: SIDE_UNKNOWN, SIDE_X and SIDE_O */

enum Side { UNKNOWN, X, O };
enum Result { INVALID_MOVE, CONTINUE, VICTORY, STALEMATE };

class GameState {
  public:
    GameState();
    int8_t processMove(uint8_t x, uint8_t y);
    uint8_t lastPlayer();
    uint8_t currentPlayer();
    void begin();
  
  private:
    uint8_t player;
    uint8_t grid[3][3];
    uint8_t turn;
    uint8_t getWinner();
    bool isStalemate();
};

#endif