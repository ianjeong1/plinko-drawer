#pragma once
#include <SFML/Graphics.hpp>
#include "Line.hpp"

class Ball : public sf::Drawable {
public:
    Ball(sf::Vector2f pos);
    void update(float dt);
    void collideWith(Ball& other);
    bool checkCollision(const Line& line);
    sf::Vector2f getPosition() const;
    sf::Vector2f getVelocity() const;
    void reset();
    bool onGround = false;

private:
    sf::CircleShape shape;
    sf::Vector2f velocity;
    sf::Vector2f slopeTangent = {0.f, 0.f};
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};