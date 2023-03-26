#include "GameUI.h";

GameUI::GameUI(Adafruit_ILI9341 *tft) {
  this->tft = tft;

  tft->begin();
  tft->fillScreen(ILI9341_WHITE);
  tft->setTextColor(ILI9341_BLUE, ILI9341_WHITE);
  tft->setTextSize(2);
  tft->setTextWrap(false);
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

void GameUI::showMessage(std::string message) {
  // Clear the previous message
  tft->fillRect(0, 0, tft->width(), GRID_START_Y - 1, ILI9341_WHITE);

  // We center the message in the space above the grid.
  // The font is 10x14 (or rather 5x7 at 2x zoom).
  size_t width = 10 * message.length();
  uint16_t x = (tft->width() - width) / 2 - 5;
  uint16_t y = GRID_START_Y / 2 - 7;

  tft->setTextColor(ILI9341_BLUE, ILI9341_WHITE);
  tft->setCursor(x, y);
  tft->write(message.c_str());
}

void GameUI::showPlayAgainDialog() {
  // Whole dialog
  unsigned int horizontalPadding = 40;
  unsigned int boxWidth = tft->width() - 2 * horizontalPadding;
  unsigned int boxHeight = 100;
  unsigned int boxTop = (tft->height() - boxHeight) / 2;
  unsigned int boxLeft = horizontalPadding + 1;
  unsigned int boxBottom = boxTop + boxHeight;
  tft->fillRect(boxLeft, boxTop, boxWidth, boxHeight, ILI9341_WHITE);
  tft->drawRect(boxLeft, boxTop, boxWidth, boxHeight, ILI9341_BLACK);

  // Title bar
  unsigned int titleBoxBottom = boxTop + 19; // 16 for font + 2 for padding + 1 for top border = 19
  tft->drawLine(boxLeft, titleBoxBottom, tft->width() - horizontalPadding - 1, boxTop + 16 + 3, ILI9341_BLACK);
  tft->setCursor(boxLeft + 2, boxTop + 2);
  tft->write("Play again?");

  // Buttons
  unsigned int borderX = tft->width() / 2;
  unsigned int buttonTop = titleBoxBottom + 1;
  unsigned int buttonBottom = boxBottom - 1;
  unsigned int buttonHeight = buttonBottom - buttonTop;
  // No is one pixel wider because the border is just left of center
  unsigned int yesWidth = boxWidth / 2 - 2;
  unsigned int noWidth = boxWidth / 2 - 1;
  tft->drawLine(borderX, titleBoxBottom, tft->width() / 2, boxTop + boxHeight - 1, ILI9341_BLACK);
  tft->fillRect(boxLeft + 1, buttonTop, yesWidth, buttonHeight, tft->color565(64, 255, 64));
  tft->fillRect(borderX + 1, buttonTop, noWidth, buttonHeight, tft->color565(255, 64, 64));
  tft->setTextColor(ILI9341_BLACK);
  tft->setCursor(boxLeft + 1 + yesWidth / 2 - 15, buttonTop + buttonHeight / 2 - 8);
  tft->write("Yes");
  tft->setCursor(borderX + 1 + noWidth / 2 - 10, buttonTop + buttonHeight / 2 - 8);
  tft->write("No");
}

// FIXME: Do this and NoButton properly.
bool GameUI::areCoordsInYesButton(uint16_t x, uint16_t y) {
  // unsigned int horizontalPadding = 40;
  // unsigned int boxLeft = horizontalPadding + 1;
  // unsigned int boxWidth = tft->width() - 2 * horizontalPadding;
  // unsigned int boxHeight = 100;
  // unsigned int boxTop = (tft->height() - boxHeight) / 2;
  // unsigned int boxBottom = boxTop + boxHeight;
  // unsigned int titleBoxBottom = boxTop + 19; // 16 for font + 2 for padding + 1 for top border = 19
  // unsigned int borderX = tft->width() / 2;

  // return x > boxLeft && x < borderX && y > titleBoxBottom && y < boxBottom;

  return x > 41 && x < 120 && y > 129 && y < 129 + 79;
}

bool GameUI::areCoordsInNoButton(uint16_t x, uint16_t y) {
  // unsigned int horizontalPadding = 40;
  // unsigned int boxLeft = horizontalPadding + 1;
  // unsigned int boxWidth = tft->width() - 2 * horizontalPadding;
  // unsigned int boxHeight = 100;
  // unsigned int boxTop = (tft->height() - boxHeight) / 2;
  // unsigned int boxBottom = boxTop + boxHeight;
  // unsigned int titleBoxBottom = boxTop + 19; // 16 for font + 2 for padding + 1 for top border = 19
  // unsigned int borderX = tft->width() / 2;

  // return x > borderX && x < tft->width() - horizontalPadding && y > titleBoxBottom && y < boxBottom;

  return x > 120 && x < 199 && y > 129 && y < 129 + 79;
}

void GameUI::showTitleScreen() {
  tft->fillScreen(ILI9341_WHITE);

  unsigned int textWidth = 11 * 18;
  tft->setCursor((tft->width() - textWidth) / 2, 20);
  tft->setTextSize(3);
  tft->setTextColor(ILI9341_BLUE);
  tft->write("Tic Tac Toe");

  textWidth = 20 * 12;
  tft->setCursor((tft->width() - textWidth) / 2, 160);
  tft->setTextSize(2);
  tft->setTextColor(ILI9341_BLACK, ILI9341_WHITE);
  tft->write("Tap anywhere to play");
}

void GameUI::blankScreen() {
  tft->fillScreen(ILI9341_WHITE);
}

void GameUI::drawGrid() {
  // We draw multiple lines to display thicker lines
  for (int i = -1; i <= 1; i++) {
    tft->drawLine(80 + i, 40, 80 + i, 280, ILI9341_BLACK);
    tft->drawLine(160 + i, 40, 160 + i, 280, ILI9341_BLACK);
    tft->drawLine(0, 120 + i, 240, 120 + i, ILI9341_BLACK);
    tft->drawLine(0, 200 + i, 240, 200 + i, ILI9341_BLACK);
  }
}