#include <raylib.h>
#include "globals.h"
#include "game.h"


using namespace std;

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