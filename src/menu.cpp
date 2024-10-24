#include "game.h"
#include "macros.h"
#include "menu.h"

void noop(void *_) {}

std::string MenuOption::getText() {
  return text;
}

MenuHandler MenuOption::getHandler() {
  return handler;
}

std::string SubMenu::getTitle() {
  return title;
}

std::string SubMenu::getText() {
  return text;
}

void SubMenu::setText(std::string text) {
  this->text = text;
}

std::vector<MenuOption> SubMenu::getOptions() {
  return options;
}

void SubMenu::setOptions(std::vector<MenuOption> options) {
  this->options = options;
}

Menu::Menu(Game* game):
    game(game),
    selectedOption(0) {
  graphics = game->getGraphics();
  std::vector<MenuOption> mainMenu = {
    MenuOption("Resume", Menu::resumeOption),
    MenuOption("New Game", Menu::newGameOption),
    MenuOption("Multiplayer", Menu::multiplayerOption),
    MenuOption("Help", Menu::helpOption)
  };

  std::vector<MenuOption> multiplayerMenu = {
    MenuOption("Host", Menu::hostOption),
    MenuOption("Join", Menu::listJoinOption)
  };

  std::vector<MenuOption> joinMenu = {
    MenuOption("Refresh", Menu::refreshJoinOption)
  };

  std::vector<MenuOption> joinRequest = {
    MenuOption("Accept", Menu::acceptJoinOption),
    MenuOption("Decline", Menu::declineJoinOption)
  };

  menus.emplace(MENU_MAIN, SubMenu("Main Menu", "Select an option", mainMenu));
  menus.emplace(MENU_MULTIPLAYER, SubMenu("Multiplayer", "Select an option", multiplayerMenu));
  menus.emplace(MENU_MULTIPLAYER_JOIN, SubMenu("Join Game", "Select a game to join", joinMenu));
  menus.emplace(MENU_MULTIPLAYER_JOIN_REQUEST, SubMenu("Join Request", "An unknown player wants to join your game", joinRequest));
}

void Menu::open() {
  game->setPaused(true);
  lButton->reset();
  rButton->reset();
  lButton->tick();
  rButton->tick();
  delay(500);
  graphics->showMenu(this);
  Serial.println("Acquiring controls");
  acquireControls();
}

void Menu::close() {
  Game::tft.fillScreen(BLACK);
  Game::tft.setTextColor(WHITE, BLACK);
  releaseControls();
  game->initialRender();
  game->setPaused(false);
}

int Menu::getSelected() {
  return selectedOption;
}

int Menu::getPreviousSelected() {
  return previousSelected;
}

void Menu::stackMenu() {
  menuStack.push_back(currentMenu);
}

void Menu::unstackMenu() {
  if (menuStack.size() > 0) {
    currentMenu = menuStack.back();
    menuStack.pop_back();
    acquireControls();
  }
}

Game* Menu::getGame() {
  return game;
}

SubMenu* Menu::getCurrentMenu() {
  return &menus.at(currentMenu);
}

SubMenu* Menu::getMenu(std::string name) {
  return &menus.at(name);
}

void Menu::setCurrentMenu(std::string name) {
  currentMenu = name;
  selectedOption = 0;
  previousSelected = 0;
  graphics->showMenu(this);
}

void Menu::previous() {
  SubMenu* subMenu = getCurrentMenu();
  previousSelected = selectedOption;
  selectedOption = (selectedOption + subMenu->getOptions().size() - 1) % subMenu->getOptions().size();
  Serial.printf("Selected option: %d\n", selectedOption);
  graphics->renderMenuOption(this);
}

void Menu::next() {
  SubMenu* subMenu = getCurrentMenu();
  previousSelected = selectedOption;
  selectedOption = (selectedOption + 1) % subMenu->getOptions().size();
  Serial.printf("Selected option: %d\n", selectedOption);
  graphics->renderMenuOption(this);
}

void Menu::select() {
  SubMenu* subMenu = getCurrentMenu();
  MenuOption option = subMenu->getOptions()[selectedOption];
  option.getHandler()(this);
}

void Menu::setControls(OneButton* lButton, OneButton* rButton) {
  this->lButton = lButton;
  this->rButton = rButton;
}

void Menu::clearControls() {
  lButton->attachClick(noop, NULL);
  lButton->attachLongPressStart(noop, NULL);
  lButton->attachPress(noop, NULL);
  lButton->attachLongPressStop(noop, NULL);
  lButton->attachDuringLongPress(noop, NULL);

  rButton->attachClick(noop, NULL);
  rButton->attachLongPressStart(noop, NULL);
  rButton->attachPress(noop, NULL);
  rButton->attachLongPressStop(noop, NULL);
  rButton->attachDuringLongPress(noop, NULL);

  lButton->reset();
  rButton->reset();
  lButton->tick();
  rButton->tick();
}

void Menu::acquireControls() {
  clearControls();
  lButton->setLongPressIntervalMs(400);
  lButton->attachClick(Menu::handlePrevious, this);
  lButton->attachLongPressStart(Menu::handleBack, this);

  rButton->setLongPressIntervalMs(400);
  rButton->attachClick(Menu::handleNext, this);
  rButton->attachLongPressStart(Menu::handleSelect, this);
}

void Menu::releaseControls() {
  clearControls();
  game->setControls(lButton, rButton);
}

void Menu::handlePrevious(void *context) {
  Menu* menu = static_cast<Menu*>(context);
  menu->previous();
}

void Menu::handleNext(void *context) {
  Menu* menu = static_cast<Menu*>(context);
  menu->next();
}

void Menu::handleSelect(void *context) {
  Menu* menu = static_cast<Menu*>(context);
  Serial.println("Confirming select");
  menu->select();
}

void Menu::handleBack(void *context) {
  Menu* menu = static_cast<Menu*>(context);
  if (menu->menuStack.size() > 0) {
    menu->unstackMenu();
    menu->setCurrentMenu(menu->currentMenu);
  } else {
    menu->close();
  }
}

void Menu::attachBack() {
  lButton->attachLongPressStart(Menu::handleBack, this);
}

void Menu::resumeOption(void *context) {
  Menu* menu = static_cast<Menu*>(context);
  menu->close();
}

void Menu::newGameOption(void *context) {
  Menu* menu = static_cast<Menu*>(context);
  Game* game = menu->getGame();
  game->reset();
  menu->close();
}

void Menu::multiplayerOption(void *context) {
  Menu* menu = static_cast<Menu*>(context);
  menu->stackMenu();
  menu->setCurrentMenu(MENU_MULTIPLAYER);
}

void Menu::hostOption(void *context) {
  Menu* menu = static_cast<Menu*>(context);
  Game* game = menu->getGame();
  menu->handleHostStart();
}

void Menu::listJoinOption(void *context) {
  Menu* menu = static_cast<Menu*>(context);
  Game* game = menu->getGame();
  menu->stackMenu();
  menu->setCurrentMenu(MENU_MULTIPLAYER_JOIN);
  game->initJoinable();
  menu->open();
}

void Menu::handleJoinRequestSent() {
  Game* game = Game::instance;
  Menu* menu = game->getMenu();
  Network* network = game->getNetwork();

  String ownMac = network->getMacString();
  char message[100];
  snprintf(message, 100, "Waiting for response. Your MAC is: %s", ownMac.c_str());
  menu->stackMenu();
  menu->clearControls();
  game->getLButton()->attachLongPressStart(Menu::handleCancel, menu);
  game->getGraphics()->showMessage("Join Request", message);
}

void Menu::handleJoinRequestReceived(uint8_t* mac) {
  Game* game = Game::instance;
  Menu* menu = game->getMenu();
  Network* network = game->getNetwork();
  SubMenu* joinRequestMenu = menu->getMenu(MENU_MULTIPLAYER_JOIN_REQUEST);

  game->setPeer(mac);

  char message[100];
  String macStr = network->stringFromMac(mac);
  snprintf(message, 100, "Player %s wants to join your game", macStr.c_str());
  joinRequestMenu->setText(message);
  menu->stackMenu();
  menu->setCurrentMenu(MENU_MULTIPLAYER_JOIN_REQUEST);
  menu->acquireControls();
}

void Menu::handleJoinRequestAccepted() {
  Game* game = Game::instance;
  game->initMultiplayer();
}

void Menu::handleJoinRequestDeclined() {
  Game* game = Game::instance;
  Menu* menu = game->getMenu();
  menu->clearControls();
  game->getLButton()->attachLongPressStart(Menu::handleMultiplayerCancel, menu);
  game->getGraphics()->showMessage("Join Request", "Your request was declined");
}

void Menu::handleHostStart() {
  Game* game = Game::instance;
  Menu* menu = game->getMenu();
  Network* network = game->getNetwork();

  network->init();
  network->enableDiscovery();
  uint8_t* mac = network->getMac();
  String macStr = network->stringFromMac(mac);
  char message[100];
  snprintf(message, 100, "Waiting for player to join. Your MAC address is:\n\n %s", macStr.c_str());
  menu->clearControls();
  game->getLButton()->attachLongPressStart(Menu::handleMultiplayerCancel, menu);
  game->getGraphics()->showMessage("Host", message);
}

void Menu::updateJoinable(std::vector<uint8_t*> discovered) {
  SubMenu* joinableMenu = getMenu(MENU_MULTIPLAYER_JOIN);
  std::vector<MenuOption> newOptions;
  Network* network = game->getNetwork();
  newOptions.push_back(MenuOption("Refresh", Menu::refreshJoinOption));
  for (uint8_t* mac : discovered) {
    String macText = network->stringFromMac(mac);
    newOptions.push_back(MenuOption(macText.c_str(), Menu::requestJoinOption));
  }
  joinableMenu->setOptions(newOptions);
}

void Menu::refreshJoinOption(void *context) {
  Menu* menu = static_cast<Menu*>(context);
  Game* game = menu->getGame();
  game->refreshJoinable();
}

void Menu::requestJoinOption(void *context) {
  Menu* menu = static_cast<Menu*>(context);
  Game* game = menu->getGame();
  Network* network = game->getNetwork();

  SubMenu* joinMenu = menu->getMenu(MENU_MULTIPLAYER_JOIN);
  std::vector<MenuOption> options = joinMenu->getOptions();
  String selectedMac = options[menu->getSelected()].getText().c_str();
  uint8_t* mac = network->macFromString(selectedMac);
  network->requestJoin(mac);
  menu->handleJoinRequestSent();
}

void Menu::acceptJoinOption(void *context) {
  Menu* menu = static_cast<Menu*>(context);
  Game* game = menu->getGame();
  Network* network = game->getNetwork();
  network->acceptJoin();
  game->initMultiplayer();
}

void Menu::declineJoinOption(void *context) {
  Menu* menu = static_cast<Menu*>(context);
  Game* game = menu->getGame();
  Network* network = game->getNetwork();
  network->declineJoin();
  game->cancelMultiplayer();
  menu->unstackMenu();
  menu->setCurrentMenu(menu->currentMenu);
}

void Menu::handleCancel(void *context) {
  Menu* menu = static_cast<Menu*>(context);
  Game* game = menu->getGame();
  Serial.println("Cancelling");
  menu->unstackMenu();
  menu->setCurrentMenu(menu->currentMenu);
}

void Menu::handleMultiplayerCancel(void *context) {
  Menu* menu = static_cast<Menu*>(context);
  Game* game = menu->getGame();
  Serial.println("Cancelling multiplayer");
  game->cancelMultiplayer();
  menu->handleCancel(context);
}

void Menu::helpOption(void *context) {

}