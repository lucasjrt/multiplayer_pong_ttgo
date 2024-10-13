#include "game.h"
#include "macros.h"

void noop(void *_) {}

Menu::Menu(Game* game):
    game(game),
    selectedOption(0) {
  graphics = new Graphics();
  options = {"Resume", "New Game", "Multiplayer", "Help"};
  handlers.push_back(&Menu::resumeOption);
  handlers.push_back(&Menu::newGameOption);
  handlers.push_back(&Menu::multiplayerOption);
  handlers.push_back(&Menu::helpOption);
}

void Menu::open() {
  game->setPaused(true);
  Game::tft.fillScreen(BLACK);
  delay(1000);
  for (int i = 0; i < options.size(); i++) {
    if (i == selectedOption) {
      graphics->drawSelectedBox(i, options[i].c_str());
    } else {
      graphics->drawClearBox(i, options[i].c_str());
    }
  }
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

bool Menu::getIsOpen() {
  return isOpen;
}

Game* Menu::getGame() {
  return game;
}

void Menu::previous() {
  previousSelected = selectedOption;
  selectedOption = (selectedOption + options.size() - 1) % options.size();
  Serial.printf("Selected option: %d\n", selectedOption);
  render();
}

void Menu::next() {
  previousSelected = selectedOption;
  selectedOption = (selectedOption + 1) % options.size();
  Serial.printf("Selected option: %d\n", selectedOption);
  render();
}

void Menu::select() {
  handlers[selectedOption](this);
}

void Menu::render() {
  graphics->drawClearBox(previousSelected, options[previousSelected].c_str());
  graphics->drawSelectedBox(selectedOption, options[selectedOption].c_str());
}

void Menu::setControls(OneButton* lButton, OneButton* rButton) {
  this->lButton = lButton;
  this->rButton = rButton;
}

void Menu::acquireControls() {
  clearControls();
  lButton->setLongPressIntervalMs(400);
  lButton->attachClick(Menu::handlePrevious, this);
  lButton->attachLongPressStart(Menu::handleQuit, this);

  rButton->setLongPressIntervalMs(400);
  rButton->attachClick(Menu::handleNext, this);
  rButton->attachLongPressStart(Menu::handleSelect, this);
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

void Menu::handleQuit(void *context) {
  Menu* menu = static_cast<Menu*>(context);
  menu->close();
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

}

void Menu::helpOption(void *context) {

}

void Graphics::drawSelectedBox(int index, const char* text) {
    Game::tft.fillRect(MENU_MARGIN, MENU_MARGIN + index * OPTION_HEIGHT, WINDOW_WIDTH - 2 * MENU_MARGIN, OPTION_HEIGHT, fgColor);
    Game::tft.drawRect(MENU_MARGIN, MENU_MARGIN + index * OPTION_HEIGHT, WINDOW_WIDTH - 2 * MENU_MARGIN, OPTION_HEIGHT, selectedColor);
    Game::tft.setTextColor(selectedColor, fgColor);
    Game::tft.drawString(text, 2 * MENU_MARGIN, MENU_MARGIN + index * OPTION_HEIGHT + OPTION_HEIGHT / 2);
}

void Graphics::drawClearBox(int index, const char *text) {
    Game::tft.fillRect(MENU_MARGIN, MENU_MARGIN + index * OPTION_HEIGHT, WINDOW_WIDTH - 2 * MENU_MARGIN, OPTION_HEIGHT, bgColor);
    Game::tft.setTextColor(fgColor, bgColor);
    Game::tft.drawString(text, 2 * MENU_MARGIN, MENU_MARGIN + index * OPTION_HEIGHT + OPTION_HEIGHT / 2);
}