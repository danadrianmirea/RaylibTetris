#pragma once

#include <iostream>
#include <vector>
#include <raylib.h>


const int defNumRows = 20;
const int defNumCols = 10;
const int defCellSize = 30;

class Grid
{
    public:
        Grid();
        void Initialize();
        void Print();
        void Draw();
        bool IsCellOutside(int row, int column);
        bool IsCellEmpty(int row, int column);
        int ClearFullRows();
        int grid[defNumRows][defNumCols];
        int GetNumCols();
        int GetNumRows();

    private:
        bool IsRowFull(int row);
        void ClearRow(int row);
        void MoveRowDown(int row, int numRows);
        bool IsValidPosition(int row, int col) const;
        int numRows;
        int numCols;
        int cellSize;
        std::vector<Color> colors;
};