#pragma once

#include <TFT_eSPI.h>
#include <OneButton.h>
#include "ball.h"
#include "macros.h"
#include "menu.h"
#include "network.h"
#include "paddle.h"

class Ball;
class Graphics;
class Menu;
class Paddle;
class Player;

enum class Side {
  UP,
  DOWN,
};

class Field {
public:
  void render();
};

typedef struct RemoteTick {
  int playerPos;
  int ballX;
  int ballY;
  int ballSpeedX;
  int ballSpeedY;
} RemoteTick;

class Game {
public:
  static TFT_eSPI tft;
  static Game* instance;

  Game();
  Menu* getMenu();
  void tick();
  void render();
  void initialRender();
  void setControls(OneButton* lButton, OneButton* rButton);
  static void handleOpenMenu(void *context);
  OneButton* getLButton();
  OneButton* getRButton();
  Graphics* getGraphics();
  Network* getNetwork();
  void renderScore();
  void setDScore(int score);
  void setUScore(int score);
  void score(int player);
  void togglePause();
  void setPaused(bool paused);
  bool isPaused();
  void reset();
  // Multiplayer
  void host();
  void refreshJoinable();
  void initJoinable();
  void join(uint8_t* mac);
  void setPeer(uint8_t* mac);
  uint8_t* getPeer();
  void initMultiplayer();
  void cancelMultiplayer();
  RemoteTick* getRemoteTick();
  void syncGame(RemoteTick* remoteTick);
  void requestSent();
private:

  bool paused;
  bool isMultiplayer;
  Network* network;
  Menu* menu;
  Ball* ball;
  Field* field;
  Graphics* graphics;
  OneButton* lButton;
  OneButton* rButton;
  Player* uPlayer;
  Player* dPlayer;
  uint8_t peerMac[6];
};