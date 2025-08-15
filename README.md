# Modern Snake Game
**COMPANY:** Codtech IT Solutions  
**NAME:** Siddharth singh Rajawat
**INTERN ID:** CT04DZ1977
**DOMAIN:** C++ Programming 
**DURATION:** 4 Weeks  
**MENTOR:** Neela Santhosh  

## 📖 Overview

This project is a visually engaging implementation of the classic **Snake Game** using **C++ and SFML**. It demonstrates your ability to build an interactive graphical application with modular design, real-time input handling, and dynamic gameplay elements.

## Features

- **Modern C++ Architecture**: Clean class-based design with proper encapsulation
- **Multiple Game States**: Menu, Playing, Paused, Game Over
- **Progressive Difficulty**: Speed increases as score grows
- **Audio System**: Sound effects with toggle capability
- **Visual Enhancements**: Grid display, different colors for snake head/body
- **Robust Input Handling**: Event-based input with anti-reverse protection
- **Error Handling**: Graceful handling of missing assets
- **Score System**: Real-time score and speed display

## Controls

- **Arrow Keys**: Move snake
- **SPACE**: Start game / Restart after game over
- **P**: Pause/Resume game
- **S**: Toggle sound on/off
- **ESC**: Quit game

## Requirements

- SFML 2.5 or higher
- C++17 compatible compiler
- macOS with Homebrew (as configured)

## Building

```bash
# Make sure SFML is installed via Homebrew
brew install sfml

# Compile the game
g++ -std=c++17 -I/opt/homebrew/include -L/opt/homebrew/lib \
    -lsfml-graphics -lsfml-audio -lsfml-system -lsfml-window \
    main.cpp Game.cpp -o snake_game

# Run the game
./snake_game
```

## Architecture

### Classes

- **Game**: Main game controller handling states, rendering, and game loop
- **Snake**: Snake entity with movement and collision logic
- **Fruit**: Fruit spawning and collision detection
- **AudioManager**: Sound system with toggle functionality

### Design Patterns

- **State Pattern**: Clean game state management
- **Entity-Component**: Separation of game objects and behaviors
- **RAII**: Automatic resource management
- **Error Handling**: Graceful degradation for missing assets

## File Structure

```
.
├── main.cpp          # Entry point
├── Game.hpp          # Game class declaration
├── Game.cpp          # Game class implementation
├── eat.wav           # Eating sound effect
├── gameover.wav      # Game over sound effect
└── README.md         # This file
```
## Gallery

<img width="793" height="627" alt="image" src="https://github.com/user-attachments/assets/2735ac1b-f44b-469c-b25c-a09850d3436b" />

<img width="789" height="619" alt="image" src="https://github.com/user-attachments/assets/778122eb-6934-41f1-bbe3-cbb1db3d59ea" />


