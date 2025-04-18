#include "grid.h"
#include "globals.h"


Grid::Grid()
{
    numRows = defNumRows;
    numCols = defNumCols;
    cellSize = defCellSize;
    colors = GetCellColors();
    Initialize();
}

void Grid::Initialize()
{
    // Clear the grid
    for (int row = 0; row < numRows; row++)
    {
        for (int col = 0; col < numCols; col++)
        {
            grid[row][col] = 0;
        }
    }
}

void Grid::Print()
{
    for (int row = 0; row < numRows; row++)
    {
        for (int col = 0; col < numCols; col++)
        {
            std::cout << grid[row][col] << " ";
        }
        std::cout << "\n";
    }
}

void Grid::Draw()
{
    for (int row = 0; row < numRows; row++)
    {
        for (int col = 0; col < numCols; col++)
        {
            int cellValue = grid[row][col]; 
            DrawRectangle(col * cellSize + 11, row * cellSize + 11, cellSize - 1, cellSize - 1, colors[cellValue]);
        }
    }
}

bool Grid::IsCellOutside(int row, int column)
{
    if (row >= 0 && row < numRows && column >= 0 && column < numCols)
    {
        return false;
    }
    return true;
}

bool Grid::IsValidPosition(int row, int col) const
{
    return row >= 0 && row < numRows && col >= 0 && col < numCols;
}

bool Grid::IsCellEmpty(int row, int column)
{
    if (!IsValidPosition(row, column)) {
        return true; // Consider out-of-bounds cells as empty
    }
    return grid[row][column] == 0;
}

int Grid::ClearFullRows()
{
    int completed = 0;

    for (int row = numRows - 1; row >= 0; row--)
    {
        if (IsRowFull(row))
        {
            ClearRow(row);
            completed++;
        }
        else if (completed > 0)
        {
            MoveRowDown(row, completed);
        }
    }

    return completed;
}

int Grid::GetNumCols()
{
    return numCols;
}

int Grid::GetNumRows()
{
    return numRows;
}

bool Grid::IsRowFull(int row)
{
    for (int column = 0; column < numCols; column++)
    {
        if (grid[row][column] == 0)
        {
            return false;
        }
    }
    return true;
}

void Grid::ClearRow(int row)
{
    for (int column = 0; column < numCols; column++)
    {
        grid[row][column] = 0;
    }
}

void Grid::MoveRowDown(int row, int numRowsToMove)
{
    if (!IsValidPosition(row, 0) || !IsValidPosition(row + numRowsToMove, 0)) {
        return; // Don't move if source or destination is out of bounds
    }
    
    for (int column = 0; column < numCols; column++)
    {
        grid[row + numRowsToMove][column] = grid[row][column];
        grid[row][column] = 0;
    }
}
