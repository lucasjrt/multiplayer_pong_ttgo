#ifndef GAME_H
#define GAME_H

#include <TFT_eSPI.h>
#include <OneButton.h>
#include "ball.h"
#include "macros.h"

class Ball;
class Paddle;

enum class Side {
  UP,
  DOWN,
};

class Field {
public:
  void render();
};

class Player {
public:
  Player(Side side);
  void tick();
  void render(bool force = false);
  Side getSide();
  Paddle* getPaddle();
  int getMovingDirection();
  int getScore();
  int getSpeed();
  void setScore(int score);
  void setPaddle(Paddle* paddle);
  void setControls(OneButton* lButton, OneButton* rButton);
  static void handleMoveLeftStart(void* context);
  static void handleMoveRightStart(void* context);
  static void handleMoveStopLeft(void* context);
  static void handleMoveStopRight(void* context);
  bool bounce(Ball* ball);
  void startMoving(int direction);
  void stopMoving();
  void centralize();
private:
  const Side side;
  int score, speed, moving;
  OneButton* lButton;
  OneButton* rButton;
  Paddle* paddle;
};

class Game {
public:
  static TFT_eSPI tft;
  Game();
  void tick();
  void render();
  void initialRender();
  void setControls(OneButton* lButton, OneButton* rButton);
  void renderScore();
  void setDScore(int score);
  void setUScore(int score);
  void score(int player);
private:
  Ball* ball;
  Field* field;
  Player* u_player;
  Player* d_player;
};

#endif