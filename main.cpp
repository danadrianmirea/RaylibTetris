#include <raylib.h>
#include "game.h"


using namespace std;

#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

int main()
{
    InitWindow(500, 620, "Tetris");
    SetTargetFPS(144);

    Game game;

    while (WindowShouldClose() == false)
    {
        game.Update();
        
        BeginDrawing();
        ClearBackground(darkBlue);
        game.Draw();
        EndDrawing();
    }

    return 0;
}