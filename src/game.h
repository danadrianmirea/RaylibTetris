#pragma once
#include <string>
#include "globals.h"
#include "grid.h"
#include "blocks.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

class Game
{
public:
    Game();
    ~Game();
    void InitGame();
    void InitializeResources();
    void Reset();
    void StartAudio();

    Game(const Game &) = delete;
    const Game &operator=(const Game &g) = delete;
    Game(Game &&) = delete;
    Game &&operator=(Game &&g) = delete;

    void Update();
    void HandleInput();
    void UpdateUI();

    void Draw();
    void DrawUI();

    void CheckForHighScore();
    void SaveHighScoreToFile();
    int LoadHighScoreFromFile();

    std::string FormatWithLeadingZeroes(int number, int width);

    bool firstTimeGameStart;
    bool isFirstFrameAfterReset;
    bool isInExitMenu;
    bool paused;
    bool lostWindowFocus;
    bool gameOver;
    bool audioInitialized;
    bool isMobile;
    bool musicEnabled;
    float touchCollisionScale;
    float buttonSize;
    float buttonRadius;
    float buttonPadding;
    Color buttonColor;
    Color arrowColor;

private:
    Block GetRandomBlock();
    std::vector<Block> GetAllBlocks();
    bool IsBlockOutside();
    bool IsBlockOutside(Block block);
    void TryToMoveBlockInside();
    bool RotateBlock();
    void LockBlock();
    void UpdateScore(int clearedRows);
    bool BlockFits();
    bool BlockFits(Block block);

    bool MoveBlockLeft();
    bool MoveBlockLeftRepeat(int count);
    bool MoveBlockRight();
    bool MoveBlockRightRepeat(int count);
    bool MoveBlockUpRepeat(int count);
    void MoveBlockDown();
    void HardDropBlock();
    void SnakeDropBlock();
    bool CheckBlockInAir();
    Sound rotateSound;
    Sound clearSound;
    Sound dropSound;
    Sound lockSound;
    Music backgroundMusic;
    Grid grid;

    std::vector<Block> blocks;
    Block currentBlock;
    Block nextBlock;
    Font font;
    int score;
    int highScore;

    // input stuff
    float lastInputTime;
    float lastRotateInputTime;
    float lastSoftDropTimeTick;
    float lastDropAfterSpawnTime;
    bool lockBlock;
    bool firstDrop;
    float lockBlockTimer;
    const float blockLockTime = 0.5f;
    int lockStateMoves;
    const int maxLockStateMoves = 5;
    int currentLevel;
    const int startingLevel = 1;
    const int maxScore = 10000;
    const float inputDelay = 0.1f;
    const float rotateInputDelay = 0.15f;
    const float softDropInputDelay = 0.1f;
    const float dropAfterSpawnDelay = 0.3f;

    float screenScale;
    RenderTexture2D targetRenderTex;

    // Mobile touch input functions
    bool CheckTouchInUpButton();
    bool CheckTouchInDownButton();
    bool CheckTouchInLeftButton();
    bool CheckTouchInRightButton();
    bool CheckTouchInCenter();

    bool exitWindowRequested;
};