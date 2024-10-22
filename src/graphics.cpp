#include "graphics.h"

void Graphics::drawSelectedBox(int index, const char* text, int topMargin) {
    uint16_t fontHeight = Game::tft.fontHeight();
    int y = topMargin + index * fontHeight * 2 + MENU_MARGIN;
    Game::tft.fillRect(MENU_MARGIN, y, WINDOW_WIDTH - MENU_MARGIN * 2, fontHeight * 2, bgColor);
    Game::tft.setTextColor(fgColor, bgColor);
    Game::tft.drawString(text, MENU_MARGIN * 2, y + fontHeight / 2);
    Game::tft.drawRect(MENU_MARGIN, y, WINDOW_WIDTH - MENU_MARGIN * 2, fontHeight * 2, selectedColor);
}

void Graphics::drawClearBox(int index, const char *text, int topMargin) {
    uint16_t fontHeight = Game::tft.fontHeight();
    int y = topMargin + index * fontHeight * 2 + MENU_MARGIN;
    Game::tft.fillRect(MENU_MARGIN, y, WINDOW_WIDTH - MENU_MARGIN * 2, fontHeight * 2, bgColor);
    Game::tft.setTextColor(fgColor, bgColor);
    Game::tft.drawString(text, MENU_MARGIN * 2, y + fontHeight / 2);
}

void Graphics::showMenu(Menu* menu) {
    SubMenu* subMenu = menu->getCurrentMenu();
    int fontHeight = Game::tft.fontHeight();
    int titleHeight = MENU_MARGIN + fontHeight * 2;
    int textHeight = titleHeight + fontHeight * 2 + MENU_MARGIN;
    Game::tft.fillScreen(bgColor);
    drawTitle(subMenu->getTitle().c_str());
    drawMessage(subMenu->getText().c_str());
    std::vector<MenuOption> options = subMenu->getOptions();
    for (int i = 0; i < options.size(); i++) {
        if (i == menu->getSelected()) {
            drawSelectedBox(i, options[i].getText().c_str(), textHeight);
        } else {
            drawClearBox(i, options[i].getText().c_str(), textHeight);
        }
    }
}

void Graphics::renderMenuOption(Menu* menu) {
    int currentSelected = menu->getSelected();
    int previousSelected = menu->getPreviousSelected();
    SubMenu* subMenu = menu->getCurrentMenu();
    int fontHeight = Game::tft.fontHeight();
    int titleHeight = MENU_MARGIN + fontHeight * 2;
    int textHeight = titleHeight + fontHeight * 2 + MENU_MARGIN;
    Game::tft.setTextColor(fgColor, bgColor);
    drawClearBox(previousSelected, subMenu->getOptions()[previousSelected].getText().c_str(), textHeight);
    drawSelectedBox(currentSelected, subMenu->getOptions()[currentSelected].getText().c_str(), textHeight);
}

void Graphics::showMessage(const char *title, const char *message) {
    Game::tft.fillScreen(bgColor);
    drawTitle(title);
    // Split words so we can wrap text and handle \n
    std::vector<std::string> words;
    std::string word;
    for (int i = 0; message[i] != '\0'; i++) {
        if (message[i] == '\n') {
            words.push_back(word);
            words.push_back("\n");
            word = "";
        } else if (message[i] == ' ') {
            words.push_back(word);
            word = "";
        } else {
            word += message[i];
        }
    }
    words.push_back(word);
    // Draw message
    int fontHeight = Game::tft.fontHeight();
    int titleHeight = MENU_MARGIN + fontHeight * 2;
    int textHeight = titleHeight + fontHeight * 2 + MENU_MARGIN;
    int x = MENU_MARGIN;
    int y = textHeight;
    while (words.size() > 0) {
        std::string line = words[0];
        words.erase(words.begin());
        while (words.size() > 0 && Game::tft.textWidth((line + " " + words[0]).c_str()) < WINDOW_WIDTH - MENU_MARGIN * 2) {
            if (words[0] == "\n") {
                words.erase(words.begin());
                break;
            }
            line += " " + words[0];
            words.erase(words.begin());
        }
        Game::tft.drawString(line.c_str(), x, y);
        y += fontHeight + LINE_GAP;
    }
}

void Graphics::drawTitle(const char *title) {
    uint16_t fontHeight = Game::tft.fontHeight();
    int textWidth = Game::tft.textWidth(title);
    int x = (WINDOW_WIDTH - textWidth) / 2;
    Game::tft.setTextColor(selectedColor, bgColor);
    Game::tft.drawString(title, x, MENU_MARGIN);
}

void Graphics::drawMessage(const char *message) {
    uint16_t fontHeight = Game::tft.fontHeight();
    int textWidth = Game::tft.textWidth(message);
    int x = (WINDOW_WIDTH - textWidth) / 2;
    int titleHeight = MENU_MARGIN + fontHeight * 2 + MENU_MARGIN;
    Game::tft.setTextColor(fgColor, bgColor);
    Game::tft.drawString(message, x, titleHeight);
}
