#include "player.h"

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

void Player::tick(RemoteTick* remoteTick) {
    Paddle* p = this->getPaddle();
    p->setPos(WINDOW_WIDTH - remoteTick->playerPos);
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

void Player::setRemote(bool remote) {
  this->remote = remote;
}

int Player::getSpeed() {
  return this->speed;
}

bool Player::isRemote() {
  return this->remote;
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