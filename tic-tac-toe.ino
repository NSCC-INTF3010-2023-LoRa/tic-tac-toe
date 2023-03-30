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
  WAITING_FOR_USER,
  WAITING_FOR_OPPONENT,
  PLAY_AGAIN_DIALOG,
  SEEKING_OPPONENT,
  REQUESTING_MATCH
};
AppState appState = TITLE_SCREEN;

enum Protocol {PROT_CHAT = 1, PROT_TIC_TAC_TOE = 2};

uint16_t id;
uint16_t opponentId = 0;
unsigned long lastSendTime = millis();
unsigned long timeout = 0;

uint8_t symbol;

enum LoRaInstructions {
  LI_INVALID = 0,
  LI_SEEKING_OPPONENT,
  LI_MATCH_REQUEST,
  LI_ACCEPT_MATCH,
  LI_PLACE_SYMBOL
};

void setup() {
  Serial.begin(9600);

  if (!ts.begin()) {
    Serial.println("Failed to start touchscreen");
    while (true) delay(1000);
  }

  if (!LoRa.begin(FREQ)) {
    Serial.println("LoRa failed");
    while (true) delay(1000);
  }

  randomSeed(analogRead(A0));
  id = random(1, 0xffff);
  ui.showTitleScreen();
  ui.setTitleScreenMessage("Tap anywhere to play");
}

TS_Point getTap() {
  TS_Point point = ts.getPoint();
  point.x = map(point.x, TS_MINX, TS_MAXX, 0, tft.width());
  point.y = map(point.y, TS_MINY, TS_MAXY, 0, tft.height());

  // A tap typically sends multiple points over the wire. We slurp them
  // up here, so that they don't cause handleGameScreenTaps() to draw an
  // X or O prematurely.
  while (!ts.bufferEmpty()) ts.getPoint();
}

void handleTitleScreenTaps() {
  if (ts.bufferEmpty()) return;

  TS_Point point = getTap();
  ui.setTitleScreenMessage("Seeking opponent...");
  appState = SEEKING_OPPONENT;
}

void handleGameScreenTaps() {
  if (ts.bufferEmpty()) return;

  TS_Point point = ts.getPoint();
  point.x = map(point.x, TS_MINX, TS_MAXX, 0, tft.width());
  point.y = map(point.y, TS_MINY, TS_MAXY, 0, tft.height());

  // A tap typically sends multiple points over the wire. We slurp them
  // up here, so that they don't cause handleGameScreenTaps() to draw an
  // X or O prematurely.
  while (!ts.bufferEmpty()) ts.getPoint();

  Serial.print("Got tap (");
  Serial.print(point.x);
  Serial.print(", ");
  Serial.print(point.y);
  Serial.println(")");

  uint8_t x = ui.pixelToGridX(point.x);
  if (x == -1) return;
  uint8_t y = ui.pixelToGridY(point.y);
  if (y == -1) return;

  int8_t result = gameState.processMove(x, y);

  if (result != INVALID_MOVE) {
    Serial.print("Sending move as ");
    Serial.println(id);
    Serial.print("  Recipient: ");
    Serial.println(opponentId);
    Serial.print("  (");
    Serial.print(x);
    Serial.print(", ");
    Serial.print(y);
    Serial.println(")");
    Serial.print("  Encoded move: ");
    Serial.println(3 * x + y);

    LoRa.beginPacket();
    LoRa.write((uint8_t) PROT_TIC_TAC_TOE);
    LoRa.write(id >> 8);
    LoRa.write(id & 0xff);
    LoRa.write(LI_PLACE_SYMBOL);
    LoRa.write(opponentId >> 8);
    LoRa.write(opponentId & 0xff);
    LoRa.write(x * 3 + y);
    LoRa.endPacket();
  }
  
  if (result == CONTINUE) {
    ui.draw(x, y, symbol);
    ui.showMessage("Opponent's turn");
    appState = WAITING_FOR_OPPONENT;
  } else if (result == STALEMATE) {
    // FIXME: This makes absolutely no sense. The TS buffer should be
    // clear out earlier in this function, and yet here we are with more
    // points in the buffer.
    while (!ts.bufferEmpty()) ts.getPoint();

    ui.draw(x, y, symbol);
    ui.showMessage("Stalemate");
    ui.showPlayAgainDialog();
    appState = PLAY_AGAIN_DIALOG;
  } else if (result == VICTORY) {
    Serial.println("  Victory!");
    Serial.print("  Tap buffer is ");
    Serial.println(ts.bufferEmpty() ? "empty" : "full");
    // FIXME: This makes absolutely no sense. The TS buffer should be
    // clear out earlier in this function, and yet here we are with more
    // points in the buffer.
    while (!ts.bufferEmpty()) ts.getPoint();

    ui.draw(x, y, symbol);
    ui.showMessage("You win!");
    ui.showPlayAgainDialog();
    appState = PLAY_AGAIN_DIALOG;
  }
}

void handleLoRaMoves() {
  // Ignore taps when it's not the user's turn, otherwise
  // they'll confuse the game when the turn is complete.
  while (!ts.bufferEmpty()) ts.getPoint();
  if (!LoRa.parsePacket()) return;

  Serial.println("Handling a LoRa move");

  uint8_t protocol = LoRa.read();
  if (protocol != PROT_TIC_TAC_TOE) {
    while (LoRa.available()) LoRa.read();
    return;
  }

  uint16_t senderId = (LoRa.read() << 8) | LoRa.read();
  uint8_t instruction = LoRa.read();

  Serial.print("  Got instruction ");
  Serial.print(instruction);
  Serial.print(" from ");
  Serial.println(senderId);

  if (instruction == LI_PLACE_SYMBOL) {
    uint16_t recipientId = (LoRa.read() << 8) | LoRa.read();
    uint8_t move = LoRa.read();

    Serial.print("  Recipient is ");
    Serial.println(recipientId);

    if (recipientId != id) return;

    uint8_t x = move / 3;
    uint8_t y = move % 3;

    Serial.print("  (");
    Serial.print(x);
    Serial.print(", ");
    Serial.print(y);
    Serial.println(")");

    int8_t result = gameState.processMove(x, y);
    uint8_t otherSymbol = symbol == SYMBOL_X ? SYMBOL_O : SYMBOL_X;
    if (result == CONTINUE) {
      ui.draw(x, y, otherSymbol);
      ui.showMessage("Your turn");
      appState = WAITING_FOR_USER;
    } else if (result == STALEMATE) {
      ui.draw(x, y, otherSymbol);
      ui.showMessage("Stalemate");
      ui.showPlayAgainDialog();
      appState = PLAY_AGAIN_DIALOG;
    } else if (result == VICTORY) {
      ui.draw(x, y, otherSymbol);
      ui.showMessage("You lose!");
      ui.showPlayAgainDialog();
      appState = PLAY_AGAIN_DIALOG;
    }
  }
}

void handlePlayAgainTaps() {
  if (ts.bufferEmpty()) return;

  TS_Point point = ts.getPoint();
  point.x = map(point.x, TS_MINX, TS_MAXX, 0, tft.width());
  point.y = map(point.y, TS_MINY, TS_MAXY, 0, tft.height());

  // A tap typically sends multiple points over the wire. We slurp them
  // up here, so that they don't cause handleGameScreenTaps() to draw an
  // X or O prematurely.
  while (!ts.bufferEmpty()) ts.getPoint();

  Serial.print("  Dialog got tap (");
  Serial.print(point.x);
  Serial.print(", ");
  Serial.print(point.y);
  Serial.println(")");

  if (ui.areCoordsInYesButton(point.x, point.y)) {
    ui.showTitleScreen();
    ui.setTitleScreenMessage("Seeking opponent...");
    gameState.begin();
    while (!ts.bufferEmpty()) ts.getPoint();
    appState = SEEKING_OPPONENT;
  } else if (ui.areCoordsInNoButton(point.x, point.y)) {
    Serial.println("  Got a no");
    ui.showTitleScreen();
    ui.setTitleScreenMessage("Tap anywhere to play");
    while (!ts.bufferEmpty()) ts.getPoint();
    appState = TITLE_SCREEN;
  } else {
  }
}

void handleSeekingOpponent() {
  if (LoRa.parsePacket()) {
    uint8_t protocol = LoRa.read();
    if (protocol != PROT_TIC_TAC_TOE) {
      while (LoRa.available()) LoRa.read();
      return;
    }

    uint16_t senderId = (LoRa.read() << 8) | LoRa.read();
    uint8_t instruction = LoRa.read();

    if (instruction == LI_SEEKING_OPPONENT) {
      Serial.print("Requesting a match with ");
      Serial.println(senderId);
      opponentId = senderId;
      appState = REQUESTING_MATCH;
    } else if (instruction == LI_MATCH_REQUEST) {
      uint16_t recipientId = (LoRa.read() << 8) | LoRa.read();

      if (recipientId == id) {
        Serial.print(senderId);
        Serial.println(" requested a match with me!");

        LoRa.beginPacket();
        LoRa.write((uint8_t) PROT_TIC_TAC_TOE);
        LoRa.write(id >> 8);
        LoRa.write(id & 0xff);
        LoRa.write(LI_ACCEPT_MATCH);
        LoRa.write(senderId >> 8);
        LoRa.write(senderId & 0xff);
        LoRa.endPacket();

        opponentId = senderId;

        ui.blankScreen();
        ui.drawGrid();
        ui.showMessage("Opponent's turn");
        // A tap typically sends multiple points over the wire. We slurp them
        // up here, so that they don't cause handleGameScreenTaps() to draw an
        // X or O prematurely.
        while (!ts.bufferEmpty()) ts.getPoint();
        symbol = SYMBOL_O;
        gameState.begin();
        appState = WAITING_FOR_OPPONENT;
      } else {
        Serial.print(senderId);
        Serial.print(" requested a match with ");
        Serial.println(recipientId);
      }
    }
  }

  // FIXME: randomize interval
  if (millis() - lastSendTime >= 1000) {
    LoRa.beginPacket();
    LoRa.write((uint8_t) PROT_TIC_TAC_TOE);
    LoRa.write(id >> 8);
    LoRa.write(id & 0xff);
    LoRa.write(LI_SEEKING_OPPONENT);
    LoRa.endPacket();

    Serial.print("Sought opponent as ");
    Serial.println(id);
    lastSendTime = millis();
  }
}

void handleRequestingMatch() {
  if (!timeout) timeout = millis() + 5000;

  if (millis() >= timeout) {
    Serial.print(opponentId);
    Serial.println(" didn't respond");
    timeout = 0;
    opponentId = 0;
    ui.setTitleScreenMessage("Tap anywhere to play");
    appState = SEEKING_OPPONENT;
  }

  if (millis() - lastSendTime >= 1000) {
    LoRa.beginPacket();
    LoRa.write((uint8_t) PROT_TIC_TAC_TOE);
    LoRa.write(id >> 8);
    LoRa.write(id & 0xff);
    LoRa.write(LI_MATCH_REQUEST);
    LoRa.write(opponentId >> 8);
    LoRa.write(opponentId & 0xff);
    LoRa.endPacket();
    lastSendTime = millis();
  }

  if (LoRa.parsePacket()) {
    uint8_t protocol = LoRa.read();
    if (protocol != PROT_TIC_TAC_TOE) {
      while (LoRa.available()) LoRa.read();
      return;
    }

    uint16_t senderId = (LoRa.read() << 8) | LoRa.read();
    uint8_t instruction = LoRa.read();

    if (instruction == LI_ACCEPT_MATCH) {
      uint16_t recipientId = (LoRa.read() << 8) | LoRa.read();

      if (recipientId == id) {
        ui.blankScreen();
        ui.drawGrid();
        ui.showMessage("Your turn");
        // A tap typically sends multiple points over the wire. We slurp them
        // up here, so that they don't cause handleGameScreenTaps() to draw an
        // X or O prematurely.
        while (!ts.bufferEmpty()) ts.getPoint();
        gameState.begin();
        opponentId = senderId;
        symbol = SYMBOL_X;
        appState = WAITING_FOR_USER;

        Serial.print("Accepted match from ");
        Serial.println(senderId);
      } else {
        Serial.print("Ignoring accept match from ");
        Serial.print(senderId);
        Serial.print( " to ");
        Serial.println(recipientId);
      }
    }
  }
}

void loop() {
  switch (appState) {
    case TITLE_SCREEN:
      handleTitleScreenTaps();
      break;
    case WAITING_FOR_USER:
      handleGameScreenTaps();
      break;
    case WAITING_FOR_OPPONENT:
      handleLoRaMoves();
      break;
    case PLAY_AGAIN_DIALOG:
      handlePlayAgainTaps();
      break;
    case SEEKING_OPPONENT:
      handleSeekingOpponent();
      break;
    case REQUESTING_MATCH:
      handleRequestingMatch();
      break;
  }
}