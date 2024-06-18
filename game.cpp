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
    grid = Grid();
    font = LoadFontEx("Font/monogram.ttf", 64, 0, 0);
    InitAudioDevice();
    music = LoadMusicStream("Sounds/music.mp3");
    SetMusicVolume(music, 0.3f);
    PlayMusicStream(music);
    
    manipulateSound = LoadSound("Sounds/rotate.mp3");
    clearSound = LoadSound("Sounds/clear.mp3");
    Init();
}

Game::~Game()
{
    CloseAudioDevice();
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
    HandleInput();
    UpdateMusicStream(music);
    if (gameOver == false)
    {
        if (EventTriggered(0.9f / currentLevel))
        {
            MoveBlockDown();
        }
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

void Game::Draw()
{
    grid.Draw();
    currentBlock.Draw(0, 0);

    DrawUI();
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

    DrawTextEx(font, TextFormat("Level: %d", currentLevel), {345, 540}, fontSize, 2, WHITE);
}

void Game::HandleInput()
{
    if (gameOver)
    {
        int keyPress = GetKeyPressed();
        if (keyPress == KEY_SPACE)
        {
            Init();
        }
    }
    else
    {
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
                    //PlaySound(manipulateSound);
                }
                lastInputTime = 0.0f;
            }

            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
            {
                goodMove = MoveBlockRight();
                lastInputTime = 0.0f;
                if (goodMove)
                {
                    //PlaySound(manipulateSound);
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
        //std::cout << "LM: " <<lockStateMoves << "\n";
        if(lockStateMoves >= maxLockStateMoves)
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
                //std::cout << "numMovesRight: " << numMovesRight << "\n";
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
            if(numMovesUp < n)
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
    //PlaySound(manipulateSound);
    return true;
}

void Game::LockBlock()
{
    std::vector<Position> tiles = currentBlock.GetCellPositions();
    for (Position item : tiles)
    {
        grid.grid[item.row][item.column] = currentBlock.id;
    }

    currentBlock = nextBlock;
    lockBlock = false;
    lockBlockTimer = 0.0f;
    lockStateMoves = 0;

    if (BlockFits() == false)
    {
        gameOver = true;
    }

    nextBlock = GetRandomBlock();
    int numFullRows = grid.ClearFullRows();

    if (numFullRows > 0)
    {
        PlaySound(clearSound);
        UpdateScore(numFullRows);
    }
    else
    {

        //PlaySound(manipulateSound);
    }

}

void Game::UpdateScore(int clearedRows)
{
    score += 100 * clearedRows;

    if (score >= currentLevel * 1000)
    {
        currentLevel++;
    }

    if (score >= maxScore)
    {
        gameOver = true;
    }
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

void Game::Init()
{
    gameOver = false;
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
}
