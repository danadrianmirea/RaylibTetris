#include "block.h"

Block::Block()
{
    cellSize = 30;
    rotationState = 0;
    rowOffset = 0;
    columnOffset = 0;
    colors = GetCellColors();
    rowOffset = 0;
    columnOffset = 0;
}

void Block::Draw(int offsetX, int offsetY)
{
    std::vector<Position> tiles = GetCellPositions();
    static const int blockGridPadding = gridThickness+1;
    for(Position item: tiles)
    {
        DrawRectangle(offsetX + item.column * cellSize + 10 + blockGridPadding, offsetY + item.row * cellSize + 10 + blockGridPadding, cellSize - blockGridPadding, cellSize - blockGridPadding, colors[id]);
    }
}

void Block::Move(int rows, int columns)
{
    rowOffset += rows;
    columnOffset += columns;
}

std::vector<Position> Block::GetCellPositions()
{
    if (cells.find(rotationState) == cells.end()) {
        return std::vector<Position>(); // Return empty vector if rotation state doesn't exist
    }
    
    std::vector<Position> tiles = cells[rotationState];
    std::vector<Position> movedTiles;
    for(Position item : tiles)
    {
        Position newPos = Position(item.row + rowOffset, item.column + columnOffset);
        movedTiles.push_back(newPos);
    }
    return movedTiles;
}

void Block::Rotate()
{
    if (cells.empty()) return;
    
    rotationState++;
    if(rotationState == (int)cells.size())
    {
        rotationState = 0;
    }
}

void Block::UndoRotation()
{
    if (cells.empty()) return;
    
    rotationState--;
    if(rotationState == -1)
    {
        rotationState = cells.size() - 1;
    }
}
