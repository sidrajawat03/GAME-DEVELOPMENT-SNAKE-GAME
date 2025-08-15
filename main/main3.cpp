#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <ctime>

using namespace sf;

const int width = 800;
const int height = 600;
const int size = 20;

struct SnakeSegment {
    int x, y;
};

int main() {
    RenderWindow window(VideoMode(width, height), "Snake Game");
    window.setFramerateLimit(10);

    // Load sounds
    SoundBuffer eatBuffer, gameoverBuffer;
    eatBuffer.loadFromFile("eat.wav");
    gameoverBuffer.loadFromFile("gameover.wav");

    Sound eatSound(eatBuffer);
    Sound gameoverSound(gameoverBuffer);

    std::vector<SnakeSegment> snake = { {10, 10} };
    SnakeSegment fruit = {5, 5};

    int dx = 1, dy = 0;
    bool gameOver = false;

    Clock clock;

    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed)
                window.close();
        }

        if (Keyboard::isKeyPressed(Keyboard::Up) && dy == 0) { dx = 0; dy = -1; }
        if (Keyboard::isKeyPressed(Keyboard::Down) && dy == 0) { dx = 0; dy = 1; }
        if (Keyboard::isKeyPressed(Keyboard::Left) && dx == 0) { dx = -1; dy = 0; }
        if (Keyboard::isKeyPressed(Keyboard::Right) && dx == 0) { dx = 1; dy = 0; }

        if (!gameOver) {
            SnakeSegment newHead = { snake[0].x + dx, snake[0].y + dy };
            snake.insert(snake.begin(), newHead);

            if (newHead.x == fruit.x && newHead.y == fruit.y) {
                eatSound.play();
                fruit.x = rand() % (width / size);
                fruit.y = rand() % (height / size);
            } else {
                snake.pop_back();
            }

            if (newHead.x < 0 || newHead.y < 0 || newHead.x >= width / size || newHead.y >= height / size) {
                gameoverSound.play();
                gameOver = true;
            }

            for (int i = 1; i < snake.size(); i++) {
                if (newHead.x == snake[i].x && newHead.y == snake[i].y) {
                    gameoverSound.play();
                    gameOver = true;
                }
            }
        }

        window.clear(Color::Black);

        // Draw snake
        for (auto &s : snake) {
            RectangleShape segment(Vector2f(size - 1, size - 1));
            segment.setFillColor(Color::Green);
            segment.setPosition(Vector2f(s.x * size, s.y * size));
            window.draw(segment);
        }

        // Draw fruit
        RectangleShape fruitRect(Vector2f(size - 1, size - 1));
        fruitRect.setFillColor(Color::Red);
        fruitRect.setPosition(Vector2f(fruit.x * size, fruit.y * size));
        window.draw(fruitRect);

        window.display();
    }

    return 0;
}
