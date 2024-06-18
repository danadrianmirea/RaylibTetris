#pragma once

#include "grid.h"
#include "blocks.h"

class Game
{
public:
    Game();
    ~Game();
    void Update();
    void Draw();
    void DrawUI();
    void HandleInput();
    bool gameOver;
    Music music;

private:
    Block GetRandomBlock();
    std::vector<Block> GetAllBlocks();
    bool IsBlockOutside();
    void TryToMoveBlockInside();
    bool RotateBlock();
    void LockBlock();
    void UpdateScore(int clearedRows);
    bool BlockFits();
    void Init();
    bool MoveBlockLeft();
    bool MoveBlockLeftRepeat(int count);
    bool MoveBlockRight();
    bool MoveBlockRightRepeat(int count);
    void MoveBlockDown();
    void HardDropBlock();
    void SnakeDropBlock();
    Sound manipulateSound;
    Sound clearSound;
    Grid grid;

    std::vector<Block> blocks;
    Block currentBlock;
    Block nextBlock;
    Font font;
    int score;

    // input stuff
    float lastInputTime;
    float lastRotateInputTime;
    float lastSoftDropTimeTick;
    bool lockBlock;
    float lockBlockTimer;
    int lockStateMoves;
    const int maxLockStateMoves = 15;
    const float blockLockTime = 0.5f;
    int currentLevel;
    const int startingLevel = 5;
    const int maxScore = 10000;
    const float inputDelay = 0.15f;
    const float rotateInputDelay = 0.2f;
    const float softDropInputDelay = 0.1f;
};