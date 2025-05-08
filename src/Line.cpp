#include "Line.hpp"
#include <cmath>

Line::Line(sf::Vector2f start, sf::Vector2f end) {
    line[0] = sf::Vertex(start, sf::Color::White);
    line[1] = sf::Vertex(end, sf::Color::White);
}

sf::Vector2f Line::getStart() const { return line[0].position; }
sf::Vector2f Line::getEnd() const { return line[1].position; }

void Line::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    const float thickness = 6.f;
    sf::Vector2f dir = line[0].position - line[1].position;
    float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    float angle = std::atan2(dir.y, dir.x) * 180 / 3.14159f;

    sf::RectangleShape rect;
    rect.setSize(sf::Vector2f(length, thickness));
    rect.setOrigin(0, thickness / 2);
    rect.setPosition(line[0].position);
    rect.setRotation(angle);
    rect.setFillColor(sf::Color::White);
    target.draw(rect, states);
}