#pragma once
#include <SDL.h>

enum MenuResult {
    MENU_QUIT,
    MENU_START
};

class Menu {
public:
    Menu();
    MenuResult show(SDL_Renderer* renderer);
};
