#include <cstdlib>
#include "game.h"
#include "macros.h"

Ball::Ball(int size):
    size(size), 
    xSpeed(0),
    maxXSpeed(5) {
  int side = random(0, 2);
  if (side) ySpeed = 2;
  else ySpeed = -2;
}

int Ball::tick() {
  if (x + size / 2 >= WINDOW_WIDTH  || x - size / 2 < 0) xSpeed = -xSpeed;
  if (y - size >= WINDOW_HEIGHT) return -1;
  else if (y + size <= 0) return 1;
  prevX = x;
  prevY = y;
  setPosition(x + xSpeed, y + ySpeed);
  return 0;
}

void Ball::render() {
  Game::tft.fillRect(prevX - size / 2, prevY - size / 2, size, size, BLACK);
  Game::tft.fillRect(x - size / 2, y - size / 2, size, size, WHITE);
}

int Ball::getX() {
  return x;
}

int Ball::getY() {
  return y;
}

int Ball::getSize() {
  return size;
}

int Ball::getMaxXSpeed() {
  return maxXSpeed;
}

Side Ball::getDirection() {
  if (ySpeed > 0) return Side::DOWN;
  return Side::UP;
}

void Ball::setPosition(int x, int y) {
  this->x = x;
  this->y = y;
}

bool Ball::isInCenter(int threshold) {
  return y + size >= WINDOW_HEIGHT / 2 - threshold && y - size < WINDOW_HEIGHT / 2 + threshold;
}

void Ball::bounce(int angle) {
  xSpeed = constrain(angle, -maxXSpeed, maxXSpeed)
  ySpeed = -ySpeed;
}

void Ball::reset() {
  Game::tft.fillCircle(x, y, size, BLACK);
  x = WINDOW_WIDTH / 2;
  y = WINDOW_HEIGHT / 2;
  // xSpeed = random(-5, 5);

  int side = random(0, 2);
  if (side) ySpeed = 2;
  else ySpeed = -2;
}