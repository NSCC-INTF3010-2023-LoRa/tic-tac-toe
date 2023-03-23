#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_CS 4   /* shield pin 10 */
#define TFT_DC 3   /* shield pin 9  */
#define TFT_SCK 7  /* shield pin 13 */
#define TFT_MISO 6 /* shield pin 12 */
#define TFT_MOSI 5 /* shield pin 11 */
// -1 for the reset pin, which we don't use
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, -1, TFT_MISO);

/* x and y should be numbers between 0 and 2 inclusive.
 * (0, 0) represents the bottom-left corner of the grid */
void drawO(uint8_t x, uint8_t y, uint16_t color) {
  // Convert grid coords to pixel coords
  x = 40 + x * 80;
  y = 240 - y * 80;

  for (int r = 30; r <= 32; r++) {
    tft.drawCircle(x, y, r, color);
  }
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
    tft.drawLine(80 + i, 40 + i, 80 + i, 280 + i, ILI9341_BLACK);
    tft.drawLine(160 + i, 40 + i, 160 + i, 280 + i, ILI9341_BLACK);
    tft.drawLine(0 + i, 120 + i, 240 + i, 120 + i, ILI9341_BLACK);
    tft.drawLine(0 + i, 200 + i, 240 + i, 200 + i, ILI9341_BLACK);
  }

  drawO(0, 0, ILI9341_BLACK);
}

void loop() {
}