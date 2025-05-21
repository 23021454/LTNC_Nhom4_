#ifndef GAME_H
#define GAME_H

#include <SDL.h>
#include <SDL_ttf.h>
#include "Player.h"
#include "SoundManager.h"
// Kích thước bảng game: 15 cột x 10 hàng
const int BOARD_WIDTH = 15;
const int BOARD_HEIGHT = 10;
// Enum định nghĩa các nhưng loại ô trên bảng game
enum TileType {
    EMPTY,
    WALL,
    BOX,
    TARGET,
    BOX_TARGET,
    PLAYER
};

enum GameState {
    PLAYING,
    IN_GAME_MENU,
    LEVEL_SELECT
};

class Game {
public:
    Game();
    ~Game();

    void setWindowAndRenderer(SDL_Window* win, SDL_Renderer* ren);
    void run();

private:
    SDL_Window* window;
    SDL_Renderer* renderer;

    SDL_Texture* playerTextures[4];
    SDL_Texture* wallTexture;
    SDL_Texture* boxTexture[4];
    SDL_Texture* targetTexture;
    SDL_Texture* boxtargetTexture[4];
    SDL_Texture* emptyTexture;
    SDL_Texture* menuIconTexture;
    SDL_Texture* resetbuttonTexture;
    SDL_Texture* levelupTexture;
    SDL_Texture* continueTexture;
    SDL_Texture* backingameTexture;
    SDL_Texture* backgroundTexture; // New background texture for level selection menu

    int currentLevel;
    SoundManager soundManager;
    bool isWin;

    SDL_Rect destRect;

    Player* player;
    int board[BOARD_HEIGHT][BOARD_WIDTH];

    bool inGameMenuActive;
    bool shouldReset;
    bool shouldReturnToMenu;

    Uint32 startTicks;
    Uint32 elapsedTime;

    GameState gameState;
    bool completedLevels[15];

    void handleEvents();
    void update();
    void render();
    void loadTextures();
    void loadBoard(int level);
    void movePlayer(int dx, int dy);
    void cleanup();

    bool isLevelCompleted(int level) const { return completedLevels[level - 1]; }
    void markLevelCompleted(int level) { completedLevels[level - 1] = true; }
};

#endif
