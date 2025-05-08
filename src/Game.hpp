#pragma once
#include <SFML/Graphics.hpp>
#include "Ball.hpp"
#include "Line.hpp"
#include "Bucket.hpp"
#include "Math.hpp"
#include <vector>

class Game {
public:
    Game();
    void run();
    bool init();

private:
    void processEvents();
    void update(float dt);
    void render();
    void handleMouseInput();
    void createBuckets();
    void updateBuckets();

    sf::RenderWindow window;
    Ball ball;
    std::vector<Line> lines;
    sf::Vector2f tempLineStart;
    sf::Font font;
    sf::Text positionText;
    bool drawing = false;
};