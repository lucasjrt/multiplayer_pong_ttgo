#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>
#include "game.h"
#include "macros.h"

class Game;
typedef void (*MenuHandler)(void*);

class Graphics {
public:
  Graphics():
    fgColor(WHITE),
    bgColor(BLACK),
    selectedColor(BLUE) {}
  void drawSelectedBox(int index, const char* string);
  void drawClearBox(int index, const char* string);
private:
  uint32_t fgColor, bgColor, selectedColor;
};

class Menu {
public:
  Menu(Game* game);
  void open();
  void close();
  void next();
  void previous();
  void back();
  void select();
  int getSelected();
  bool getIsOpen();
  Game* getGame();
  void render();
  void setControls(OneButton* lButton, OneButton* rButton);
  static void handlePrevious(void *context);
  static void handleNext(void *context);
  static void handleSelect(void *context);
  static void handleQuit(void *context);
  static void resumeOption(void *context);
  static void newGameOption(void *context);
  static void multiplayerOption(void *context);
  static void helpOption(void *context);
  void clearControls();
private:
  Game* game;
  bool isOpen;
  std::vector<std::string> options;
  int previousSelected;
  int selectedOption;
  std::vector<MenuHandler> handlers;
  OneButton* lButton;
  OneButton* rButton;
  Graphics* graphics;


  void acquireControls();
  void releaseControls();
};

#endif