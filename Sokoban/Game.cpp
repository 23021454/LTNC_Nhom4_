#include "Game.h"
#include <iostream>
#include <iomanip>
#include <SDL_image.h>
#include"SoundManager.h"
#include <SDL_ttf.h>
#include <fstream>
#include <sstream>
#include <vector>

// Hàm khởi tạo: Khởi tạo các biến thành giá trị mặc định
Game::Game() : 
    window(nullptr), renderer(nullptr), player(nullptr),
inGameMenuActive(false), shouldReset(false), shouldReturnToMenu(false),
menuIconTexture(nullptr), backgroundTexture(nullptr), currentLevel(1), isWin(false), gameState(PLAYING),
startTicks(0), elapsedTime(0), destRect{ 0, 0, 0, 0 } {

    for (int i = 0; i < 15; ++i) {
        completedLevels[i] = false;
    }

    for (int i = 0; i < 4; ++i) {
        playerTextures[i] = nullptr;
        boxTexture[i] = nullptr;
        boxtargetTexture[i] = nullptr;
    }
    wallTexture = nullptr;
    targetTexture = nullptr;
    emptyTexture = nullptr;
    resetbuttonTexture = nullptr;
    levelupTexture = nullptr;
    continueTexture = nullptr;
    backingameTexture = nullptr;
    for (int y = 0; y < BOARD_HEIGHT; ++y) {
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            board[y][x] = EMPTY;
        }
    }
}

Game::~Game() {
    cleanup();
}

void Game::setWindowAndRenderer(SDL_Window* win, SDL_Renderer* ren) {
    window = win;
    renderer = ren;
    loadTextures();
    player = new Player(1, 1);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    soundManager.loadSound("move", "sokoban/move.wav");
    soundManager.loadSound("yay", "sokoban/win.wav");
    soundManager.loadSound("meow", "sokoban/meow.wav");
    soundManager.loadSound("lose", "sokoban/lose.wav");
    soundManager.loadSound("click", "sokoban/buttonclick.wav");
    loadBoard(currentLevel);
    startTicks = SDL_GetTicks();
}

int typeBox[BOARD_HEIGHT][BOARD_WIDTH];

void Game::loadTextures() {
    playerTextures[0] = IMG_LoadTexture(renderer, "sokoban/player_05.png");
    playerTextures[1] = IMG_LoadTexture(renderer, "sokoban/player_08.png");
    playerTextures[2] = IMG_LoadTexture(renderer, "sokoban/player_17.png");
    playerTextures[3] = IMG_LoadTexture(renderer, "sokoban/player_20.png");

    wallTexture = IMG_LoadTexture(renderer, "sokoban/block_06.png");
    boxTexture[0] = IMG_LoadTexture(renderer, "sokoban/crate_03.png");
    boxTexture[1] = IMG_LoadTexture(renderer, "sokoban/crate_031.png");
    boxTexture[2] = IMG_LoadTexture(renderer, "sokoban/crate_032.png");
    boxTexture[3] = IMG_LoadTexture(renderer, "sokoban/crate_033.png");
    targetTexture = IMG_LoadTexture(renderer, "sokoban/ground_04.png");
    emptyTexture = IMG_LoadTexture(renderer, "sokoban/ground_01.png");

    boxtargetTexture[0] = IMG_LoadTexture(renderer, "sokoban/boxtarget_03.png");
    boxtargetTexture[1] = IMG_LoadTexture(renderer, "sokoban/boxtarget_031.png");
    boxtargetTexture[2] = IMG_LoadTexture(renderer, "sokoban/boxtarget_032.png");
    boxtargetTexture[3] = IMG_LoadTexture(renderer, "sokoban/boxtarget_033.png");

    menuIconTexture = IMG_LoadTexture(renderer, "sokoban/menu.png");
    resetbuttonTexture = IMG_LoadTexture(renderer, "sokoban/buttoningame.png");
    levelupTexture = IMG_LoadTexture(renderer, "sokoban/buttoningame.png");
    continueTexture = IMG_LoadTexture(renderer, "sokoban/buttoningame.png");
    backingameTexture = IMG_LoadTexture(renderer, "sokoban/buttoningame.png");

    backgroundTexture = IMG_LoadTexture(renderer, "sokoban/background_menu.png");

    if (!menuIconTexture) {
        std::cerr << "Không tải được biểu tượng menu: " << IMG_GetError() << std::endl;
    }
}

void Game::loadBoard(int level) {
    for (int y = 0; y < BOARD_HEIGHT; ++y) {
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            typeBox[y][x] = -1;
            board[y][x] = EMPTY;
        }
    }

    std::string filename = "level/level" + std::to_string(level) + ".txt";
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Không mở được file cấp độ: " << filename << std::endl;
        if (!player) {
            player = new Player(1, 1);
        }
        return;
    }

    std::string line;
    unsigned int y = 0;
    bool playerFound = false;
    while (std::getline(file, line) && y < BOARD_HEIGHT) {
        for (unsigned int x = 0; x < BOARD_WIDTH && x < line.length(); ++x) {
            switch (line[x]) {
            case '#': board[y][x] = WALL; break;
            case 'P':
                board[y][x] = EMPTY;
                if (player) {
                    delete player;
                }
                player = new Player(x, y);
                playerFound = true;
                break;
            case 'B': board[y][x] = BOX; typeBox[y][x] = 0; break;
            case 'C': board[y][x] = BOX; typeBox[y][x] = 1; break;
            case 'D': board[y][x] = BOX; typeBox[y][x] = 2; break;
            case 'E': board[y][x] = BOX; typeBox[y][x] = 3; break;
            case 'T': board[y][x] = TARGET; break;
            case 'X': board[y][x] = BOX_TARGET; typeBox[y][x] = 0; break;
            default: board[y][x] = EMPTY; break;
            }
        }
        ++y;
    }
    file.close();

    if (!playerFound) {
        if (player) {
            delete player;
        }
        player = new Player(1, 1);
    }
}

void Game::movePlayer(int dx, int dy) {
    if (!player) return;

    int newX = player->getX() + dx;
    int newY = player->getY() + dy;

    if (newX >= 0 && newX < BOARD_WIDTH && newY >= 0 && newY < BOARD_HEIGHT &&
        board[newY][newX] != WALL) {

        if (board[newY][newX] == BOX || board[newY][newX] == BOX_TARGET) {
            int boxNewX = newX + dx;
            int boxNewY = newY + dy;
            soundManager.playSound("meow");

            if (boxNewX >= 0 && boxNewX < BOARD_WIDTH && boxNewY >= 0 && boxNewY < BOARD_HEIGHT) {
                if (board[boxNewY][boxNewX] == EMPTY || board[boxNewY][boxNewX] == TARGET) {
                    board[boxNewY][boxNewX] = (board[boxNewY][boxNewX] == TARGET) ? BOX_TARGET : BOX;
                    board[newY][newX] = (board[newY][newX] == BOX_TARGET) ? TARGET : EMPTY;
                    typeBox[boxNewY][boxNewX] = typeBox[newY][newX];
                    typeBox[newY][newX] = -1;
                    player->move(dx, dy);
                }
            }
        }
        else if (board[newY][newX] == EMPTY || board[newY][newX] == TARGET) {
            player->move(dx, dy);
            soundManager.playSound("move");
        }
    }
}

void Game::handleEvents() {
    if (!player) return;

    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    // Định nghĩa vị trí menu icon động (góc trên bên phải, cách mép 10 pixel)
    SDL_Rect menuIcon = { windowWidth - 40 - 10, 10, 40, 40 };

    // Định nghĩa vị trí các nút trong menu trong game (căn giữa màn hình)
    const int buttonWidth = 200;
    const int buttonHeight = 50;
    const int spacing = 20; // Khoảng cách giữa các nút
    int totalHeight = 3 * buttonHeight + 2 * spacing;
    int startY = (windowHeight - totalHeight) / 2; // Căn giữa theo chiều dọc

    SDL_Rect firstBtn = { (windowWidth - buttonWidth) / 2, startY, buttonWidth, buttonHeight };
    SDL_Rect continueBtn = { (windowWidth - buttonWidth) / 2, startY + buttonHeight + spacing, buttonWidth, buttonHeight };
    SDL_Rect backMenuBtn = { (windowWidth - buttonWidth) / 2, startY + 2 * (buttonHeight + spacing), buttonWidth, buttonHeight };

    SDL_Rect levelButtons[15];
    const int levelButtonWidth = 80;
    const int levelButtonHeight = 80;
    const int startX = (windowWidth - 5 * levelButtonWidth - 4 * 10) / 2;
    const int levelStartY = (windowHeight - 3 * levelButtonHeight - 2 * 10) / 2;
    for (int i = 0; i < 15; ++i) {
        int row = i / 5;
        int col = i % 5;
        levelButtons[i] = { startX + col * (levelButtonWidth + 10), levelStartY + row * (levelButtonHeight + 10), levelButtonWidth, levelButtonHeight };
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            cleanup();
            exit(0);
        }

        if (event.type == SDL_MOUSEBUTTONDOWN) {
            int x = event.button.x;
            int y = event.button.y;

            if (gameState == PLAYING && !inGameMenuActive &&
                x >= menuIcon.x && x <= menuIcon.x + menuIcon.w &&
                y >= menuIcon.y && y <= menuIcon.y + menuIcon.h) {
                soundManager.playSound("click");
                inGameMenuActive = true;
                gameState = IN_GAME_MENU;
                return;
            }

            if (gameState == IN_GAME_MENU) {
                if (isWin) {
                    if (x >= firstBtn.x && x <= firstBtn.x + firstBtn.w &&
                        y >= firstBtn.y && y <= firstBtn.y + firstBtn.h) {
                        soundManager.playSound("click");
                        shouldReset = true;
                        inGameMenuActive = false;
                        gameState = PLAYING;
                    }
                }
                else {
                    if (x >= firstBtn.x && x <= firstBtn.x + firstBtn.w &&
                        y >= firstBtn.y && y <= firstBtn.y + firstBtn.h) {
                        soundManager.playSound("lose");
                        shouldReset = true;
                        inGameMenuActive = false;
                        gameState = PLAYING;
                    }
                    else if (x >= continueBtn.x && x <= continueBtn.x + continueBtn.w &&
                        y >= continueBtn.y && y <= continueBtn.y + continueBtn.h) {
                        soundManager.playSound("click");
                        inGameMenuActive = false;
                        gameState = PLAYING;
                    }
                }

                if (x >= backMenuBtn.x && x <= backMenuBtn.x + backMenuBtn.w &&
                    y >= backMenuBtn.y && y <= backMenuBtn.y + backMenuBtn.h) {
                    soundManager.playSound("click");
                    shouldReturnToMenu = true;
                    inGameMenuActive = false;
                    gameState = LEVEL_SELECT;
                }

                return;
            }

            if (gameState == LEVEL_SELECT) {
                for (int i = 0; i < 15; ++i) {
                    bool isUnlocked = (i == 0);
                    if (!isUnlocked) {
                        isUnlocked = true;
                        for (int j = 0; j < i; ++j) {
                            if (!completedLevels[j]) {
                                isUnlocked = false;
                                break;
                            }
                        }
                    }

                    if (!isUnlocked) continue;

                    if (x >= levelButtons[i].x && x <= levelButtons[i].x + levelButtons[i].w &&
                        y >= levelButtons[i].y && y <= levelButtons[i].y + levelButtons[i].h) {
                        currentLevel = i + 1;
                        loadBoard(currentLevel);
                        gameState = PLAYING;
                        startTicks = SDL_GetTicks();
                        isWin = false;
                        break;
                    }
                }
                return;
            }
        }

        if (gameState == PLAYING && !inGameMenuActive && event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_UP:    movePlayer(0, -1); player->setDirection(1); break;
            case SDLK_DOWN:  movePlayer(0, 1);  player->setDirection(0); break;
            case SDLK_LEFT:  movePlayer(-1, 0); player->setDirection(3); break;
            case SDLK_RIGHT: movePlayer(1, 0);  player->setDirection(2); break;
            }
        }
    }
}

void Game::update() {
    if (gameState != PLAYING) return;

    elapsedTime = SDL_GetTicks() - startTicks;
    if (shouldReset) {
        bool nextLevelUnlocked = true;
        if (isWin && currentLevel < 15) {
            int nextLevel = currentLevel;
            for (int j = 0; j < nextLevel; ++j) {
                if (!completedLevels[j]) {
                    nextLevelUnlocked = false;
                    break;
                }
            }
            if (nextLevelUnlocked) {
                currentLevel++;
            }
        }
        loadBoard(currentLevel);
        shouldReset = false;
        isWin = false;
        startTicks = SDL_GetTicks();
    }

    if (shouldReturnToMenu) {
        shouldReturnToMenu = false;
        return;
    }

    bool win = true;
    for (int y = 0; y < BOARD_HEIGHT; ++y) {
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            if (board[y][x] == BOX) {
                win = false;
                break;
            }
        }
    }

    if (win && !isWin) {
        isWin = true;
        std::cout << "You win! Time: " << elapsedTime / 1000.0f << " s\n";
        soundManager.playSound("yay");
        markLevelCompleted(currentLevel);
        inGameMenuActive = true;
        gameState = IN_GAME_MENU;
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 255, 208, 199, 255);
    SDL_RenderClear(renderer);

    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    if (gameState == LEVEL_SELECT) {
        if (backgroundTexture) {
            SDL_Rect backgroundRect = { 0, 0, windowWidth, windowHeight };
            SDL_RenderCopy(renderer, backgroundTexture, NULL, &backgroundRect);
        }

        const int buttonWidth = 80;
        const int buttonHeight = 80;
        const int startX = (windowWidth - 5 * buttonWidth - 4 * 10) / 2;
        const int startY = (windowHeight - 3 * buttonHeight - 2 * 10) / 2;

        TTF_Font* font = TTF_OpenFont("PixelOperatorHB8.ttf", 24);
        if (font) {
            for (int i = 0; i < 15; ++i) {
                int row = i / 5;
                int col = i % 5;
                SDL_Rect btnRect = { startX + col * (buttonWidth + 10), startY + row * (buttonHeight + 10), buttonWidth, buttonHeight };

                bool isUnlocked = (i == 0);
                if (!isUnlocked) {
                    isUnlocked = true;
                    for (int j = 0; j < i; ++j) {
                        if (!completedLevels[j]) {
                            isUnlocked = false;
                            break;
                        }
                    }
                }

                if (isUnlocked) {
                    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
                }
                else {
                    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
                }
                SDL_RenderFillRect(renderer, &btnRect);

                SDL_Color color;
                if (!isUnlocked) {
                    color = { 100, 100, 100, 255 };
                }
                else if (completedLevels[i]) {
                    color = { 0, 255, 0, 255 };
                }
                else {
                    color = { 200, 200, 200, 255 };
                }

                std::string levelText = std::to_string(i + 1);
                SDL_Surface* surface = TTF_RenderText_Blended(font, levelText.c_str(), color);
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_Rect textRect = { btnRect.x + (btnRect.w - surface->w) / 2, btnRect.y + (btnRect.h - surface->h) / 2, surface->w, surface->h };
                SDL_RenderCopy(renderer, texture, NULL, &textRect);

                SDL_FreeSurface(surface);
                SDL_DestroyTexture(texture);
            }
            TTF_CloseFont(font);
        }
    }
    else if (gameState == PLAYING || gameState == IN_GAME_MENU) {
        const int tileSize = 64;
        int minX = BOARD_WIDTH, minY = BOARD_HEIGHT;
        int maxX = 0, maxY = 0;

        for (int y = 0; y < BOARD_HEIGHT; ++y) {
            for (int x = 0; x < BOARD_WIDTH; ++x) {
                if (board[y][x] != EMPTY) {
                    if (x < minX) minX = x;
                    if (x > maxX) maxX = x;
                    if (y < minY) minY = y;
                    if (y > maxY) maxY = y;
                }
            }
        }

        if (player) {
            if (player->getX() < minX) minX = player->getX();
            if (player->getX() > maxX) maxX = player->getX();
            if (player->getY() < minY) minY = player->getY();
            if (player->getY() > maxY) maxY = player->getY();
        }

        int visibleWidth = maxX - minX + 1;
        int visibleHeight = maxY - minY + 1;

        int boardPixelWidth = visibleWidth * tileSize;
        int boardPixelHeight = visibleHeight * tileSize;

        int offsetX = (windowWidth - boardPixelWidth) / 2;
        int offsetY = (windowHeight - boardPixelHeight) / 2;

        bool isInner[BOARD_HEIGHT][BOARD_WIDTH] = { false };
        std::vector<std::pair<int, int>> queue;
        if (player) {
            queue.push_back({ player->getX(), player->getY() });
            isInner[player->getY()][player->getX()] = true;
        }

        while (!queue.empty()) {
            std::pair<int, int> pos = queue.back();
            int x = pos.first;
            int y = pos.second;
            queue.pop_back();

            int directions[4][2] = { {0, 1}, {0, -1}, {1, 0}, {-1, 0} };
            for (auto& dir : directions) {
                int newX = x + dir[0];
                int newY = y + dir[1];

                if (newX >= 0 && newX < BOARD_WIDTH && newY >= 0 && newY < BOARD_HEIGHT &&
                    !isInner[newY][newX] && board[newY][newX] != WALL) {
                    isInner[newY][newX] = true;
                    queue.push_back({ newX, newY });
                }
            }
        }

        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                if (board[y][x] == EMPTY && !isInner[y][x]) {
                    continue;
                }

                SDL_Rect destRect = {
                    offsetX + (x - minX) * tileSize,
                    offsetY + (y - minY) * tileSize,
                    tileSize, tileSize
                };
                switch (board[y][x]) {
                case WALL:
                    SDL_RenderCopy(renderer, wallTexture, NULL, &destRect);
                    break;
                case BOX:
                    SDL_RenderCopy(renderer, emptyTexture, NULL, &destRect);
                    SDL_RenderCopy(renderer, boxTexture[typeBox[y][x]], NULL, &destRect);
                    break;
                case BOX_TARGET:
                    SDL_RenderCopy(renderer, emptyTexture, NULL, &destRect);
                    SDL_RenderCopy(renderer, boxtargetTexture[typeBox[y][x]], NULL, &destRect);
                    break;
                case TARGET:
                    SDL_RenderCopy(renderer, emptyTexture, NULL, &destRect);
                    SDL_RenderCopy(renderer, targetTexture, NULL, &destRect);
                    break;
                case EMPTY:
                    SDL_RenderCopy(renderer, emptyTexture, NULL, &destRect);
                    break;
                }
            }
        }

        if (player) {
            SDL_Rect playerRect = {
                offsetX + (player->getX() - minX) * tileSize,
                offsetY + (player->getY() - minY) * tileSize,
                tileSize, tileSize
            };
            SDL_RenderCopy(renderer, playerTextures[player->getDirection()], NULL, &playerRect);
        }

        // Vẽ menu icon ở góc trên bên phải (cách mép phải 10 pixel)
        SDL_Rect menuIcon = { windowWidth - 40 - 10, 10, 40, 40 };
        SDL_RenderCopy(renderer, menuIconTexture, NULL, &menuIcon);

        if (inGameMenuActive) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
            SDL_Rect overlay = { 0, 0, windowWidth, windowHeight };
            SDL_RenderFillRect(renderer, &overlay);

            // Căn giữa các nút
            const int buttonWidth = 200;
            const int buttonHeight = 50;
            const int spacing = 20;
            int totalHeight = 3 * buttonHeight + 2 * spacing;
            int startY = (windowHeight - totalHeight) / 2;

            SDL_Rect resetBtn = { (windowWidth - buttonWidth) / 2, startY, buttonWidth, buttonHeight };
            SDL_Rect continueBtn = { (windowWidth - buttonWidth) / 2, startY + buttonHeight + spacing, buttonWidth, buttonHeight };
            SDL_Rect backBtn = { (windowWidth - buttonWidth) / 2, startY + 2 * (buttonHeight + spacing), buttonWidth, buttonHeight };

            SDL_RenderCopy(renderer, resetbuttonTexture, NULL, &resetBtn);
            SDL_RenderCopy(renderer, continueTexture, NULL, &continueBtn);
            SDL_RenderCopy(renderer, backingameTexture, NULL, &backBtn);

            TTF_Font* font = TTF_OpenFont("PixelOperatorHB8.ttf", 16);
            if (font) {
                SDL_Color color = { 255, 255, 255 };
                SDL_Surface* s1 = TTF_RenderText_Blended(font, isWin ? "Up Level" : "Reset", color);
                SDL_Surface* s2 = TTF_RenderText_Blended(font, "Continue", color);
                SDL_Surface* s3 = TTF_RenderText_Blended(font, "Back to Menu", color);
                SDL_Texture* t1 = SDL_CreateTextureFromSurface(renderer, s1);
                SDL_Texture* t2 = SDL_CreateTextureFromSurface(renderer, s2);
                SDL_Texture* t3 = SDL_CreateTextureFromSurface(renderer, s3);

                SDL_Rect r1 = { resetBtn.x + (200 - s1->w) / 2, resetBtn.y + (50 - s1->h) / 2, s1->w, s1->h };
                SDL_Rect r2 = { continueBtn.x + (200 - s2->w) / 2, continueBtn.y + (50 - s2->h) / 2, s2->w, s2->h };
                SDL_Rect r3 = { backBtn.x + (200 - s3->w) / 2, backBtn.y + (50 - s3->h) / 2, s3->w, s3->h };

                SDL_RenderCopy(renderer, t1, NULL, &r1);
                SDL_RenderCopy(renderer, t2, NULL, &r2);
                SDL_RenderCopy(renderer, t3, NULL, &r3);

                SDL_FreeSurface(s1); SDL_FreeSurface(s2); SDL_FreeSurface(s3);
                SDL_DestroyTexture(t1); SDL_DestroyTexture(t2); SDL_DestroyTexture(t3);
                TTF_CloseFont(font);
            }
        }

        TTF_Font* font = TTF_OpenFont("PixelOperatorHB8.ttf", 12);
        if (font) {
            SDL_Color color = { 0, 0, 0 };
            std::stringstream ss;
            ss << "Time: " << std::fixed << std::setprecision(2) << elapsedTime / 1000.0f << "s";

            SDL_Surface* timeSurface = TTF_RenderText_Blended(font, ss.str().c_str(), color);
            SDL_Texture* timeTexture = SDL_CreateTextureFromSurface(renderer, timeSurface);
            SDL_Rect timeRect = { 10, 10, timeSurface->w, timeSurface->h };
            SDL_RenderCopy(renderer, timeTexture, NULL, &timeRect);

            SDL_FreeSurface(timeSurface);
            SDL_DestroyTexture(timeTexture);
            TTF_CloseFont(font);
        }
    }

    SDL_RenderPresent(renderer);
}

void Game::cleanup() {
    for (int i = 0; i < 4; ++i) {
        SDL_DestroyTexture(playerTextures[i]);
        SDL_DestroyTexture(boxTexture[i]);
        SDL_DestroyTexture(boxtargetTexture[i]);
    }
    SDL_DestroyTexture(wallTexture);
    SDL_DestroyTexture(targetTexture);
    SDL_DestroyTexture(emptyTexture);
    SDL_DestroyTexture(menuIconTexture);
    SDL_DestroyTexture(resetbuttonTexture);
    SDL_DestroyTexture(levelupTexture);
    SDL_DestroyTexture(continueTexture);
    SDL_DestroyTexture(backingameTexture);
    SDL_DestroyTexture(backgroundTexture);
    if (player) {
        delete player;
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::run() {
    while (true) {
        handleEvents();
        update();
        render();
        SDL_Delay(24);
    }
}
