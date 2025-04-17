#include <raylib.h>
#include "globals.h"
#include "game.h"

#ifdef __EMSCRIPTEN__
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

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(MainLoop, 0, 1);
#else
    while (!exitWindow)
    {
        MainLoop();
    }

    CloseAudioDevice();
    CloseWindow();
#endif

    return 0;
}