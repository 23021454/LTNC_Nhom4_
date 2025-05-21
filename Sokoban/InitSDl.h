#pragma once
#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>

namespace InitSDL {
    bool init(SDL_Window** window, SDL_Renderer** renderer, int width, int height);
    void quit(SDL_Window* window, SDL_Renderer* renderer);
}
