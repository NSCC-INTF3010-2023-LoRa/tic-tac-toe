#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>

#include "GameState.h"
#include "GameUI.h"

/* The touchscreen reports values in this range for tap coords. We use this to
 * calculate the pixel that was clicked. */
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

#define TFT_CS 4 /* shield pin 10 */
#define TFT_DC 3 /* shield pin 9  */
#define SCK    7 /* shield pin 13 */
#define MISO   6 /* shield pin 12 */
#define MOSI   5 /* shield pin 11 */
#define TS_CS  8 /* shield pin 8  */
// -1 for the reset pin, which we don't use
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, MOSI, SCK, -1, MISO);
Adafruit_STMPE610 ts = Adafruit_STMPE610(TS_CS, MOSI, MISO, SCK);

GameState gameState;
GameUI ui(&tft);

enum AppState { TITLE_SCREEN, GAME_IN_PROGRESS, PLAY_AGAIN_DIALOG };
AppState appState = TITLE_SCREEN;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!ts.begin()) {
    Serial.println("Failed to start touchscreen");
    while (true) delay(1000);
  }

  // ui.showMessage("X's turn");
  ui.showTitleScreen();
}

void handleTitleScreenTaps(TS_Point point) {
  ui.blankScreen();
  ui.drawGrid();
  ui.showMessage("X's turn");
  // A tap typically sends multiple points over the wire. We slurp them
  // up here, so that they don't cause handleGameScreenTaps() to draw an
  // X or O prematurely.
  while (!ts.bufferEmpty()) ts.getPoint();
  appState = GAME_IN_PROGRESS;
}

void handleGameScreenTaps(TS_Point point) {
  uint8_t x = ui.pixelToGridX(point.x);
  if (x == -1) return;
  uint8_t y = ui.pixelToGridY(point.y);
  if (y == -1) return;

   int8_t result = gameState.processMove(x, y);
  
  if (result == CONTINUE) {
    uint8_t lastPlayer = gameState.lastPlayer();
    ui.draw(x, y, lastPlayer == X ? SYMBOL_X : SYMBOL_O);
    ui.showMessage(lastPlayer == X ? "O's turn" : "X's turn");
  } else if (result == STALEMATE) {
    uint8_t player = gameState.currentPlayer();
    ui.draw(x, y, player == X ? SYMBOL_X : SYMBOL_O);
    ui.showMessage("Stalemate");
    ui.showPlayAgainDialog();
  } else if (result == VICTORY) {
    uint8_t player = gameState.currentPlayer();
    ui.draw(x, y, player == X ? SYMBOL_X : SYMBOL_O);
    ui.showMessage(player == X ? "X wins!" : "O wins!");
    ui.showPlayAgainDialog();
  }
}

void loop() {
  if (ts.bufferEmpty()) return;

  TS_Point point = ts.getPoint();
  point.x = map(point.x, TS_MINX, TS_MAXX, 0, tft.width());
  point.y = map(point.y, TS_MINY, TS_MAXY, 0, tft.height());

  switch (appState) {
    case TITLE_SCREEN:
      handleTitleScreenTaps(point);
      break;
    case GAME_IN_PROGRESS:
      handleGameScreenTaps(point);
      break;
  }
}