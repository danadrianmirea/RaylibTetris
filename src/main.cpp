#include <raylib.h>
#include "globals.h"
#include "game.h"
#include <iostream>

#ifdef EMSCRIPTEN_BUILD
#include <emscripten.h>
#endif

Game* game;

using namespace std;

void MainLoop(void* arg)
{
    Game* gamePtr = static_cast<Game*>(arg);
    gamePtr->Update();
    gamePtr->Draw();
}

int main()
{
    std::cout << "Starting Tetris application..." << std::endl;
    
    std::cout << "Initializing window..." << std::endl;
    InitWindow(gameScreenWidth, gameScreenHeight, "Tetris");

    if (!IsWindowReady()) {
        std::cerr << "Failed to initialize window!" << std::endl;
        return 1;
    }
    std::cout << "Window initialized successfully" << std::endl;

#ifndef EMSCRIPTEN_BUILD
    SetWindowPosition(50, 50);
    ToggleBorderlessWindowed();
#endif

    SetExitKey(KEY_NULL);    
    SetTargetFPS(144);

    std::cout << "Creating game instance..." << std::endl;
    game = new Game();
    std::cout << "Game instance created successfully" << std::endl;

    std::cout << "Initializing game resources..." << std::endl;
    game->InitializeResources();
    std::cout << "Game resources initialized successfully" << std::endl;

    std::cout << "Entering main loop..." << std::endl;
#ifdef EMSCRIPTEN_BUILD
    emscripten_set_main_loop_arg(MainLoop, &game, 0, 1);
#else
    while (!WindowShouldClose() && !exitWindow)
    {
        MainLoop(game);
    }
    std::cout << "Exiting main loop..." << std::endl;

    CloseAudioDevice();
    CloseWindow();
#endif

    delete game;
    std::cout << "Application exiting normally" << std::endl;
    return 0;
}