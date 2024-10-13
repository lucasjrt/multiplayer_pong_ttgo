#include <Arduino.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <OneButton.h>
#include "game.h"
#include "macros.h"

#define LBUTTON 0
#define RBUTTON 35

#define UPS 30 // Updates per second
#define INTERVAL 1000 / UPS

unsigned long previousMillis = 0;

Game* game;
OneButton lButton, rButton;

void setup(void) {
  Serial.begin(115200);
  Serial.println("Starting function");
  randomSeed(analogRead(0)*analogRead(1));
  Serial.println("Random seed generated");

  Game::tft.init();
  Game::tft.setRotation(0);
  Game::tft.fillScreen(BLACK);
  
  lButton.setup(LBUTTON, INPUT_PULLDOWN, true);
  rButton.setup(RBUTTON, INPUT_PULLDOWN, true);

  game = new Game();
  game->setControls(&lButton, &rButton);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= INTERVAL) {
    previousMillis = currentMillis;
    game->tick();
    game->render();
  }
}
