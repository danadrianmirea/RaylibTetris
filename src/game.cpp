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
    targetRenderTex = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    if (!targetRenderTex.texture.id) {
        throw std::runtime_error("Failed to create render texture");
    }
    SetTextureFilter(targetRenderTex.texture, TEXTURE_FILTER_BILINEAR);

    grid = Grid();
    
    font = LoadFontEx("Font/monogram.ttf", 64, 0, 0);
    if (!font.texture.id) {
        throw std::runtime_error("Failed to load font");
    }
}

void Game::StartAudio()
{
    // Check if audio device is already initialized
    if (!IsAudioDeviceReady()) {
        InitAudioDevice();
        if (!IsAudioDeviceReady()) {
            return;
        }
    }
    
    SetMasterVolume(0.22f);

    // Load audio resources after device is initialized
    const char* rotatePath = "Sounds/rotate.mp3";
    const char* clearPath = "Sounds/clear.mp3";

    if (FileExists(rotatePath)) {
        manipulateSound = LoadSound(rotatePath);
    }

    if (FileExists(clearPath)) {
        clearSound = LoadSound(clearPath);
    }
}

void Game::InitGame()
{
    // Initialize grid first
    grid.Initialize();
    
    // Initialize blocks with error handling
    blocks = GetAllBlocks();
    currentBlock = GetRandomBlock();
    nextBlock = GetRandomBlock();
    
    // Initialize other game state
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
    // Calculate scale based on web canvas dimensions and desired scale
    float widthScale = (float)WEB_WIDTH * WEB_SCREEN_SCALE / gameScreenWidth;
    float heightScale = (float)WEB_HEIGHT * WEB_SCREEN_SCALE / gameScreenHeight;
    screenScale = MIN(widthScale, heightScale);
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
    std::vector<Block> allBlocks;
    allBlocks.reserve(7); // Pre-allocate space for all blocks    
    allBlocks.push_back(IBlock());
    allBlocks.push_back(JBlock());
    allBlocks.push_back(LBlock());
    allBlocks.push_back(OBlock());
    allBlocks.push_back(SBlock());
    allBlocks.push_back(TBlock());
    allBlocks.push_back(ZBlock());
    return allBlocks;
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
    // Check if resources are ready
    if (!font.texture.id) {
        return;
    }
    
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
        DrawRectangleRounded({(float)(GetScreenWidthWrapper() / 2 - 430), (float)(GetScreenHeightWrapper() / 2 - 120), 860, 410}, 0.76f, 20, BLACK);
        DrawText("RAYLIB TETRIS", GetScreenWidthWrapper() / 2 - 200, GetScreenHeightWrapper() / 2 - 100, 50, yellow);
        DrawText("Controls:", GetScreenWidthWrapper() / 2 - 200, GetScreenHeightWrapper() / 2 - 30, 40, yellow);
        DrawText("Left/Right Arrow or A/D: Move", GetScreenWidthWrapper() / 2 - 400, GetScreenHeightWrapper() / 2 + 30, 30, WHITE);
        DrawText("Up Arrow or W: Rotate", GetScreenWidthWrapper() / 2 - 400, GetScreenHeightWrapper() / 2 + 70, 30, WHITE);
        DrawText("Down Arrow or S: Soft Drop", GetScreenWidthWrapper() / 2 - 400, GetScreenHeightWrapper() / 2 + 110, 30, WHITE);
        DrawText("P: Pause", GetScreenWidthWrapper() / 2 - 400, GetScreenHeightWrapper() / 2 + 150, 30, WHITE);
        DrawText("Press ENTER to play", GetScreenWidthWrapper() / 2 - 200, GetScreenHeightWrapper() / 2 + 210, 40, yellow);
    }
    else if (paused)
    {
        DrawRectangleRounded({(float)(GetScreenWidthWrapper() / 2 - 500), (float)(GetScreenHeightWrapper() / 2 - 40), 1000, 120}, 0.76f, 20, BLACK);
    #ifndef EMSCRIPTEN_BUILD
        DrawText("Game paused, press P to continue", GetScreenWidthWrapper() / 2 - 400, GetScreenHeightWrapper() / 2, 40, yellow);
    #else
        DrawText("Game paused, press P or ESC to continue", GetScreenWidthWrapper() / 2 - 400, GetScreenHeightWrapper() / 2, 40, yellow);
    #endif
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
    return loadedHighScore;
}

std::string Game::FormatWithLeadingZeroes(int number, int width)
{
    if (width <= 0) {
        return "0";
    }
    
    std::string numberText = std::to_string(number);
    if (numberText.length() > width) {
        return std::string(width, '9');
    }
    
    int leadingZeros = width - numberText.length();
    if (leadingZeros < 0) {
        leadingZeros = 0;
    }
    
    return std::string(leadingZeros, '0') + numberText;
}

void Game::HandleInput()
{
    if (isFirstFrameAfterReset)
    {
        isFirstFrameAfterReset = false;
        return;
    }

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
}

void Game::UpdateUI()
{
#ifndef EMSCRIPTEN_BUILD
    if (WindowShouldClose() || (IsKeyPressed(KEY_ESCAPE) && exitWindowRequested == false) && !firstTimeGameStart)
    {
        exitWindowRequested = true;
        isInExitMenu = true;
        return;
    }
#endif

    if (firstTimeGameStart)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            firstTimeGameStart = false;
            // Initialize game state first
            InitGame();
            StartAudio();
        }
        return;
    }

    if (gameOver)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            gameOver = false;
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

#ifndef EMSCRIPTEN_BUILD
    if (exitWindowRequested == false && lostWindowFocus == false && gameOver == false && isFirstFrameAfterReset == false && IsKeyPressed(KEY_P))
#else
    if (exitWindowRequested == false && lostWindowFocus == false && gameOver == false && isFirstFrameAfterReset == false && (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)))
#endif
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
