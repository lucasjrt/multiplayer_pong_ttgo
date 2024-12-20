#pragma once
#include "game.h"

enum class Side;

class Ball {
public:
  Ball(int size);
  int tick();
  void render();
  int getX();
  int getY();
  int getSize();
  int getSpeedX();
  int getSpeedY();
  int getMaxXSpeed();
  Side getDirection();
  void setX(int x);
  void setY(int y);
  void setSpeedX(int xSpeed);
  void setSpeedY(int ySpeed);
  void setPosition(int x, int y);
  bool isInCenter(int threshold);
  void bounce(int angle);
  void recenter();
  void reset();
private:
  int x, y;
  int prevX, prevY;
  int size;
  int maxXSpeed;
  int xSpeed, ySpeed;
};