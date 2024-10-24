#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include "ball.h"
#include "game.h"
#include "macros.h"
#include "menu.h"
#include "network.h"
#include "paddle.h"
#include "player.h"

TFT_eSPI Game::tft = TFT_eSPI();
Game* Game::instance = nullptr;

Game::Game():
    field(nullptr),
    ball(nullptr),
    menu(nullptr),
    uPlayer(nullptr),
    dPlayer(nullptr),
    isMultiplayer(false),
    paused(false),
    lButton(nullptr),
    rButton(nullptr),
    peerMac{0} {
  if (instance) {
    // Crash if there is more than one instance
    Game::tft.fillScreen(RED);
    while (1) {}
  }
  instance = this;
  network = new Network(this);
  graphics = new Graphics();
  field = new Field();
  ball = new Ball(8);
  menu = new Menu(this);
  uPlayer = new Player(Side::UP);
  dPlayer = new Player(Side::DOWN);
  Paddle* uPaddle = new Paddle(WINDOW_WIDTH / 2);
  Paddle* dPaddle = new Paddle(WINDOW_WIDTH / 2);
  
  uPlayer->setPaddle(uPaddle);
  uPaddle->setPlayer(uPlayer);
  
  dPlayer->setPaddle(dPaddle);
  dPaddle->setPlayer(dPlayer);
  
  ball->setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

  initialRender();
}

void Game::tick() {
  this->lButton->tick();
  this->rButton->tick();
  if (paused) return;
  if (isMultiplayer) {
    network->sendTick(getRemoteTick());
    RemoteTick* remoteTick = network->receiveTick();
    if (remoteTick == nullptr) {
      Serial.println("Didn't receive a remote tick");
      Game::tft.fillScreen(BLUE);
      while (1) {}
    } else {
      Serial.println("Ticking remotely");
      uPlayer->tick(remoteTick);
      syncGame(remoteTick);
    }
  }
  dPlayer->tick();
  int scoredPlayer = ball->tick();
  int angle = 1024;
  if (ball->getDirection() == Side::UP) {
    angle = uPlayer->bounce(ball);
  } else if (ball->getDirection() == Side::DOWN) {
    angle = dPlayer->bounce(ball);
  }
  if (angle != 1024) {
    ball->bounce(angle);
  }
  if (scoredPlayer) {
    Serial.printf("Scored player: %d\n", scoredPlayer);
    score(scoredPlayer);
  }
}

void Game::render() {
  if (paused) return;
  uPlayer->render();
  dPlayer->render();
  if (ball->isInCenter(20)) {
    field->render();
    renderScore();
  }
  ball->render();
}

Menu* Game::getMenu() {
  return menu;
}

void Game::initialRender() {
  ball->render();
  uPlayer->render(true);
  dPlayer->render(true);
  renderScore();
  field->render();
}

void Game::setControls(OneButton* lButton, OneButton* rButton) {
  this->lButton = lButton;
  this->rButton = rButton;

  this->lButton->attachDuringLongPress(Game::handleOpenMenu, this);
  this->rButton->attachDuringLongPress(Game::handleOpenMenu, this);

  dPlayer->setControls(this->lButton, this->rButton);
  menu->setControls(this->lButton, this->rButton);
}

void Game::handleOpenMenu(void *context) {
  Game* game = static_cast<Game*>(context);
  OneButton* lButton = game->getLButton();
  OneButton* rButton = game->getRButton();
  if (lButton->getPressedMs() >= 2000 && rButton->getPressedMs() >= 2000 &&
      abs((int) lButton->getPressedMs() - (int) rButton->getPressedMs()) < 100) {
    Serial.println("Open menu");
    game->getMenu()->setCurrentMenu(MENU_MAIN);
    game->getMenu()->open();
  }
}

OneButton* Game::getLButton() {
  return lButton;
}

OneButton* Game::getRButton() {
  return rButton;
}

Graphics* Game::getGraphics() {
  return graphics;
}

Network* Game::getNetwork() {
  return network;
}

void Game::renderScore() {
  int u_score_int = uPlayer->getScore();
  int d_score_int = dPlayer->getScore();
  String u_score = String(u_score_int);
  String d_score = String(d_score_int);
  Game::tft.drawString(u_score, 10, WINDOW_HEIGHT / 2 - 20);
  Game::tft.drawString(d_score, WINDOW_WIDTH - (d_score.length() * 2) - 15, WINDOW_HEIGHT / 2 + 20);
}

void Game::setDScore(int score) {
  dPlayer->setScore(score);
}

void Game::setUScore(int score) {
  uPlayer->setScore(score);
}

void Game::score(int player) {
  delay(1000);
  if (player < 0) {
    setUScore(uPlayer->getScore() + 1);
  } else if (player > 0) {
    setDScore(dPlayer->getScore() + 1);
  }

  ball->recenter();
  uPlayer->centralize();
  dPlayer->centralize();
  initialRender();
  delay(100);
}

void Game::togglePause() {
  paused = !paused;
}

void Game::setPaused(bool paused) {
  this->paused = paused;
}

bool Game::isPaused() {
  return paused;
}

void Game::reset() {
  dPlayer->reset();
  uPlayer->reset();
  ball->recenter();
}

void Game::host() {
  menu->handleHostStart();
}

void Game::refreshJoinable() {
  network->discover();
  network->resetDiscovered();
  graphics->showMessage("Join Game", "Searching for games...");
  delay(500);
  std::vector<uint8_t*> discovered = network->getDiscovered();
  menu->updateJoinable(discovered);
  graphics->showMenu(menu);
}

void Game::initJoinable() {
  network->init();
  refreshJoinable();
}

void Game::join(uint8_t* mac) {
  setPeer(mac);
  initMultiplayer();
}

void Game::setPeer(uint8_t* mac) {
  network->setPeer(mac);
  memcpy(peerMac, mac, 6);
}

uint8_t* Game::getPeer() {
  return peerMac;
}

void Game::initMultiplayer() {
  Serial.println("Initializing multiplayer");
  isMultiplayer = true;
  reset();
  menu->close();
  setPaused(false);
}

void Game::cancelMultiplayer() {
  network->deinit();
}

RemoteTick* Game::getRemoteTick() {
  RemoteTick* tick = new RemoteTick();
  tick->playerPos = dPlayer->getPaddle()->getPos();
  tick->ballX = ball->getX();
  tick->ballY = ball->getY();
  tick->ballSpeedX = ball->getSpeedX();
  tick->ballSpeedY = ball->getSpeedY();
  return tick;
}

void Game::syncGame(RemoteTick* remoteTick) {
  if (ball->getX() != remoteTick->ballX) {
    Serial.printf("Ball X out of sync. Local: %d | Remote: %d", ball->getX(), remoteTick->ballX);
  }
  if (ball->getY() != remoteTick->ballY) {
    Serial.printf("Ball Y out of sync. Local: %d | Remote: %d", ball->getY(), remoteTick->ballY);
  }
  if (ball->getSpeedX() != remoteTick->ballSpeedX) {
    Serial.printf("Ball speed X out of sync. Local: %d | Remote: %d", ball->getSpeedX(), remoteTick->ballSpeedX);
  }
  if (ball->getSpeedY() != remoteTick->ballSpeedY) {
    Serial.printf("Ball speed Y out of sync. Local: %d | Remote: %d", ball->getSpeedY(), remoteTick->ballSpeedY);
  }
}

void Field::render() {
  for (int i = 0; i < WINDOW_WIDTH; i += 8) {
    Game::tft.fillRect(i, WINDOW_HEIGHT / 2, 4, 2, WHITE);
  }
}
