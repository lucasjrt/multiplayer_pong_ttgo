#ifndef GAME_H
#define GAME_H

#include <TFT_eSPI.h>
#include <OneButton.h>
#include "ball.h"
#include "macros.h"
#include "menu.h"

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
  int bounce(Ball* ball);
  void startMoving(int direction);
  void stopMoving();
  void centralize();
  void reset();
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
  Menu* getMenu();
  void tick();
  void render();
  void initialRender();
  void setControls(OneButton* lButton, OneButton* rButton);
  static void handleOpenMenu(void *context);
  OneButton* getLButton();
  OneButton* getRButton();
  void renderScore();
  void setDScore(int score);
  void setUScore(int score);
  void score(int player);
  void togglePause();
  void setPaused(bool paused);
  bool isPaused();
  void reset();
private:
  bool paused;
  Menu* menu;
  Ball* ball;
  Field* field;
  OneButton* lButton;
  OneButton* rButton;
  Player* u_player;
  Player* d_player;
};

#endif