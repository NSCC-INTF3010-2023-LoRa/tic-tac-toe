#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>
#include <LoRa.h>

#include "GameState.h"
#include "GameUI.h"

#define FREQ 915E6

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

enum AppState {
  TITLE_SCREEN,
  GAME_IN_PROGRESS,
  PLAY_AGAIN_DIALOG,
  SEEKING_OPPONENT
};
AppState appState = TITLE_SCREEN;

uint16_t id;
unsigned long lastSendTime = millis();

enum LoRaInstructions {LI_INVALID = 0, LI_SEEKING_OPPONENT};

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!ts.begin()) {
    Serial.println("Failed to start touchscreen");
    while (true) delay(1000);
  }

  if (!LoRa.begin(FREQ)) {
    Serial.println("LoRa failed");
    while (true) delay(1000);
  }

  randomSeed(analogRead(A0));
  id = random(0xffff);
  ui.showTitleScreen();
}

void handleTitleScreenTaps() {
  if (ts.bufferEmpty()) return;

  TS_Point point = ts.getPoint();
  point.x = map(point.x, TS_MINX, TS_MAXX, 0, tft.width());
  point.y = map(point.y, TS_MINY, TS_MAXY, 0, tft.height());

  appState = SEEKING_OPPONENT;
  return;

  ui.blankScreen();
  ui.drawGrid();
  ui.showMessage("X's turn");
  // A tap typically sends multiple points over the wire. We slurp them
  // up here, so that they don't cause handleGameScreenTaps() to draw an
  // X or O prematurely.
  while (!ts.bufferEmpty()) ts.getPoint();
  gameState.begin();
  appState = GAME_IN_PROGRESS;
}

void handleGameScreenTaps() {
  if (ts.bufferEmpty()) return;

  TS_Point point = ts.getPoint();
  point.x = map(point.x, TS_MINX, TS_MAXX, 0, tft.width());
  point.y = map(point.y, TS_MINY, TS_MAXY, 0, tft.height());

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
    ui.showMessage("  Stalemate");
    ui.showPlayAgainDialog();
    appState = PLAY_AGAIN_DIALOG;
  } else if (result == VICTORY) {
    uint8_t player = gameState.currentPlayer();
    ui.draw(x, y, player == X ? SYMBOL_X : SYMBOL_O);
    ui.showMessage(player == X ? "X wins!" : "O wins!");
    ui.showPlayAgainDialog();
    appState = PLAY_AGAIN_DIALOG;
  }
}

void handlePlayAgainTaps() {
  if (ts.bufferEmpty()) return;

  TS_Point point = ts.getPoint();
  point.x = map(point.x, TS_MINX, TS_MAXX, 0, tft.width());
  point.y = map(point.y, TS_MINY, TS_MAXY, 0, tft.height());

  if (ui.areCoordsInYesButton(point.x, point.y)) {
    ui.blankScreen();
    ui.drawGrid();
    ui.showMessage("X's turn");
    // A tap typically sends multiple points over the wire. We slurp them
    // up here, so that they don't cause handleGameScreenTaps() to draw an
    // X or O prematurely.
    while (!ts.bufferEmpty()) ts.getPoint();
    gameState.begin();
    appState = GAME_IN_PROGRESS;
  } else if (ui.areCoordsInNoButton(point.x, point.y)) {
    ui.showTitleScreen();
    // A tap typically sends multiple points over the wire. We slurp them
    // up here, so that they don't cause handleTitleScreenTaps() to draw an
    // X or O prematurely.
    while (!ts.bufferEmpty()) ts.getPoint();
    appState = TITLE_SCREEN;
  }
}

void handleSeekingOpponent() {
  if (LoRa.parsePacket()) {
    uint16_t senderId = (LoRa.read() << 8) | LoRa.read();
    uint8_t instruction = LoRa.read();

    if (instruction == LI_SEEKING_OPPONENT) {
      Serial.print(senderId);
      Serial.println(" is seeking an opponent");
    } else {
      Serial.print(senderId);
      Serial.println(" sent an invalid instruction");
    }
  }

  if (millis() - lastSendTime >= 1000) {
    LoRa.beginPacket();
    LoRa.write(id >> 8);
    LoRa.write(id & 0xff);
    LoRa.write(LI_SEEKING_OPPONENT);
    LoRa.endPacket();

    Serial.print("Sought opponent as ");
    Serial.println(id);
    lastSendTime = millis();
  }
}

void loop() {
  switch (appState) {
    case TITLE_SCREEN:
      handleTitleScreenTaps();
      break;
    case GAME_IN_PROGRESS:
      handleGameScreenTaps();
      break;
    case PLAY_AGAIN_DIALOG:
      handlePlayAgainTaps();
      break;
    case SEEKING_OPPONENT:
      handleSeekingOpponent();
      break;
  }
}