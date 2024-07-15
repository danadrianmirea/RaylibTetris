#include <raylib.h>
#include "globals.h"
#include "game.h"

using namespace std;

int main()
{
    InitWindow(gameScreenWidth, gameScreenHeight, "Tetris");
    InitAudioDevice();
    SetMasterVolume(0.22f);
    SetExitKey(KEY_NULL);

    Game game;

    SetWindowSize(windowWidth, windowHeight);
    SetWindowPosition(50, 50);
    SetTargetFPS(144);
    
    ToggleBorderlessWindowed();

    while (!exitWindow)
    {
        game.Update();
        game.Draw();
    }
    
    CloseAudioDevice();
    CloseWindow();

    return 0;
}