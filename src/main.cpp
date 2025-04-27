#include <raylib.h>
#include "globals.h"
#include "game.h"
#include <iostream>

#ifdef EMSCRIPTEN_BUILD
#include <emscripten.h>
#endif

Game* game = nullptr;

using namespace std;

void MainLoop(void* arg)
{
    Game* gamePtr = static_cast<Game*>(arg); 
    gamePtr->Update();
    gamePtr->Draw();
}

int main()
{
    InitWindow(gameScreenWidth, gameScreenHeight, "Tetris");
#ifndef EMSCRIPTEN_BUILD
    SetWindowState(FLAG_WINDOW_RESIZABLE);
#endif

    if (!IsWindowReady()) {
        return 1;
    }

#ifndef EMSCRIPTEN_BUILD
    if (fullscreen) {
        ToggleBorderlessWindowed();
    }
#endif

    SetExitKey(KEY_NULL);
    SetTargetFPS(144);

    game = new Game();
    if (!game) {
        CloseWindow();
        return 1;
    }
    game->InitializeResources();
 
#ifdef EMSCRIPTEN_BUILD
    emscripten_set_main_loop_arg(MainLoop, game, 0, 1);
#else
    while (!WindowShouldClose() && !exitWindow)
    {
        MainLoop(game);
    }
#endif

    CloseAudioDevice();
    CloseWindow();

    if (game) {
        delete game;
        game = nullptr;
    }
    return 0;
}