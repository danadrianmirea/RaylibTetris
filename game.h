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
    bool IsBlockOutside(Block block);
    void TryToMoveBlockInside();
    bool RotateBlock();
    void LockBlock();
    void UpdateScore(int clearedRows);
    bool BlockFits();
    bool BlockFits(Block block);
    void Init();
    bool MoveBlockLeft();
    bool MoveBlockLeftRepeat(int count);
    bool MoveBlockRight();
    bool MoveBlockRightRepeat(int count);
    bool MoveBlockUpRepeat(int count);
    void MoveBlockDown();
    void HardDropBlock();
    void SnakeDropBlock();
    bool CheckBlockInAir();
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
    const float blockLockTime = 0.5f;
    int lockStateMoves;
    const int maxLockStateMoves = 5;
    int currentLevel;
    const int startingLevel = 1;
    const int maxScore = 10000;
    const float inputDelay = 0.1f;
    const float rotateInputDelay = 0.15f;
    const float softDropInputDelay = 0.1f;
};