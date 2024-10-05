#ifndef BALL_H
#define BALL_H

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
  Side getDirection();
  void setX(int x);
  void setY(int y);
  void setPosition(int x, int y);
  bool isInCenter(int threshold);
  void bounce();
  void reset();
private:
  int x, y;
  int prevX, prevY;
  int size;
  int xSpeed, ySpeed;
};

#endif