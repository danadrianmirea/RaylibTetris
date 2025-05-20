# Raylib Tetris

A classic Tetris game implementation using the Raylib game development library.

Play on itch.io: https://adrianmirea.itch.io/

## Features

- Classic Tetris gameplay
- High score tracking
- Sound effects
- Smooth controls and animations
- Next block preview
- Score system

## Controls

- **Left Arrow**: Move block left
- **Right Arrow**: Move block right
- **Down Arrow**: Move block down
- **Up Arrow**: Rotate block
- **Space**: Hard drop (instantly drop the block)

## Requirements

- C++ compiler (GCC/Clang/MSVC)
- CMake (version 3.10 or higher)
- Raylib library

## Building the Game

1. Make sure you have Raylib installed on your system
2. Create a build directory and navigate to it:
   ```bash
   mkdir build
   cd build
   ```
3. Configure the project with CMake:
   ```bash
   cmake ..
   ```
4. Build the project:
   ```bash
   cmake --build .
   ```
5. The executable `RaylibTetris.exe` will be created in the build directory

## Project Structure

- `main.cpp`: Entry point of the game
- `game.cpp`/`game.h`: Main game logic and state management
- `grid.cpp`/`grid.h`: Grid management and collision detection
- `block.cpp`/`block.h`: Block class implementation
- `blocks.h`: Tetromino definitions
- `position.cpp`/`position.h`: Position handling
- `globals.cpp`/`globals.h`: Global game constants and utilities
- `Sounds/`: Directory containing game audio files
- `Font/`: Directory containing game fonts

## License

This project is licensed under the terms specified in the LICENSE.txt file.

Big thanks to https://github.com/educ8s (https://www.youtube.com/@programmingwithnick) for making great Raylib tutorials on youtube.
