# Raylib Tetris

A classic Tetris game implementation using the Raylib game development library.

## Features

- Classic Tetris gameplay
- High score tracking
- Sound effects and background music
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

- C++ compiler (GCC/Clang)
- Raylib library
- Make (for building)

## Building the Game

1. Make sure you have Raylib installed on your system
2. Run `make` in the project directory
3. The executable `game.exe` will be created

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
