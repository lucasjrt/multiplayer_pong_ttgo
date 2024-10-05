#include <TFT_eSPI.h>
#include "paddle.h"
#include "game.h"
#include "macros.h"

void Paddle::render(bool force) {
  int direction = player->getMovingDirection();
  if (!direction && !force) return;

  Side side = player->getSide();
  int speed = player->getSpeed();
  int y = 0;
  if (side == Side::DOWN) y = WINDOW_HEIGHT - height;
  Game::tft.fillRect(pos - width / 2, y, width, height, WHITE);
  if (direction > 0) Game::tft.fillRect(pos - width / 2 - speed * 2, y, speed * 2, height, BLACK);
  else if (direction < 0) Game::tft.fillRect(pos + width / 2 + 1, y, speed * 2, height, BLACK);
}

void Paddle::centralize() {
  Side side = player->getSide();
  int y = 0;
  if (side == Side::DOWN) y = WINDOW_HEIGHT - height;
  Game::tft.fillRect(0, y, WINDOW_WIDTH, height, BLACK);
  pos = WINDOW_WIDTH / 2;
  render();
}

void Paddle::setPlayer(Player* player) {
  this->player = player;
}

void Paddle::setPos(int pos) {
  if (pos - width / 2 < 0) pos = width / 2;
  else if (pos + width / 2 > WINDOW_WIDTH) pos = WINDOW_WIDTH - width / 2 - 1;
  this->pos = pos;
}

int Paddle::bounce(Ball* ball) {
  // TODO: Improve comparisons
  if (!(ball->getX() + ball->getSize() / 2 > pos - width / 2 &&
        ball->getX() - ball->getSize() / 2 < pos + width / 2)) {
    return -1;
  }

  if ((player->getSide() == Side::DOWN && ball->getY() + ball->getSize() / 2 < WINDOW_HEIGHT - height) ||
      (player->getSide() == Side::UP   && ball->getY() - ball->getSize() / 2 > height)) {
    return -1;
  }
  return ball->getX() - pos;
}

int Paddle::getPos() {
  return this->pos;
}

int Paddle::getHeight() {
  return this->height;
}

int Paddle::getWidth() {
  return this->width;
}