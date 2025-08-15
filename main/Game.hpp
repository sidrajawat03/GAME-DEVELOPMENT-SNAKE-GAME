#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <memory>
#include <random>
#include <string>

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct Position {
    int x, y;
    
    Position(int x = 0, int y = 0) : x(x), y(y) {}
    
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
    
    Position operator+(const Position& other) const {
        return Position(x + other.x, y + other.y);
    }
};

class Snake {
private:
    std::vector<Position> body;
    Direction direction;
    Direction nextDirection;
    
public:
    Snake();
    void move();
    void grow();
    void setDirection(Direction dir);
    bool checkSelfCollision() const;
    const std::vector<Position>& getBody() const { return body; }
    const Position& getHead() const { return body.front(); }
    void reset();
};

class Fruit {
private:
    Position position;
    std::mt19937 rng;
    std::uniform_int_distribution<int> xDist;
    std::uniform_int_distribution<int> yDist;
    
public:
    Fruit(int gridWidth, int gridHeight);
    void respawn(const std::vector<Position>& snakeBody);
    const Position& getPosition() const { return position; }
};

class AudioManager {
private:
    sf::SoundBuffer eatBuffer;
    sf::SoundBuffer gameOverBuffer;
    sf::SoundBuffer moveBuffer;
    std::unique_ptr<sf::Sound> eatSound;
    std::unique_ptr<sf::Sound> gameOverSound;
    std::unique_ptr<sf::Sound> moveSound;
    std::unique_ptr<sf::Music> music;
    bool soundEnabled;
    bool musicEnabled;
    static constexpr float MUSIC_VOLUME = 15.f; // percent (0-100)
    
public:
    AudioManager();
    bool loadSounds();
    void playEatSound();
    void playGameOverSound();
    void playMoveSound();
    void startMusic();
    void stopMusic();
    void toggleSound() { soundEnabled = !soundEnabled; }
    void toggleMusic();
    bool isSoundEnabled() const { return soundEnabled; }
    bool isMusicEnabled() const { return musicEnabled; }
};

class Game {
private:
    sf::RenderWindow window;
    sf::Font font;
    std::unique_ptr<sf::Text> scoreText;
    std::unique_ptr<sf::Text> gameOverText;
    std::unique_ptr<sf::Text> menuText;
    std::unique_ptr<sf::Text> pauseText;
    
    Snake snake;
    Fruit fruit;
    AudioManager audioManager;
    
    GameState gameState;
    int score;
    float gameSpeed;
    sf::Clock gameClock;
    sf::Time lastUpdate;
    std::vector<Position> prevSnakeBody; // for interpolation

    // Textures & sprites
    sf::Texture bgTexture;
    sf::Texture headTexture;
    sf::Texture bodyTexture;
    sf::Texture fruitTexture;
    std::unique_ptr<sf::Sprite> backgroundSprite;
    bool texturesLoaded = false;
    bool fruitTextureLoaded = false;

    // Font management
    std::vector<std::string> fontPaths;
    int currentFontIndex = 0;
    void applyFont();
    
    // Grid settings
    static const int GRID_WIDTH = 40;
    static const int GRID_HEIGHT = 30;
    static const int CELL_SIZE = 20;
    static const int WINDOW_WIDTH = GRID_WIDTH * CELL_SIZE;
    static const int WINDOW_HEIGHT = GRID_HEIGHT * CELL_SIZE;
    
    // Game settings
    static constexpr float BASE_SPEED = 150.0f; // milliseconds per move
    static constexpr float SPEED_INCREASE = 5.0f;
    static constexpr float HEAD_SCALE = 1.4f; // enlarge head sprite for visibility (1.0 = fit cell)
    static constexpr float FRUIT_SCALE = 1.4f; // enlarge fruit sprite
    
public:
    Game();
    bool initialize();
    void run();
    
private:
    void handleEvents();
    void update();
    void render();
    void resetGame();
    void updateScore();
    void drawGrid();
    void drawSnake();
    void drawFruit();
    void drawUI();
    bool isValidPosition(const Position& pos) const;
    sf::Vector2f gridToPixel(const Position& pos) const;
};
