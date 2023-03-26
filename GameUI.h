#ifndef GAMEUI_H
#define GAMEUI_H

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>

#include <cstdint>
#include <string>

// Determines the length of lines that draw X's
#define X_OFFSET 23

// 240x240 centered on 240x320 screen
#define GRID_START_X 0
#define GRID_START_Y 40
#define GRID_END_X   240
#define GRID_END_Y   280
#define GRID_CELL_WIDTH ((GRID_END_X - GRID_START_X) / 3)
#define GRID_CELL_CENTER_OFFSET (GRID_CELL_WIDTH / 2)

enum Symbol {SYMBOL_X, SYMBOL_O};

class GameUI {
  public:
    GameUI(Adafruit_ILI9341 *tft);
    uint8_t pixelToGridX(uint16_t coord);
    uint8_t pixelToGridY(uint16_t coord);
    void draw(uint8_t x, uint8_t y, int symbol);
    void showMessage(std::string message);
    void showPlayAgainDialog();
    void showTitleScreen();
    void blankScreen();
    void drawGrid();

  private:
    Adafruit_ILI9341 *tft;
    void drawO(uint8_t x, uint8_t y, uint16_t color);
    void drawX(uint8_t x, uint8_t y, uint16_t color);
    uint16_t gridToPixelX(uint8_t coord);
    uint16_t gridToPixelY(uint8_t coord);
};

#endif