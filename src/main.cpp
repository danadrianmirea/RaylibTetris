#include <raylib.h>
#include "globals.h"
#include "game.h"
#include <iostream>

#ifdef EMSCRIPTEN_BUILD
#include <emscripten.h>
#endif

using namespace std;

void MainLoop(Game* game)
{
    game->Update();
    game->Draw();
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
    
    SetWindowPosition(50, 50);

    std::cout << "Initializing audio..." << std::endl;
    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
        std::cerr << "Failed to initialize audio device!" << std::endl;
        CloseWindow();
        return 1;
    }
    std::cout << "Audio initialized successfully" << std::endl;
    
    SetMasterVolume(0.22f);
    SetExitKey(KEY_NULL);

    ToggleBorderlessWindowed();
    SetTargetFPS(144);

    std::cout << "Creating game instance..." << std::endl;
    Game game;
    std::cout << "Game instance created successfully" << std::endl;

    std::cout << "Initializing game resources..." << std::endl;
    game.InitializeResources();
    std::cout << "Game resources initialized successfully" << std::endl;

    std::cout << "Entering main loop..." << std::endl;
#ifdef EMSCRIPTEN_BUILD
    emscripten_set_main_loop_arg(MainLoop, &game, 0, 1);
#else
    while (!WindowShouldClose() && !exitWindow)
    {
        MainLoop(&game);
    }
    std::cout << "Exiting main loop..." << std::endl;

    CloseAudioDevice();
    CloseWindow();
#endif

    std::cout << "Application exiting normally" << std::endl;
    return 0;
}