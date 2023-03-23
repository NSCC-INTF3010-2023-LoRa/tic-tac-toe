#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>

#define TFT_CS 4 /* shield pin 10 */
#define TFT_DC 3 /* shield pin 9  */
#define SCK    7 /* shield pin 13 */
#define MISO   6 /* shield pin 12 */
#define MOSI   5 /* shield pin 11 */
#define TS_CS  8 /* shield pin 8  */
// -1 for the reset pin, which we don't use
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, MOSI, SCK, -1, MISO);
Adafruit_STMPE610 ts = Adafruit_STMPE610(TS_CS, MOSI, MISO, SCK);

// Determines the length of lines that draw X's
#define X_OFFSET 23

#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

// 240x240 centered on 240x320 screen
#define GRID_START_X 0
#define GRID_START_Y 40
#define GRID_END_X   240
#define GRID_END_Y   280
#define GRID_CELL_WIDTH ((GRID_END_X - GRID_START_X) / 3)
#define GRID_CELL_CENTER_OFFSET (GRID_CELL_WIDTH / 2)

#define TURN_X 0
#define TURN_O 1
uint8_t turn = TURN_X;

#define CELL_UNKNOWN 0
#define CELL_X 1
#define CELL_O 2
uint8_t grid[3][3] = {
  {CELL_UNKNOWN, CELL_UNKNOWN, CELL_UNKNOWN},
  {CELL_UNKNOWN, CELL_UNKNOWN, CELL_UNKNOWN},
  {CELL_UNKNOWN, CELL_UNKNOWN, CELL_UNKNOWN}
};

uint16_t gridToPixelX(uint8_t coord) {
  return GRID_CELL_CENTER_OFFSET + coord * GRID_CELL_WIDTH;
}

uint16_t gridToPixelY(uint8_t coord) {
  return GRID_END_Y - GRID_CELL_CENTER_OFFSET - coord * GRID_CELL_WIDTH;
}

uint8_t pixelToGridX(uint16_t coord) {
  if (GRID_START_X > coord) return -1;
  if (GRID_END_X < coord) return -1;

  return (coord - GRID_START_X) / GRID_CELL_WIDTH;
}

uint8_t pixelToGridY(uint16_t coord) {
  if (GRID_START_Y > coord) return -1;
  if (GRID_END_Y < coord) return -1;

  return 2 - (coord - GRID_START_Y) / GRID_CELL_WIDTH;
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

void triggerVictory(uint8_t side) {
  tft.setCursor(0, 0);
  tft.write(side == CELL_X ? "X" : "O");
  tft.write(" wins!");
}

void processTurn(uint8_t x, uint8_t y) {
  if (grid[x][y] != CELL_UNKNOWN) return;

  if (turn == TURN_X) {
    grid[x][y] = CELL_X;
    drawX(x, y, ILI9341_BLACK);
    turn = TURN_O;
  } else {
    grid[x][y] = CELL_O;
    drawO(x, y, ILI9341_BLACK);
    turn = TURN_X;
  }

  // Check for victory
  if (grid[0][y] == grid[1][y] && grid[1][y] == grid[2][y]) {
    return triggerVictory(grid[x][y]);
  }
  if (grid[x][0] == grid[x][1] && grid[x][1] == grid[x][2]) {
    return triggerVictory(grid[x][y]);
  }
  if (x == y) { // We're on the off diagonal
    if (grid[0][0] == grid[1][1] && grid[1][1] == grid[2][2]) {
      return triggerVictory(grid[x][y]);
    }
  }
  if (x + y == 2) { // We're on the main diagonal
    if (grid[0][2] == grid[1][1] && grid[1][1] == grid[2][0]) {
      return triggerVictory(grid[x][y]);
    }
  }

  // FIXME: check for stalemates
}

void setup() {
  Serial.begin(9600);
  tft.begin();
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
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

  while (!Serial);
  if (!ts.begin()) {
    Serial.println("Failed to start touchscreen");
    while (true) delay(1000);
  }
}

void loop() {
  if (ts.bufferEmpty()) return;

  TS_Point point = ts.getPoint();
  point.x = map(point.x, TS_MINX, TS_MAXX, 0, tft.width());
  point.y = map(point.y, TS_MINY, TS_MAXY, 0, tft.height());

  uint8_t x = pixelToGridX(point.x);
  if (x == -1) return;
  uint8_t y = pixelToGridY(point.y);
  if (y == -1) return;

  // drawX(x, y, ILI9341_BLACK);
  processTurn(x, y);
}