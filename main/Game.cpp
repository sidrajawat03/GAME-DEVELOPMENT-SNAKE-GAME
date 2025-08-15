#include "Game.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>

// Snake Implementation
Snake::Snake() : direction(Direction::RIGHT), nextDirection(Direction::RIGHT) {
    reset();
}

void Snake::reset() {
    body.clear();
    body.push_back(Position(20, 15)); // Center of grid
    body.push_back(Position(19, 15));
    body.push_back(Position(18, 15));
    direction = Direction::RIGHT;
    nextDirection = Direction::RIGHT;
}

void Snake::move() {
    direction = nextDirection;
    
    Position directionVector;
    switch (direction) {
        case Direction::UP:    directionVector = Position(0, -1); break;
        case Direction::DOWN:  directionVector = Position(0, 1); break;
        case Direction::LEFT:  directionVector = Position(-1, 0); break;
        case Direction::RIGHT: directionVector = Position(1, 0); break;
    }
    
    Position newHead = body.front() + directionVector;
    body.insert(body.begin(), newHead);
    body.pop_back();
}

void Snake::grow() {
    // Don't remove the tail on next move
    Position directionVector;
    switch (direction) {
        case Direction::UP:    directionVector = Position(0, -1); break;
        case Direction::DOWN:  directionVector = Position(0, 1); break;
        case Direction::LEFT:  directionVector = Position(-1, 0); break;
        case Direction::RIGHT: directionVector = Position(1, 0); break;
    }
    
    Position newHead = body.front() + directionVector;
    body.insert(body.begin(), newHead);
}

void Snake::setDirection(Direction dir) {
    // Prevent 180-degree turns
    bool canChangeDirection = false;
    switch (dir) {
        case Direction::UP:    canChangeDirection = (direction != Direction::DOWN); break;
        case Direction::DOWN:  canChangeDirection = (direction != Direction::UP); break;
        case Direction::LEFT:  canChangeDirection = (direction != Direction::RIGHT); break;
        case Direction::RIGHT: canChangeDirection = (direction != Direction::LEFT); break;
    }
    
    if (canChangeDirection) {
        nextDirection = dir;
    }
}

bool Snake::checkSelfCollision() const {
    const Position& head = body.front();
    for (size_t i = 1; i < body.size(); ++i) {
        if (head == body[i]) {
            return true;
        }
    }
    return false;
}

// Fruit Implementation
Fruit::Fruit(int gridWidth, int gridHeight) 
    : rng(std::random_device{}())
    , xDist(0, gridWidth - 1)
    , yDist(0, gridHeight - 1) {
    position = Position(xDist(rng), yDist(rng));
}

void Fruit::respawn(const std::vector<Position>& snakeBody) {
    do {
        position = Position(xDist(rng), yDist(rng));
    } while (std::find(snakeBody.begin(), snakeBody.end(), position) != snakeBody.end());
}

// AudioManager Implementation
AudioManager::AudioManager() : eatSound(), gameOverSound(), moveSound(), music(), soundEnabled(true), musicEnabled(true) {}

bool AudioManager::loadSounds() {
    bool success = true;
    auto tryLoadSound = [&](sf::SoundBuffer& buf, const std::vector<std::string>& names) -> bool {
        for (const auto& n : names) {
            if (buf.loadFromFile(n)) return true;
        }
        return false;
    };

    // Base directory for our known assets
    const std::string base = "assets/audios/";
    if (tryLoadSound(eatBuffer, { base + "eat.wav", base + "eat.ogg", base + "eat.mp3" })) {
        eatSound = std::make_unique<sf::Sound>(eatBuffer);
    } else { std::cerr << "Warning: eat sound not found in " << base << std::endl; success = false; }

    if (tryLoadSound(gameOverBuffer, { base + "gameover.wav", base + "gameover.ogg", base + "gameover.mp3" })) {
        gameOverSound = std::make_unique<sf::Sound>(gameOverBuffer);
    } else { std::cerr << "Warning: gameover sound not found in " << base << std::endl; success = false; }

    if (tryLoadSound(moveBuffer, { base + "move.wav", base + "move.ogg", base + "move.mp3" })) {
        moveSound = std::make_unique<sf::Sound>(moveBuffer);
    } // optional, no warning

    music = std::make_unique<sf::Music>();
    if (musicEnabled) {
        if (!(music->openFromFile(base + "bgmusic.ogg") || music->openFromFile(base + "bgmusic.mp3") || music->openFromFile(base + "bgmusic.wav"))) {
            std::cerr << "Info: background music not found in " << base << std::endl;
            musicEnabled = false;
        } else {
#ifdef SFML_AUDIO_VERSION_3
            music->setLooping(true);
#endif
            music->setVolume(MUSIC_VOLUME);
            music->play();
        }
    }
    if (!success) soundEnabled = false;
    return success;
}

void AudioManager::playEatSound() {
    if (soundEnabled && eatSound) {
        eatSound->play();
    }
}

void AudioManager::playGameOverSound() {
    if (soundEnabled && gameOverSound) {
        gameOverSound->play();
    }
}

void AudioManager::playMoveSound() {
    if (soundEnabled && moveSound) {
        if (moveSound->getStatus() != sf::SoundSource::Status::Playing) {
            moveSound->play();
        }
    }
}

void AudioManager::startMusic() {
    if (musicEnabled && music && music->getStatus() != sf::SoundSource::Status::Playing) {
#ifdef SFML_AUDIO_VERSION_3
        music->setLooping(true);
#endif
    music->setVolume(MUSIC_VOLUME);
        music->play();
    }
}

void AudioManager::stopMusic() {
    if (music && music->getStatus() == sf::SoundSource::Status::Playing) music->stop();
}

void AudioManager::toggleMusic() {
    musicEnabled = !musicEnabled;
    if (musicEnabled) startMusic(); else stopMusic();
}

// Game Implementation
Game::Game() 
    : window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Modern Snake Game", sf::Style::Titlebar | sf::Style::Close)
    , fruit(GRID_WIDTH, GRID_HEIGHT)
    , gameState(GameState::MENU)
    , score(0)
    , gameSpeed(BASE_SPEED)
    , lastUpdate(sf::Time::Zero) {
}

bool Game::initialize() {
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);
    
    // Discover TTF fonts in assets/ttf
    fontPaths.clear();
    try {
        for (const auto& entry : std::filesystem::directory_iterator("assets/ttf")) {
            if (entry.is_regular_file()) {
                auto p = entry.path();
                if (p.extension() == ".ttf") fontPaths.push_back(p.string());
            }
        }
    } catch (...) {}
    if (fontPaths.empty()) {
        fontPaths.push_back("assets/DejaVuSans.ttf"); // fallback candidate
    }
    if (!font.openFromFile(fontPaths[0])) {
        std::cerr << "Warning: Failed to load initial font: " << fontPaths[0] << std::endl;
    }
    
    // Initialize text objects (construct after font load for SFML 3)
    scoreText = std::make_unique<sf::Text>(font, "Score: 0 | Speed: 0", 24);
    scoreText->setFillColor(sf::Color::White);
    scoreText->setPosition({10, 10});

    gameOverText = std::make_unique<sf::Text>(font, "GAME OVER", 48);
    gameOverText->setFillColor(sf::Color::Red);
    sf::FloatRect textRect = gameOverText->getLocalBounds();
    gameOverText->setOrigin({textRect.position.x + textRect.size.x / 2.0f, textRect.position.y + textRect.size.y / 2.0f});
    gameOverText->setPosition({WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 50});
    
    menuText = std::make_unique<sf::Text>(font, "Press SPACE to Start\nArrow Keys to Move\nP to Pause\nS to Toggle Sound\nESC to Quit", 24);
    menuText->setFillColor(sf::Color::White);
    textRect = menuText->getLocalBounds();
    menuText->setOrigin({textRect.position.x + textRect.size.x / 2.0f, textRect.position.y + textRect.size.y / 2.0f});
    menuText->setPosition({WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f});
    
    pauseText = std::make_unique<sf::Text>(font, "PAUSED", 48);
    pauseText->setFillColor(sf::Color::Yellow);
    textRect = pauseText->getLocalBounds();
    pauseText->setOrigin({textRect.position.x + textRect.size.x / 2.0f, textRect.position.y + textRect.size.y / 2.0f});
    pauseText->setPosition({WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f});
    
    // Load textures
    texturesLoaded = bgTexture.loadFromFile("assets/imgs/bg.png") &&
                     headTexture.loadFromFile("assets/imgs/head.png") &&
                     bodyTexture.loadFromFile("assets/imgs/body.png");
    fruitTextureLoaded = fruitTexture.loadFromFile("assets/imgs/fruit.png");
    if (texturesLoaded) {
        backgroundSprite = std::make_unique<sf::Sprite>(bgTexture);
        // Scale background to cover window (maintain aspect ratio, center)
        auto texSize = bgTexture.getSize();
        float scaleX = (float)WINDOW_WIDTH / texSize.x;
        float scaleY = (float)WINDOW_HEIGHT / texSize.y;
        float scale = std::max(scaleX, scaleY); // cover
        backgroundSprite->setScale(sf::Vector2f(scale, scale));
        // center
        float drawnW = texSize.x * scale;
        float drawnH = texSize.y * scale;
        backgroundSprite->setPosition(sf::Vector2f((WINDOW_WIDTH - drawnW) / 2.f, (WINDOW_HEIGHT - drawnH) / 2.f));
    } else {
        std::cerr << "Warning: Could not load one or more textures." << std::endl;
    }
    if (!fruitTextureLoaded) {
        std::cerr << "Warning: Could not load fruit texture." << std::endl;
    }

    // Load audio
    audioManager.loadSounds();
    // Set initial score text
    // score text already set

    return true;
}

void Game::applyFont() {
    if (fontPaths.empty()) return;
    if (!font.openFromFile(fontPaths[currentFontIndex])) {
        std::cerr << "Warning: Could not load font: " << fontPaths[currentFontIndex] << std::endl;
        return;
    }
    if (scoreText) scoreText->setFont(font);
    if (gameOverText) gameOverText->setFont(font);
    if (menuText) menuText->setFont(font);
    if (pauseText) pauseText->setFont(font);
}

void Game::run() {
    if (!initialize()) {
        std::cerr << "Failed to initialize game!" << std::endl;
        return;
    }
    while (window.isOpen()) {
        handleEvents();
        update();
        render();
    }
}

void Game::handleEvents() {
    while (auto evOpt = window.pollEvent()) {
        const sf::Event& event = *evOpt;
        if (event.is<sf::Event::Closed>()) {
            window.close();
            continue;
        }
        if (auto keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            switch (keyPressed->code) {
                case sf::Keyboard::Key::Escape:
                    window.close();
                    break;
                case sf::Keyboard::Key::Space:
                    if (gameState == GameState::MENU || gameState == GameState::GAME_OVER) {
                        resetGame();
                        gameState = GameState::PLAYING;
                    }
                    break;
                case sf::Keyboard::Key::P:
                    if (gameState == GameState::PLAYING) {
                        gameState = GameState::PAUSED;
                    } else if (gameState == GameState::PAUSED) {
                        gameState = GameState::PLAYING;
                    }
                    break;
                case sf::Keyboard::Key::S:
                    audioManager.toggleSound();
                    break;
                case sf::Keyboard::Key::M:
                    audioManager.toggleMusic();
                    break;
                case sf::Keyboard::Key::F:
                    if (!fontPaths.empty()) {
                        currentFontIndex = (currentFontIndex + 1) % fontPaths.size();
                        applyFont();
                    }
                    break;
                case sf::Keyboard::Key::Up:
                    if (gameState == GameState::PLAYING) { snake.setDirection(Direction::UP); audioManager.playMoveSound(); }
                    break;
                case sf::Keyboard::Key::Down:
                    if (gameState == GameState::PLAYING) { snake.setDirection(Direction::DOWN); audioManager.playMoveSound(); }
                    break;
                case sf::Keyboard::Key::Left:
                    if (gameState == GameState::PLAYING) { snake.setDirection(Direction::LEFT); audioManager.playMoveSound(); }
                    break;
                case sf::Keyboard::Key::Right:
                    if (gameState == GameState::PLAYING) { snake.setDirection(Direction::RIGHT); audioManager.playMoveSound(); }
                    break;
                default:
                    break;
            }
        }
    }
}

void Game::update() {
    if (gameState != GameState::PLAYING) {
        return;
    }
    
    sf::Time elapsed = gameClock.getElapsedTime();
    if (elapsed - lastUpdate >= sf::milliseconds(static_cast<int>(gameSpeed))) {
    prevSnakeBody = snake.getBody();
    snake.move();
        
        // Check wall collision
        const Position& head = snake.getHead();
        if (!isValidPosition(head)) {
            audioManager.playGameOverSound();
            gameState = GameState::GAME_OVER;
            return;
        }
        
        // Check self collision
        if (snake.checkSelfCollision()) {
            audioManager.playGameOverSound();
            gameState = GameState::GAME_OVER;
            return;
        }
        
        // Check fruit collision
        if (head == fruit.getPosition()) {
            audioManager.playEatSound();
            snake.grow();
            fruit.respawn(snake.getBody());
            updateScore();
        }
        
        lastUpdate = elapsed;
    }
}

void Game::render() {
    window.clear(sf::Color::Black);
    if (texturesLoaded && backgroundSprite) {
        window.draw(*backgroundSprite);
    }
    
    switch (gameState) {
        case GameState::MENU:
            if (menuText) window.draw(*menuText);
            break;
            
        case GameState::PLAYING:
        case GameState::PAUSED:
            drawGrid();
            drawFruit();
            drawSnake();
            drawUI();
            
            if (gameState == GameState::PAUSED) {
                if (pauseText) window.draw(*pauseText);
            }
            break;
            
        case GameState::GAME_OVER:
            drawGrid();
            drawFruit();
            drawSnake();
            drawUI();
            if (gameOverText) window.draw(*gameOverText);
            
            sf::Text restartText(font, "Press SPACE to Restart", 24);
            restartText.setFillColor(sf::Color::White);
            sf::FloatRect textRect = restartText.getLocalBounds();
            restartText.setOrigin({textRect.position.x + textRect.size.x / 2.0f, textRect.position.y + textRect.size.y / 2.0f});
            restartText.setPosition({WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f + 50});
            window.draw(restartText);
            break;
    }
    
    window.display();
}

void Game::resetGame() {
    snake.reset();
    fruit.respawn(snake.getBody());
    score = 0;
    gameSpeed = BASE_SPEED;
    lastUpdate = sf::Time::Zero;
    gameClock.restart();
}

void Game::updateScore() {
    score += 10;
    // Increase speed slightly with each fruit eaten
    if (gameSpeed > 50.0f) {
        gameSpeed -= SPEED_INCREASE;
    }
    
    std::ostringstream ss;
    ss << "Score: " << score << " | Speed: " << static_cast<int>(BASE_SPEED - gameSpeed + SPEED_INCREASE);
    if (!audioManager.isSoundEnabled()) ss << " | Sound: OFF";
    if (!audioManager.isMusicEnabled()) ss << " | Music: OFF";
    if (scoreText) scoreText->setString(ss.str());
}

void Game::drawGrid() {
    // Draw subtle grid lines
    sf::RectangleShape line;
    line.setFillColor(sf::Color(40, 40, 40));
    
    // Vertical lines
    for (int x = 0; x <= GRID_WIDTH; ++x) {
        line.setSize({1, static_cast<float>(WINDOW_HEIGHT)});
        line.setPosition({static_cast<float>(x * CELL_SIZE), 0});
        window.draw(line);
    }
    
    // Horizontal lines
    for (int y = 0; y <= GRID_HEIGHT; ++y) {
        line.setSize({static_cast<float>(WINDOW_WIDTH), 1});
        line.setPosition({0, static_cast<float>(y * CELL_SIZE)});
        window.draw(line);
    }
}

void Game::drawSnake() {
    const auto& body = snake.getBody();
    if (body.empty()) return;

    sf::Vector2f headPos = gridToPixel(body[0]);
    if (texturesLoaded) {
        sf::Sprite head(headTexture);
        // Base scale to fit cell then enlarge
        float baseScaleX = (float)CELL_SIZE / headTexture.getSize().x * HEAD_SCALE;
        float baseScaleY = (float)CELL_SIZE / headTexture.getSize().y * HEAD_SCALE;
        head.setScale(sf::Vector2f(baseScaleX, baseScaleY));
        // Use center origin so rotation works naturally
        head.setOrigin(sf::Vector2f(headTexture.getSize().x / 2.f, headTexture.getSize().y / 2.f));
        // Place at center of cell
        sf::Vector2f cellCenter(headPos.x + CELL_SIZE / 2.f, headPos.y + CELL_SIZE / 2.f);
        head.setPosition(cellCenter);
    float rotation = 0.f; // degrees
        // Determine direction from the first two segments (head and next). This avoids storing direction explicitly here.
        if (body.size() > 1) {
            Position headGrid = body[0];
            Position nextGrid = body[1];
            int dx = headGrid.x - nextGrid.x;
            int dy = headGrid.y - nextGrid.y;
            if (dx == 1 && dy == 0) rotation = 0.f;          // moving RIGHT
            else if (dx == -1 && dy == 0) rotation = 180.f;   // moving LEFT
            else if (dx == 0 && dy == -1) rotation = 90.f;    // moving DOWN (since texture right -> rotate 90 to face down)
            else if (dx == 0 && dy == 1) rotation = -90.f;    // moving UP
        }
    head.setRotation(sf::degrees(rotation));
        window.draw(head);
    } else {
        sf::RectangleShape fallback({(float)CELL_SIZE - 2.f, (float)CELL_SIZE - 2.f});
        fallback.setFillColor(sf::Color::Green);
    fallback.setPosition(sf::Vector2f(headPos.x + 1.f, headPos.y + 1.f));
        window.draw(fallback);
    }

    // Body segments with gradient
    for (size_t i = 1; i < body.size(); ++i) {
        float t = static_cast<float>(i) / (body.size()-1);
        sf::Vector2f p = gridToPixel(body[i]);
        if (texturesLoaded) {
            sf::Sprite seg(bodyTexture);
            seg.setPosition(p);
            seg.setScale(sf::Vector2f((float)CELL_SIZE / bodyTexture.getSize().x, (float)CELL_SIZE / bodyTexture.getSize().y));
            window.draw(seg);
        } else {
            sf::RectangleShape segRect({CELL_SIZE - 3.f, CELL_SIZE - 3.f});
            segRect.setFillColor(sf::Color(0, 180, 0));
            segRect.setPosition(sf::Vector2f(p.x + 1.5f, p.y + 1.5f));
            window.draw(segRect);
        }
    }
}

void Game::drawFruit() {
    sf::Vector2f pixelPos = gridToPixel(fruit.getPosition());
    if (fruitTextureLoaded) {
        sf::Sprite fruitSprite(fruitTexture);
    float baseScaleX = (float)CELL_SIZE / fruitTexture.getSize().x * FRUIT_SCALE;
    float baseScaleY = (float)CELL_SIZE / fruitTexture.getSize().y * FRUIT_SCALE;
    fruitSprite.setScale(sf::Vector2f(baseScaleX, baseScaleY));
    fruitSprite.setOrigin(sf::Vector2f(fruitTexture.getSize().x / 2.f, fruitTexture.getSize().y / 2.f));
    fruitSprite.setPosition(sf::Vector2f(pixelPos.x + CELL_SIZE / 2.f, pixelPos.y + CELL_SIZE / 2.f));
        window.draw(fruitSprite);
    } else {
        sf::CircleShape fruitShape(CELL_SIZE / 2.0f - 2);
        fruitShape.setFillColor(sf::Color::Red);
        fruitShape.setPosition({pixelPos.x + 2, pixelPos.y + 2});
        window.draw(fruitShape);
    }
}

void Game::drawUI() {
    if (scoreText) window.draw(*scoreText);
}

bool Game::isValidPosition(const Position& pos) const {
    return pos.x >= 0 && pos.x < GRID_WIDTH && pos.y >= 0 && pos.y < GRID_HEIGHT;
}

sf::Vector2f Game::gridToPixel(const Position& pos) const {
    return {static_cast<float>(pos.x * CELL_SIZE), static_cast<float>(pos.y * CELL_SIZE)};
}
