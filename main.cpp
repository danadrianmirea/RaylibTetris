#include <raylib.h>
#include "game.h"


using namespace std;

int main()
{
    Color darkBlue = {44, 44, 127, 255};

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