#include <iostream>
#include <fstream>
#include <random>
#include <string>

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
    isFirstFrameAfterReset = false;
    isInExitMenu = false;
    paused = false;
    lostWindowFocus = false;
    gameOver = false;
}

void Game::InitializeResources()
{
    // Initialize other resources
    targetRenderTex = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(targetRenderTex.texture, TEXTURE_FILTER_BILINEAR); // Texture scale filter to use

    grid = Grid();
    font = LoadFontEx("Font/monogram.ttf", 64, 0, 0);
}

void Game::StartAudio()
{
    std::cout << "Initializing audio..." << std::endl;
    
    // Check if audio device is already initialized
    if (!IsAudioDeviceReady()) {
        InitAudioDevice();
        if (!IsAudioDeviceReady()) {
            std::cerr << "Failed to initialize audio device!" << std::endl;
            return;  // Don't close window, just return
        }
    }
    std::cout << "Audio initialized successfully" << std::endl;
    
    SetMasterVolume(0.22f);

    // Load audio resources after device is initialized
    const char* rotatePath = "Sounds/rotate.mp3";
    const char* clearPath = "Sounds/clear.mp3";

    if (FileExists(rotatePath)) {
        manipulateSound = LoadSound(rotatePath);
    } else {
        std::cerr << "Warning: Rotate sound file not found: " << rotatePath << std::endl;
    }

    if (FileExists(clearPath)) {
        clearSound = LoadSound(clearPath);
    } else {
        std::cerr << "Warning: Clear sound file not found: " << clearPath << std::endl;
    }
}

void Game::InitGame()
{
    grid.Initialize();
    blocks = GetAllBlocks();
    currentBlock = GetRandomBlock();  // Initialize currentBlock
    nextBlock = GetRandomBlock();
    score = 0;
    highScore = LoadHighScoreFromFile();
    lockBlockTimer = 0.0f;
    lockBlock = false;
    lockStateMoves = 0;
    currentLevel = startingLevel;
    lastInputTime = inputDelay;
    lastRotateInputTime = rotateInputDelay;
    lastSoftDropTimeTick = softDropInputDelay;

#ifndef EMSCRIPTEN_BUILD
    screenScale = MIN((float)GetScreenWidth() / gameScreenWidth, (float)GetScreenHeight() / gameScreenHeight);
#else
    screenScale = WEB_SCREEN_SCALE;
#endif
}

void Game::Reset()
{
    InitGame();
}

Game::~Game()
{
    UnloadRenderTexture(targetRenderTex);
    UnloadFont(font);
    UnloadSound(manipulateSound);
    UnloadSound(clearSound);
}

Block Game::GetRandomBlock()
{
    if (blocks.empty())
    {
        blocks = GetAllBlocks();
    }
    
    if (blocks.empty()) {
        // Return a default block if somehow we have no blocks
        return Block();
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
#ifndef EMSCRIPTEN_BUILD
    screenScale = MIN((float)GetScreenWidthWrapper() / gameScreenWidth, (float)GetScreenHeightWrapper() / gameScreenHeight);
#endif

    UpdateUI();


    bool running = (firstTimeGameStart == false && paused == false && lostWindowFocus == false && isInExitMenu == false && gameOver == false);

    
    if (running)
    {
        HandleInput();        
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
                   (Rectangle){(GetScreenWidthWrapper() - ((float)gameScreenWidth * screenScale)) * 0.5f, (GetScreenHeightWrapper() - ((float)gameScreenHeight * screenScale)) * 0.5f, (float)gameScreenWidth * screenScale, (float)gameScreenHeight * screenScale},
                   (Vector2){0, 0}, 0.0f, WHITE);
    DrawScreenSpaceUI();
    EndDrawing();
}

void Game::DrawUI()
{
    const int fontSize = 30;

    DrawTextEx(font, "Score", {365, 15}, fontSize, 2, WHITE);
    DrawRectangleRounded(Rectangle{320, 55, 170, 60}, 0.3, 6, lightBlue);
    std::string scoreText = FormatWithLeadingZeroes(score, 7);
    DrawTextEx(font, scoreText.c_str(), {355, 65}, fontSize, 2, WHITE);

    DrawTextEx(font, "High Score", {325, 135}, fontSize, 2, WHITE);
    DrawRectangleRounded(Rectangle{320, 175, 170, 60}, 0.3, 6, lightBlue);
    std::string highScoreText = FormatWithLeadingZeroes(highScore, 7);
    DrawTextEx(font, highScoreText.c_str(), {355, 185}, fontSize, 2, WHITE);

    DrawRectangleRounded(Rectangle{320, 275, 170, 180}, 0.3, 6, lightBlue);
    DrawTextEx(font, "Next", {365, 275}, fontSize, 2, WHITE);
    nextBlock.Draw(260, 340);

    DrawTextEx(font, TextFormat("Level: %d", currentLevel), {345, 540}, fontSize, 2, WHITE);
}

void Game::DrawScreenSpaceUI()
{
    if (exitWindowRequested)
    {
        DrawRectangleRounded({(float)(GetScreenWidthWrapper() / 2 - 500), (float)(GetScreenHeightWrapper() / 2 - 40), 1000, 120}, 0.76f, 20, BLACK);
        DrawText("Are you sure you want to exit? [Y/N]", GetScreenWidthWrapper() / 2 - 400, GetScreenHeightWrapper() / 2, 40, yellow);
    }
    else if (firstTimeGameStart)
    {
        DrawRectangleRounded({(float)(GetScreenWidthWrapper() / 2 - 500), (float)(GetScreenHeightWrapper() / 2 - 40), 1000, 120}, 0.76f, 20, BLACK);
        DrawText("Press ENTER to play", GetScreenWidthWrapper() / 2 - 200, GetScreenHeightWrapper() / 2, 40, yellow);
    }
    else if (paused)
    {
        DrawRectangleRounded({(float)(GetScreenWidthWrapper() / 2 - 500), (float)(GetScreenHeightWrapper() / 2 - 40), 1000, 120}, 0.76f, 20, BLACK);
        DrawText("Game paused, press P to continue", GetScreenWidthWrapper() / 2 - 400, GetScreenHeightWrapper() / 2, 40, yellow);
    }
    else if (lostWindowFocus)
    {
        DrawRectangleRounded({(float)(GetScreenWidthWrapper() / 2 - 500), (float)(GetScreenHeightWrapper() / 2 - 40), 1000, 120}, 0.76f, 20, BLACK);
        DrawText("Game paused, focus window to continue", GetScreenWidthWrapper() / 2 - 400, GetScreenHeightWrapper() / 2, 40, yellow);
    }
    else if (gameOver)
    {
        DrawRectangleRounded({(float)(GetScreenWidthWrapper() / 2 - 500), (float)(GetScreenHeightWrapper() / 2 - 40), 1000, 120}, 0.76f, 20, BLACK);
        DrawText("Game over, press ENTER to play again", GetScreenWidthWrapper() / 2 - 400, GetScreenHeightWrapper() / 2, 40, yellow);
    }
}

void Game::CheckForHighScore()
{
    if (score > highScore)
    {
        highScore = score;
        SaveHighScoreToFile();
    }
}

void Game::SaveHighScoreToFile()
{
    std::ofstream highScoreFile("highscore.txt");
    if (highScoreFile.is_open())
    {
        highScoreFile << highScore;
        highScoreFile.close();
    }
    else
    {
        std::cerr << "Failed to save highscore to file \n";
    }
}

int Game::LoadHighScoreFromFile()
{
    int loadedHighScore = 0;
    std::ifstream highscoreFile("highscore.txt");
    if (highscoreFile.is_open())
    {
        highscoreFile >> loadedHighScore;
        highscoreFile.close();
    }
    else
    {
#ifndef EMSCRIPTEN_BUILD
        std::cerr << "Failed to load highscore from file\n";
#endif
    }
    return loadedHighScore;
}

std::string Game::FormatWithLeadingZeroes(int number, int width)
{
    std::string numberText = std::to_string(number);
    int leadingZeros = width - numberText.length();
    numberText = std::string(leadingZeros, '0') + numberText;
    return numberText;
}

void Game::HandleInput()
{

    if (isFirstFrameAfterReset)
    {
        isFirstFrameAfterReset = false;
        return;
    }

    /*
    if (gameOver)
    {
        int keyPress = GetKeyPressed();
        if (keyPress == KEY_ENTER)
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

    if (firstTimeGameStart)
    {
        std::cout << "Waiting for ENTER key press... firstTimeGameStart=" << firstTimeGameStart << std::endl;
        if (IsKeyPressed(KEY_ENTER))
        {
            std::cout << "ENTER key pressed, starting game..." << std::endl;
            firstTimeGameStart = false;
            StartAudio();  // Start audio on first user interaction
            InitGame();    // Initialize game state when starting
        }
        return;
    }

    if (gameOver)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            Reset();
        }
        return;
    }


#ifdef AM_RAY_DEBUG
#ifndef EMSCRIPTEN_BUILD
    if (IsKeyPressed(KEY_ENTER) && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
    {
        if (fullscreen)
        {
            fullscreen = false;
            ToggleBorderlessWindowed();
            SetWindowPosition(minimizeOffset, minimizeOffset);
        }
        else
        {
            fullscreen = true;
            ToggleBorderlessWindowed();
        }
    }
#endif
#endif

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

        // PlaySound(manipulateSound);
    }
}

void Game::UpdateScore(int clearedRows)
{
    score += 100 * clearedRows;

    if (score >= currentLevel * 1000)
    {
        currentLevel++;
        if (currentLevel > 10)
        {
            currentLevel = 10;
        }
    }

    CheckForHighScore();
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
