#include "InitSDL.h"
#include "Game.h"
#include "Menu.h"
#include <SDL_mixer.h>

int main(int argc, char* argv[]) {
    
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;


    if (!InitSDL::init(&window, &renderer, 960, 700)) {
        return -1;
    }

    //Khởi tạo SDL_Mixer
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            SDL_Log("SDL_mixer could not initialize! Mix_Error: %s", Mix_GetError());
            InitSDL::quit(window, renderer);
            return -1;
        }


    Menu menu;
    MenuResult result = menu.show(renderer);

    if (result == MENU_START) {
        Game game;
        game.setWindowAndRenderer(window, renderer);
        game.run();
    }

    Mix_CloseAudio();

    InitSDL::quit(window, renderer);
    return 0;
}
