#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include "ball.h"
#include "game.h"
#include "macros.h"
#include "paddle.h"

TFT_eSPI Game::tft = TFT_eSPI(); 

Game::Game():
    field(nullptr),
    ball(nullptr),
    menu(nullptr),
    u_player(nullptr),
    d_player(nullptr),
    paused(false),
    lButton(nullptr),
    rButton(nullptr),
    peerMac{0} {
  graphics = new Graphics();
  field = new Field();
  ball = new Ball(8);
  menu = new Menu(this);
  u_player = new Player(Side::UP);
  d_player = new Player(Side::DOWN);
  Paddle* u_paddle = new Paddle(WINDOW_WIDTH / 2);
  Paddle* d_paddle = new Paddle(WINDOW_WIDTH / 2);
  
  u_player->setPaddle(u_paddle);
  u_paddle->setPlayer(u_player);
  
  d_player->setPaddle(d_paddle);
  d_paddle->setPlayer(d_player);
  
  ball->setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

  initialRender();
}

void Game::tick() {
  this->lButton->tick();
  this->rButton->tick();
  if (paused) return;
  d_player->tick();
  int scoredPlayer = ball->tick();
  int angle = 1024;
  if (ball->getDirection() == Side::UP) {
    angle = u_player->bounce(ball);
  } else if (ball->getDirection() == Side::DOWN) {
    angle = d_player->bounce(ball);
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
  u_player->render();
  d_player->render();
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
  u_player->render(true);
  d_player->render(true);
  renderScore();
  field->render();
}

void Game::setControls(OneButton* lButton, OneButton* rButton) {
  this->lButton = lButton;
  this->rButton = rButton;

  this->lButton->attachDuringLongPress(Game::handleOpenMenu, this);
  this->rButton->attachDuringLongPress(Game::handleOpenMenu, this);

  d_player->setControls(this->lButton, this->rButton);
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

void Game::renderScore() {
  int u_score_int = u_player->getScore();
  int d_score_int = d_player->getScore();
  String u_score = String(u_score_int);
  String d_score = String(d_score_int);
  Game::tft.drawString(u_score, 10, WINDOW_HEIGHT / 2 - 20);
  Game::tft.drawString(d_score, WINDOW_WIDTH - (d_score.length() * 2) - 15, WINDOW_HEIGHT / 2 + 20);
}

void Game::setDScore(int score) {
  d_player->setScore(score);
}

void Game::setUScore(int score) {
  u_player->setScore(score);
}

void Game::score(int player) {
  delay(1000);
  if (player < 0) {
    setUScore(u_player->getScore() + 1);
  } else if (player > 0) {
    setDScore(d_player->getScore() + 1);
  }

  ball->recenter();
  u_player->centralize();
  d_player->centralize();
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
  d_player->reset();
  u_player->reset();
  ball->recenter();
  
}

void Game::host() {
  WiFi.disconnect();
  Serial.println("Hosting game");

  // Get own mac address
  String mac = WiFi.macAddress();
  char message[100];
  snprintf(message, 100, "Waiting for player to join. Your MAC address is:\n\n %s", mac.c_str());
  graphics->showMessage("Host", message);
}

void Game::join() {
  Serial.println("Joining game");
}

Player::Player(Side side):
    side(side),
    score(0),
    paddle(nullptr),
    speed(3),
    moving(0),
    lButton(nullptr),
    rButton(nullptr) {}

void Player::tick() {
  if (this->moving) {
    Paddle* p = this->getPaddle();
    p->setPos(p->getPos() + moving);
  }
}

void Player::render(bool force) {
  paddle->render(force);
}

Paddle* Player::getPaddle() {
  return this->paddle;
}

Side Player::getSide() {
  return side;
}

int Player::getMovingDirection() {
  return this->moving;
}

int Player::getScore() {
  return this->score;
}

void Player::setPaddle(Paddle *paddle) {
  this->paddle = paddle;
}

void Player::setScore(int score) {
  this->score = score;
}

void Player::setControls(OneButton* lButton, OneButton* rButton) {
  this->lButton = lButton;
  this->rButton = rButton;

  this->lButton->setLongPressIntervalMs(0);
  this->rButton->setLongPressIntervalMs(0);
  
  this->lButton->setClickMs(0);
  this->rButton->setClickMs(0);

  this->lButton->attachPress(Player::handleMoveLeftStart, this);
  this->lButton->attachClick(Player::handleMoveLeftStop, this);
  this->lButton->attachLongPressStop(Player::handleMoveLeftStop, this);
  
  this->rButton->attachPress(Player::handleMoveRightStart, this);
  this->rButton->attachClick(Player::handleMoveRightStop, this);
  this->rButton->attachLongPressStop(Player::handleMoveRightStop, this);
}

int Player::getSpeed() {
  return this->speed;
}

void Player::handleMoveLeftStart(void* context) {
  Serial.println("Handle move left");
  Player* player = static_cast<Player*>(context);
  player->startMoving(-1);
}

void Player::handleMoveRightStart(void* context) {
  Serial.println("Handle move right");
  Player* player = static_cast<Player*>(context);
  player->startMoving(1);
}

void Player::handleMoveLeftStop(void* context) {
  Serial.println("Handle stop left");
  Player* player = static_cast<Player*>(context);
  if (player->getMovingDirection() < 0) player->stopMoving();
}

void Player::handleMoveRightStop(void* context) {
  Serial.println("Handle stop right");
  Player* player = static_cast<Player*>(context);
  if (player->getMovingDirection() > 0) player->stopMoving();
}

void Player::startMoving(int direction) {
  this->moving = speed * direction;
}

void Player::stopMoving() {
  this->moving = 0;
}

void Player::centralize() {
  moving = 0;
  paddle->centralize();
}

int Player::bounce(Ball* ball) {
  return paddle->bounce(ball);
}

void Player::reset() {
  stopMoving();
  paddle->reset();
  score = 0;
}

void Field::render() {
  for (int i = 0; i < WINDOW_WIDTH; i += 8) {
    Game::tft.fillRect(i, WINDOW_HEIGHT / 2, 4, 2, WHITE);
  }
}
