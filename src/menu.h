#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "game.h"
#include "graphics.h"
#include "macros.h"

class Game;

typedef void (*MenuHandler)(void*);

class MenuOption {
public:
  MenuOption(std::string text, MenuHandler handler):
    text(text),
    handler(handler) {}
  std::string getText();
  MenuHandler getHandler();
private:
  std::string text;
  MenuHandler handler;
};

class SubMenu {
public:
  SubMenu(std::string title, std::string text, std::vector<MenuOption> options):
    title(title),
    text(text),
    options(options) {}
  std::string getTitle();
  std::string getText();
  std::vector<MenuOption> getOptions();
private:
  std::string title;
  std::string text;
  std::vector<MenuOption> options;
};

class Menu {
public:
  Menu(Game* game);
  void open();
  void close();
  void next();
  void previous();
  void select();
  int getSelected();
  int getPreviousSelected();
  Game* getGame();
  SubMenu* getCurrentMenu();
  void setCurrentMenu(std::string name);
  // Button handlers
  void setControls(OneButton* lButton, OneButton* rButton);
  void clearControls();
  static void handlePrevious(void *context);
  static void handleNext(void *context);
  static void handleSelect(void *context);
  static void handleQuit(void *context);
  // Main menu options
  static void resumeOption(void *context);
  static void newGameOption(void *context);
  static void multiplayerOption(void *context);
  static void helpOption(void *context);
  // Multiplayer options
  static void hostOption(void *context);
  static void joinOption(void *context);
private:
  Game* game;
  int previousSelected;
  int selectedOption;
  std::unordered_map<std::string, SubMenu> menus;
  std::string currentMenu;

  OneButton* lButton;
  OneButton* rButton;
  Graphics* graphics;

  void acquireControls();
  void releaseControls();
};