#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_CS 4   /* shield pin 10 */
#define TFT_DC 3   /* shield pin 9  */
#define TFT_SCK 7  /* shield pin 13 */
#define TFT_MISO 6 /* shield pin 12 */
#define TFT_MOSI 5 /* shield pin 11 */
// -1 for the reset pin, which we don't use
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, -1, TFT_MISO);

// Determines the length of lines that draw X's
#define X_OFFSET 23

uint16_t gridToPixelX(uint8_t coord) {
  return 40 + coord * 80;
}

uint16_t gridToPixelY(uint8_t coord) {
  return 240 - coord * 80;
}

/* x and y should be numbers between 0 and 2 inclusive.
 * (0, 0) represents the bottom-left corner of the grid */
void drawO(uint8_t x, uint8_t y, uint16_t color) {
  x = gridToPixelX(x);
  y = gridToPixelY(y);

  for (int r = 30; r <= 32; r++) {
    tft.drawCircle(x, y, r, color);
  }
}

void drawX(uint8_t x, uint8_t y, uint16_t color) {
  x = gridToPixelX(x);
  y = gridToPixelY(y);

  // Main diagonal - NW to SE
  tft.drawLine(x - X_OFFSET - 1, y - X_OFFSET + 1, x + X_OFFSET - 1, y + X_OFFSET + 1, color);
  tft.drawLine(x - X_OFFSET, y - X_OFFSET + 1, x + X_OFFSET, y + X_OFFSET + 1, color);
  tft.drawLine(x - X_OFFSET, y - X_OFFSET, x + X_OFFSET, y + X_OFFSET, color);
  tft.drawLine(x - X_OFFSET + 1, y - X_OFFSET, x + X_OFFSET + 1, y + X_OFFSET, color);
  tft.drawLine(x - X_OFFSET + 1, y - X_OFFSET - 1, x + X_OFFSET + 1, y + X_OFFSET - 1, color);

  // Off diagonal - SE to NW
  tft.drawLine(x - X_OFFSET - 1, y + X_OFFSET - 1, x + X_OFFSET - 1, y - X_OFFSET - 1, color);
  tft.drawLine(x - X_OFFSET, y + X_OFFSET - 1, x + X_OFFSET, y - X_OFFSET - 1, color);
  tft.drawLine(x - X_OFFSET, y + X_OFFSET, x + X_OFFSET, y - X_OFFSET, color);
  tft.drawLine(x - X_OFFSET, y + X_OFFSET + 1, x + X_OFFSET, y - X_OFFSET + 1, color);
  tft.drawLine(x - X_OFFSET + 1, y + X_OFFSET + 1, x + X_OFFSET + 1, y - X_OFFSET + 1, color);
}

void setup() {
  tft.begin();
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextColor(ILI9341_BLUE, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextWrap(false);

  // Display the tic-tac-toe grid. We draw multiple lines to
  // display thicker lines
  for (int i = -1; i <= 1; i++) {
    tft.drawLine(80 + i, 40, 80 + i, 280, ILI9341_BLACK);
    tft.drawLine(160 + i, 40, 160 + i, 280, ILI9341_BLACK);
    tft.drawLine(0, 120 + i, 240, 120 + i, ILI9341_BLACK);
    tft.drawLine(0, 200 + i, 240, 200 + i, ILI9341_BLACK);
  }

  drawO(0, 0, ILI9341_BLACK);
  drawX(2, 0, ILI9341_BLACK);
}

void loop() {
}