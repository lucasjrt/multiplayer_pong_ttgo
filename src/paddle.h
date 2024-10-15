#pragma once

#include "game.h"

class Player;

class Paddle {
public:
  Paddle(int pos):
      pos(pos),
      player(nullptr),
      width(30),
      height(4) {
    pos = pos - width / 2;
  }
  void render(bool force = false);
  void setPlayer(Player* player);
  void setPos(int pos);
  void centralize();
  int bounce(Ball* ball);
  int getPos();
  int getHeight();
  int getWidth();
  void reset();
private:
  int pos;
  int width;
  int height;
  Player* player;
};