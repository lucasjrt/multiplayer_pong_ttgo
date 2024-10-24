#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "macros.h"
#include "menu.h"

class Menu;
class SubMenu;

class Graphics {
public:
    Graphics():
        fgColor(WHITE),
        bgColor(BLACK),
        selectedColor(LIGHT_BLUE) {}
    void drawSelectedBox(int index, const char *string, int topMargin = 0);
    void drawClearBox(int index, const char *string, int topMargin = 0);
    void showMenu(Menu* menu);
    void renderMenuOption(Menu* menu);
    void showMessage(const char *title, const char *message);
    int getLineCount(const char *message);
private:
    uint32_t fgColor, bgColor, selectedColor;
    void drawTitle(const char *title);
    void drawMessage(const char *message, int topMargin = 0);
    std::vector<std::string> wrapText(const char *text);
};