#include <raylib.h>
#include "globals.h"
#include "game.h"

#ifdef EMSCRIPTEN_BUILD
#include <emscripten.h>
#endif

using namespace std;

Game game;

void MainLoop()
{
    game.Update();
    game.Draw();
}

int main()
{
    InitWindow(gameScreenWidth, gameScreenHeight, "Tetris");
    SetWindowPosition(50, 50);

    InitAudioDevice();
    SetMasterVolume(0.22f);
    SetExitKey(KEY_NULL);

    ToggleBorderlessWindowed();
    SetTargetFPS(144);

#ifdef EMSCRIPTEN_BUILD
    emscripten_set_main_loop(MainLoop, 0, 1);
#else
    while (!WindowShouldClose())
    {
        MainLoop();
    }

    CloseAudioDevice();
    CloseWindow();
#endif

    return 0;
}