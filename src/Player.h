#pragma once

#include <OneButton.h>
#include "ball.h"
#include "game.h"
#include "macros.h"
#include "paddle.h"

enum class Side;
class Ball;
class Paddle;

class Player {
public:
  Player(Side side);
  void tick();
  void tick(RemoteTick* remoteTick);
  void render(bool force = false);
  Side getSide();
  Paddle* getPaddle();
  int getMovingDirection();
  int getScore();
  int getSpeed();
  bool isRemote();
  void setScore(int score);
  void setPaddle(Paddle* paddle);
  void setControls(OneButton* lButton, OneButton* rButton);
  void setRemote(bool remote);
  static void handleMoveLeftStart(void* context);
  static void handleMoveRightStart(void* context);
  static void handleMoveLeftStop(void* context);
  static void handleMoveRightStop(void* context);
  int bounce(Ball* ball);
  void startMoving(int direction);
  void stopMoving();
  void centralize();
  void reset();
private:
  const Side side;
  int score, speed, moving;
  bool remote;
  OneButton* lButton;
  OneButton* rButton;
  Paddle* paddle;
};