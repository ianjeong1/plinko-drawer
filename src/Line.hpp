#pragma once
#include <SFML/Graphics.hpp>

class Line : public sf::Drawable {
public:
    Line(sf::Vector2f start, sf::Vector2f end);
    sf::Vector2f getStart() const;
    sf::Vector2f getEnd() const;

private:
    sf::Vertex line[2];
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};