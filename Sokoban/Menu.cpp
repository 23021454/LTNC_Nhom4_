#include "Menu.h"
#include <iostream>
#include <SDL_image.h>
#include "SoundManager.h"
#include <SDL_ttf.h>
#include <SDL_mixer.h>

Menu::Menu() {}

MenuResult Menu::show(SDL_Renderer* renderer) {
    SDL_Texture* backgroundSheet = IMG_LoadTexture(renderer, "sokoban/background.png");
    if (!backgroundSheet) {
        std::cerr << "Failed to load background sheet: " << IMG_GetError() << std::endl;
        return MENU_QUIT;
    }

    const int totalFrames = 11;
    int textureWidth, textureHeight;
    SDL_QueryTexture(backgroundSheet, nullptr, nullptr, &textureWidth, &textureHeight);

    int frameWidth = textureWidth / totalFrames;
    int frameHeight = textureHeight;

    int currentFrame = 0;
    Uint32 lastFrameTime = SDL_GetTicks();
    const int frameDelay = 100;

    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    int buttonWidth = 250;
    int buttonHeight = 70;
    int spacing = 10;

    // Load start button image
    SDL_Texture* startButtonTexture = IMG_LoadTexture(renderer, "sokoban/button.png");
    if (!startButtonTexture) {
        std::cerr << "Failed to load start button image: " << IMG_GetError() << std::endl;
        return MENU_QUIT;
    }


    // Load bg music
    Mix_Music* bgm = Mix_LoadMUS("sokoban/backgroundmusic.mp3");
    if (!bgm) {
        std::cout << "Failed to load music: " << Mix_GetError() << std::endl;
    }
    else {
        Mix_PlayMusic(bgm, -1); // Phát lặp vô hạn
    }

    Mix_Chunk* clickSound = Mix_LoadWAV("sokoban/buttonclick.wav");
    if (!clickSound) {
        std::cerr << "Failed to load click sound: " << Mix_GetError() << std::endl;
    }


    SDL_Rect startButton;
    SDL_Rect quitButton;

    // Tính v? trí nút đ? căn gi?a
    startButton.w = quitButton.w = buttonWidth;
    startButton.h = quitButton.h = buttonHeight;
    startButton.x = quitButton.x = (windowWidth - buttonWidth) / 2;

    int totalHeight = buttonHeight * 2 + spacing;
    int startY = (windowHeight - totalHeight) / 2;

    startButton.y = startY+25;
    quitButton.y = startY + buttonHeight + spacing+25;

    bool running = true;
    SDL_Event event;

    //SDL_Rect startButton = { 380, 200, 200, 60 };
    //SDL_Rect quitButton = { 380, 300, 200, 60 };

    // Load font
    TTF_Font* font = TTF_OpenFont("PixelOperatorHB8.ttf", 20);
    if (!font) {
        std::cout << "Failed to load font: " << TTF_GetError() << std::endl;
        return MENU_QUIT;
    }

    SDL_Color textColor = { 255, 255, 255 };

    // Create text for buttons
    SDL_Surface* startSurface = TTF_RenderText_Blended(font, "Start Game", textColor);
    SDL_Surface* quitSurface = TTF_RenderText_Blended(font, "Quit", textColor);


    SDL_Texture* startText = SDL_CreateTextureFromSurface(renderer, startSurface);
    SDL_Texture* quitText = SDL_CreateTextureFromSurface(renderer, quitSurface);

    SDL_Rect startTextRect = {
    startButton.x + (startButton.w - startSurface->w) / 2,
    startButton.y + (startButton.h - startSurface->h) / 2-5,
    startSurface->w,
    startSurface->h
    };

    SDL_Rect quitTextRect = {
        quitButton.x + (quitButton.w - quitSurface->w) / 2,
        quitButton.y + (quitButton.h - quitSurface->h) / 2-5 ,
        quitSurface->w,
        quitSurface->h
    };


    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                return MENU_QUIT;

            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x = event.button.x;
                int y = event.button.y;

                if (x >= startButton.x && x <= startButton.x + startButton.w &&
                    y >= startButton.y && y <= startButton.y + startButton.h) {

                    if (clickSound) Mix_PlayChannel(-1, clickSound, 0);

                    if (bgm) {
                        Mix_HaltMusic();
                        Mix_FreeMusic(bgm);
                    }

                    return MENU_START;
                }

                if (x >= quitButton.x && x <= quitButton.x + quitButton.w &&
                    y >= quitButton.y && y <= quitButton.y + quitButton.h) {

                    if (clickSound) Mix_PlayChannel(-1, clickSound, 0);

                    return MENU_QUIT;
                }
            }
        }

        // Cập nhật frame animation nền
        Uint32 now = SDL_GetTicks();
        if (now - lastFrameTime >= frameDelay) {
            currentFrame = (currentFrame + 1) % totalFrames;
            lastFrameTime = now;
        }

        // VẼ HÌNH NỀN (frame hiện tại từ sprite sheet)
        SDL_Rect srcRect = {
            currentFrame * frameWidth, 0,
            frameWidth, frameHeight
        };

        SDL_Rect dstRect = {
            0, 0,
            windowWidth, windowHeight // scale lên full màn hình
        };

        SDL_RenderCopy(renderer, backgroundSheet, &srcRect, &dstRect);


        // Draw background
       // SDL_SetRenderDrawColor(renderer, 30, 30, 80, 255);
        //SDL_RenderClear(renderer);

        // Draw buttons
        SDL_RenderCopy(renderer, startButtonTexture, nullptr, &startButton);
        SDL_RenderCopy(renderer, startButtonTexture, nullptr, &quitButton);

        // Draw text
        SDL_RenderCopy(renderer, startText, nullptr, &startTextRect);
        SDL_RenderCopy(renderer, quitText, nullptr, &quitTextRect);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    // Cleanup
    if (bgm) {
        Mix_HaltMusic();
        Mix_FreeMusic(bgm);
    }
    if (clickSound) Mix_FreeChunk(clickSound);
    SDL_DestroyTexture(startText);
    SDL_DestroyTexture(quitText);
    SDL_FreeSurface(startSurface);
    SDL_FreeSurface(quitSurface);
    SDL_DestroyTexture(startButtonTexture);
    TTF_CloseFont(font);


    return MENU_QUIT;
}
