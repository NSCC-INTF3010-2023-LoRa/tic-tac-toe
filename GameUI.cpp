#include "GameUI.h";

GameUI::GameUI(Adafruit_ILI9341 *tft) {
  this->tft = tft;

  tft->begin();
  tft->fillScreen(ILI9341_WHITE);
  tft->setTextColor(ILI9341_BLUE, ILI9341_WHITE);
  tft->setTextSize(2);
  tft->setTextWrap(false);

  // Display the tic-tac-toe grid. We draw multiple lines to
  // display thicker lines
  for (int i = -1; i <= 1; i++) {
    tft->drawLine(80 + i, 40, 80 + i, 280, ILI9341_BLACK);
    tft->drawLine(160 + i, 40, 160 + i, 280, ILI9341_BLACK);
    tft->drawLine(0, 120 + i, 240, 120 + i, ILI9341_BLACK);
    tft->drawLine(0, 200 + i, 240, 200 + i, ILI9341_BLACK);
  }
}

uint16_t GameUI::gridToPixelX(uint8_t coord) {
  return GRID_CELL_CENTER_OFFSET + coord * GRID_CELL_WIDTH;
}

uint16_t GameUI::gridToPixelY(uint8_t coord) {
  return GRID_END_Y - GRID_CELL_CENTER_OFFSET - coord * GRID_CELL_WIDTH;
}

uint8_t GameUI::pixelToGridX(uint16_t coord) {
  if (GRID_START_X > coord) return -1;
  if (GRID_END_X < coord) return -1;

  return (coord - GRID_START_X) / GRID_CELL_WIDTH;
}

uint8_t GameUI::pixelToGridY(uint16_t coord) {
  if (GRID_START_Y > coord) return -1;
  if (GRID_END_Y < coord) return -1;

  return 2 - (coord - GRID_START_Y) / GRID_CELL_WIDTH;
}

void GameUI::draw(uint8_t x, uint8_t y, int symbol) {
  if (symbol == SYMBOL_X)      drawX(x, y, ILI9341_BLACK);
  else if (symbol == SYMBOL_O) drawO(x, y, ILI9341_BLACK);
  else Serial.println("ERROR: GameUI::draw(): attempt to draw symbol other than X or O");
}

/* x and y should be numbers between 0 and 2 inclusive.
 * (0, 0) represents the bottom-left corner of the grid */
void GameUI::drawO(uint8_t x, uint8_t y, uint16_t color) {
  x = gridToPixelX(x);
  y = gridToPixelY(y);

  for (int r = 30; r <= 32; r++) {
    tft->drawCircle(x, y, r, color);
  }
}

void GameUI::drawX(uint8_t x, uint8_t y, uint16_t color) {
  x = gridToPixelX(x);
  y = gridToPixelY(y);

  // Main diagonal - NW to SE
  tft->drawLine(x - X_OFFSET - 1, y - X_OFFSET + 1, x + X_OFFSET - 1, y + X_OFFSET + 1, color);
  tft->drawLine(x - X_OFFSET,     y - X_OFFSET + 1, x + X_OFFSET,     y + X_OFFSET + 1, color);
  tft->drawLine(x - X_OFFSET,     y - X_OFFSET,     x + X_OFFSET,     y + X_OFFSET,     color);
  tft->drawLine(x - X_OFFSET + 1, y - X_OFFSET,     x + X_OFFSET + 1, y + X_OFFSET,     color);
  tft->drawLine(x - X_OFFSET + 1, y - X_OFFSET - 1, x + X_OFFSET + 1, y + X_OFFSET - 1, color);

  // Off diagonal - SE to NW
  tft->drawLine(x - X_OFFSET - 1, y + X_OFFSET - 1, x + X_OFFSET - 1, y - X_OFFSET - 1, color);
  tft->drawLine(x - X_OFFSET,     y + X_OFFSET - 1, x + X_OFFSET,     y - X_OFFSET - 1, color);
  tft->drawLine(x - X_OFFSET,     y + X_OFFSET,     x + X_OFFSET,     y - X_OFFSET,     color);
  tft->drawLine(x - X_OFFSET,     y + X_OFFSET + 1, x + X_OFFSET,     y - X_OFFSET + 1, color);
  tft->drawLine(x - X_OFFSET + 1, y + X_OFFSET + 1, x + X_OFFSET + 1, y - X_OFFSET + 1, color);
}

void GameUI::showVictory(const char *sideName) {
  tft->setCursor(0, 0);
  tft->write(sideName);
  tft->write(" wins!");
}

void GameUI::showStalemate() {
  tft->setCursor(0, 0);
  tft->write("Stalemate");
}

void GameUI::showMessage(std::string message) {
  // We center the message in the space above the grid.
  // The font is 10x14 (or rather 5x7 at 2x zoom).
  size_t width = 10 * message.length();
  uint16_t x = (tft->width() - width) / 2 - 5;
  uint16_t y = GRID_START_Y / 2 - 7;

  tft->setCursor(x, y);
  tft->write(message.c_str());
}