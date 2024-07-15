#include <iostream>
#include <random>

#include "game.h"

float lastUpdateTime = 0.0f;

bool EventTriggered(float interval)
{
    float currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

Game::Game()
{
    firstTimeGameStart = true;

    targetRenderTex = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(targetRenderTex.texture, TEXTURE_FILTER_BILINEAR); // Texture scale filter to use

    grid = Grid();
    font = LoadFontEx("Font/monogram.ttf", 64, 0, 0);
    InitAudioDevice();
    music = LoadMusicStream("Sounds/music.mp3");
    SetMusicVolume(music, 0.3f);
    PlayMusicStream(music);

    manipulateSound = LoadSound("Sounds/rotate.mp3");
    clearSound = LoadSound("Sounds/clear.mp3");

    InitGame();
}

void Game::InitGame()
{
    grid.Initialize();
    blocks = GetAllBlocks();
    currentBlock = GetRandomBlock();
    nextBlock = GetRandomBlock();
    score = 0;
    lockBlockTimer = 0.0f;
    lockBlock = false;
    lockStateMoves = 0;
    currentLevel = startingLevel;
    lastInputTime = inputDelay;
    lastRotateInputTime = rotateInputDelay;
    lastSoftDropTimeTick = softDropInputDelay;

    isFirstFrameAfterReset = true;
    isInExitMenu = false;
    paused = false;
    lostWindowFocus = false;
    gameOver = false;

    screenScale = MIN((float)GetScreenWidth() / gameScreenWidth, (float)GetScreenHeight() / gameScreenHeight);
}

void Game::Reset()
{
    InitGame();
}


Game::~Game()
{
    CloseAudioDevice();
    UnloadRenderTexture(targetRenderTex);
    UnloadFont(font);
    UnloadMusicStream(music);
    UnloadSound(manipulateSound);
    UnloadSound(clearSound);
}

Block Game::GetRandomBlock()
{
    if (blocks.empty())
    {
        blocks = GetAllBlocks();
    }
    int randomIndex = rand() % blocks.size();
    Block block = blocks[randomIndex];
    blocks.erase(blocks.begin() + randomIndex);
    return block;
}

std::vector<Block> Game::GetAllBlocks()
{
    return {IBlock(), JBlock(), LBlock(), OBlock(), SBlock(), TBlock(), ZBlock()};
}

void Game::Update()
{
    screenScale = MIN((float)GetScreenWidth() / gameScreenWidth, (float)GetScreenHeight() / gameScreenHeight);
    UpdateUI();

    bool running = (firstTimeGameStart == false && paused == false && lostWindowFocus == false && isInExitMenu == false && gameOver == false);

    if (running)
    {
        HandleInput();
        UpdateMusicStream(music);

        if (EventTriggered(0.9f / currentLevel))
        {
            MoveBlockDown();
        }

        if (lockBlock)
        {
            lockBlockTimer += GetFrameTime();
            if (lockBlockTimer > blockLockTime)
            {
                LockBlock();
            }
        }
    }
}

void Game::Draw()
{
    // render everything to a texture
    BeginTextureMode(targetRenderTex);
    ClearBackground(darkBlue);
    grid.Draw();
    currentBlock.Draw(0, 0);
    DrawUI();

    EndTextureMode();

    // render the scaled frame texture to the screen
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(targetRenderTex.texture, (Rectangle){0.0f, 0.0f, (float)targetRenderTex.texture.width, (float)-targetRenderTex.texture.height},
                   (Rectangle){(GetScreenWidth() - ((float)gameScreenWidth * screenScale)) * 0.5f, (GetScreenHeight() - ((float)gameScreenHeight * screenScale)) * 0.5f, (float)gameScreenWidth * screenScale, (float)gameScreenHeight * screenScale},
                   (Vector2){0, 0}, 0.0f, WHITE);

    DrawScreenSpaceUI();
    EndDrawing();
}

void Game::DrawUI()
{
    const int fontSize = 30;

    DrawRectangleRounded(Rectangle{320, 55, 170, 60}, 0.3, 6, lightBlue);
    DrawRectangleRounded(Rectangle{320, 215, 170, 180}, 0.3, 6, lightBlue);

    DrawTextEx(font, "Score", {365, 15}, fontSize, 2, WHITE);
    DrawTextEx(font, TextFormat("%d", score), {365, 60}, fontSize, 2, WHITE);

    DrawTextEx(font, "Next", {365, 175}, fontSize, 2, WHITE);
    nextBlock.Draw(260, 260);

    /*
        if (gameOver)
        {
            if (score < maxScore)
            {
                DrawTextEx(font, "Game Over", {320, 450}, fontSize, 2, WHITE);
                DrawTextEx(font, "Press Space", {320, 490}, fontSize, 2, WHITE);
            }
            else
            {
                DrawTextEx(font, "You win!", {320, 450}, fontSize, 2, WHITE);
                DrawTextEx(font, "Press Space", {320, 490}, fontSize, 2, WHITE);
            }
        }
    */
    DrawTextEx(font, TextFormat("Level: %d", currentLevel), {345, 540}, fontSize, 2, WHITE);
}

void Game::DrawScreenSpaceUI()
{
     if (exitWindowRequested)
    {
        DrawRectangleRounded({(float)(GetScreenWidth() / 2 - 500), (float)(GetScreenHeight() / 2 - 40), 1000, 120}, 0.76f, 20, BLACK);
        DrawText("Are you sure you want to exit? [Y/N]", GetScreenWidth() / 2 - 400, GetScreenHeight() / 2, 40, yellow);
    }
    else if (firstTimeGameStart)
    {
        DrawRectangleRounded({(float)(GetScreenWidth() / 2 - 500), (float)(GetScreenHeight() / 2 - 40), 1000, 120}, 0.76f, 20, BLACK);
        DrawText("Press SPACE to play", GetScreenWidth() / 2 - 200, GetScreenHeight() / 2, 40, yellow);
    }
    else if (paused)
    {
        DrawRectangleRounded({(float)(GetScreenWidth() / 2 - 500), (float)(GetScreenHeight() / 2 - 40), 1000, 120}, 0.76f, 20, BLACK);
        DrawText("Game paused, press P to continue", GetScreenWidth() / 2 - 400, GetScreenHeight() / 2, 40, yellow);
    }
    else if (lostWindowFocus)
    {
        DrawRectangleRounded({(float)(GetScreenWidth() / 2 - 500), (float)(GetScreenHeight() / 2 - 40), 1000, 120}, 0.76f, 20, BLACK);
        DrawText("Game paused, focus window to continue", GetScreenWidth() / 2 - 400, GetScreenHeight() / 2, 40, yellow);
    }
    else if (gameOver)
    {
        DrawRectangleRounded({(float)(GetScreenWidth() / 2 - 500), (float)(GetScreenHeight() / 2 - 40), 1000, 120}, 0.76f, 20, BLACK);
        DrawText("Game over, press SPACE to play again", GetScreenWidth() / 2 - 400, GetScreenHeight() / 2, 40, yellow);
    }   
}

void Game::HandleInput()
{
    /*
    if (gameOver)
    {
        int keyPress = GetKeyPressed();
        if (keyPress == KEY_SPACE)
        {
            InitGame();
        }
    }

    else
    {
    */
    lastInputTime += GetFrameTime();
    lastSoftDropTimeTick += GetFrameTime();
    lastRotateInputTime += GetFrameTime();

    bool goodMove = false;

    if (lastInputTime >= inputDelay)
    {
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        {
            goodMove = MoveBlockLeft();
            if (goodMove)
            {
                // PlaySound(manipulateSound);
            }
            lastInputTime = 0.0f;
        }

        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        {
            goodMove = MoveBlockRight();
            lastInputTime = 0.0f;
            if (goodMove)
            {
                // PlaySound(manipulateSound);
            }
        }
    }

    if (lastRotateInputTime >= rotateInputDelay)
    {
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        {
            goodMove = RotateBlock();
            lastRotateInputTime = 0.0f;
        }
    }

    if (lastSoftDropTimeTick >= softDropInputDelay)
    {
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        {
            // MoveBlockDown();
            SnakeDropBlock();
            lastSoftDropTimeTick = 0.0f;
        }
    }

    if (goodMove)
    {
        if (lockBlock)
        {
            if (lockStateMoves < maxLockStateMoves)
            {
                // reset lock timer on good move
                lockBlockTimer = 0.0f;
                lockStateMoves++;
            }
        }
    }
    //}
}

void Game::UpdateUI()
{
    if (WindowShouldClose() || (IsKeyPressed(KEY_ESCAPE) && exitWindowRequested == false))
    {
        exitWindowRequested = true;
        isInExitMenu = true;
        return;
    }

    if (IsKeyPressed(KEY_ENTER) && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
    {
        if (fullscreen)
        {
            fullscreen = false;
        }
        else
        {
            fullscreen = true;
        }
        // ToggleFullscreen();

        SetWindowSize(windowWidth, windowHeight);
        ToggleBorderlessWindowed();
    }

    if (firstTimeGameStart && IsKeyPressed(KEY_SPACE))
    {
        firstTimeGameStart = false;
    }
    else if (gameOver && IsKeyPressed(KEY_SPACE))
    {
        Reset();
    }

    if (exitWindowRequested)
    {
        if (IsKeyPressed(KEY_Y))
        {
            exitWindow = true;
        }
        else if (IsKeyPressed(KEY_N) || IsKeyPressed(KEY_ESCAPE))
        {
            exitWindowRequested = false;
            isInExitMenu = false;
        }
    }

    if (IsWindowFocused() == false)
    {
        lostWindowFocus = true;
    }
    else
    {
        lostWindowFocus = false;
    }

    if (exitWindowRequested == false && lostWindowFocus == false && gameOver == false && isFirstFrameAfterReset == false && IsKeyPressed(KEY_P))
    {
        if (paused)
        {
            paused = false;
        }
        else
        {
            paused = true;
        }
    }
}

bool Game::MoveBlockLeft()
{
    currentBlock.Move(0, -1);
    if (IsBlockOutside() || BlockFits() == false)
    {
        currentBlock.Move(0, 1);
        return false;
    }
    return true;
}

bool Game::MoveBlockLeftRepeat(int count)
{
    currentBlock.Move(0, -count);
    if (IsBlockOutside() || BlockFits() == false)
    {
        currentBlock.Move(0, count);
        return false;
    }
    return true;
}

bool Game::MoveBlockRight()
{
    currentBlock.Move(0, 1);
    if (IsBlockOutside() || BlockFits() == false)
    {
        currentBlock.Move(0, -1);
        return false;
    }
    return true;
}

bool Game::MoveBlockRightRepeat(int count)
{
    currentBlock.Move(0, count);
    if (IsBlockOutside() || BlockFits() == false)
    {
        currentBlock.Move(0, -count);
        return false;
    }
    return true;
}

bool Game::MoveBlockUpRepeat(int count)
{
    currentBlock.Move(-count, 0);
    if (IsBlockOutside() || BlockFits() == false)
    {
        currentBlock.Move(count, 0);
        return false;
    }
    return true;
}

void Game::MoveBlockDown()
{
    currentBlock.Move(1, 0);
    if (IsBlockOutside() || BlockFits() == false)
    {
        currentBlock.Move(-1, 0);
        lockBlock = true;
        // std::cout << "LM: " <<lockStateMoves << "\n";

        if (lockStateMoves >= maxLockStateMoves)
        {
            LockBlock();
        }
    }
    else
    {
        lockBlockTimer = 0.0f;
        lockBlock = false;
        lockStateMoves = 0;
    }
}

void Game::HardDropBlock()
{
    while (true)
    {
        currentBlock.Move(1, 0);
        if (IsBlockOutside() || BlockFits() == false)
        {
            currentBlock.Move(-1, 0);
            LockBlock();
            break;
        }
    }
}

void Game::SnakeDropBlock()
{
    while (true)
    {
        currentBlock.Move(1, 0);
        if (IsBlockOutside() || BlockFits() == false)
        {
            currentBlock.Move(-1, 0);
            lockBlock = true;
            break;
        }
    }
}

bool Game::CheckBlockInAir()
{
    Block testBlock = currentBlock;
    testBlock.Move(1, 0);
    if (IsBlockOutside(testBlock) || BlockFits(testBlock) == false)
    {
        return false;
    }
    return true;
}

bool Game::IsBlockOutside()
{
    std::vector<Position> tiles = currentBlock.GetCellPositions();
    for (Position item : tiles)
    {
        if (grid.IsCellOutside(item.row, item.column))
        {
            return true;
        }
    }
    return false;
}

bool Game::IsBlockOutside(Block block)
{
    std::vector<Position> tiles = block.GetCellPositions();
    for (Position item : tiles)
    {
        if (grid.IsCellOutside(item.row, item.column))
        {
            return true;
        }
    }
    return false;
}

void Game::TryToMoveBlockInside()
{
    std::vector<Position> tiles = currentBlock.GetCellPositions();

    int numMovesLeft = 0;
    int numMovesRight = 0;
    int numMovesUp = 0;
    int n;

    for (Position item : tiles)
    {
        if (item.column < 0)
        {
            n = item.column;
            if (numMovesRight < -n)
            {
                numMovesRight = -n;
                // std::cout << "numMovesRight: " << numMovesRight << "\n";
            }
        }
        else if (item.column > grid.GetNumCols() - 1)
        {
            n = item.column - (grid.GetNumCols() - 1);
            if (numMovesLeft < n)
            {
                numMovesLeft = n;
                // std::cout << "numMovesLeft: " << numMovesLeft << "\n";
            }
        }

        if (item.row > grid.GetNumRows() - 1)
        {
            n = item.row - (grid.GetNumRows() - 1);
            if (numMovesUp < n)
            {
                numMovesUp = n;
            }
        }
    }

    if (numMovesLeft)
    {
        MoveBlockLeftRepeat(numMovesLeft);
    }
    else if (numMovesRight)
    {
        MoveBlockRightRepeat(numMovesRight);
    }
    if (numMovesUp)
    {
        MoveBlockUpRepeat(numMovesUp);
    }
}

bool Game::RotateBlock()
{
    currentBlock.Rotate();
    if (IsBlockOutside())
    {
        TryToMoveBlockInside();
    }

    if (IsBlockOutside() || BlockFits() == false)
    {
        currentBlock.UndoRotation();
        return false;
    }
    // PlaySound(manipulateSound);
    return true;
}

void Game::LockBlock()
{
    if (CheckBlockInAir())
    {
        return;
    }

    std::vector<Position> tiles = currentBlock.GetCellPositions();
    for (Position item : tiles)
    {
        grid.grid[item.row][item.column] = currentBlock.id;
    }

    currentBlock = nextBlock;
    lockBlock = false;
    lockBlockTimer = 0.0f;
    lockStateMoves = 0;

    /*
        if (BlockFits() == false)
        {
            gameOver = true;
        }
        */

    nextBlock = GetRandomBlock();
    int numFullRows = grid.ClearFullRows();

    if (numFullRows > 0)
    {
        PlaySound(clearSound);
        UpdateScore(numFullRows);
    }
    else
    {

        // PlaySound(manipulateSound);
    }
}

void Game::UpdateScore(int clearedRows)
{
    score += 100 * clearedRows;

    if (score >= currentLevel * 1000)
    {
        currentLevel++;
    }

    /*
        if (score >= maxScore)
        {
            gameOver = true;
        }
        */
}

bool Game::BlockFits()
{
    std::vector<Position> tiles = currentBlock.GetCellPositions();

    for (Position item : tiles)
    {
        if (grid.IsCellEmpty(item.row, item.column) == false)
        {
            return false;
        }
    }
    return true;
}

bool Game::BlockFits(Block block)
{
    std::vector<Position> tiles = block.GetCellPositions();

    for (Position item : tiles)
    {
        if (grid.IsCellEmpty(item.row, item.column) == false)
        {
            return false;
        }
    }
    return true;
}
