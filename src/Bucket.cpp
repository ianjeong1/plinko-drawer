#include "Bucket.hpp"
#include <iostream>
#include <iomanip>
#include <sstream> 
#include <cmath>
#include <unistd.h>

Bucket::Bucket(float position, float width, float label, int count, sf::Font& font)
    : position(position), width(width), label(label), count(count) {
    bucketRect.setSize(sf::Vector2f(width, 20.f));
    bucketRect.setPosition(position, 1050.f);
    bucketRect.setFillColor(sf::Color::Black);

    labelText.setString(std::to_string(label));
    labelText.setFont(font);
    labelText.setCharacterSize(14);
    labelText.setFillColor(sf::Color::Black);
    labelText.setPosition(position + 5.f, 1050.f);
}

void Bucket::increment() {
    count++;
}

void Bucket::update(float expectedWinnings, float min, float max) { 
    label = expectedWinnings;

    // Assign colour heatmap
    float t = (label - min) / (max - min);
    t = std::max(0.f, std::min(1.f, t));

    sf::Color color(
        static_cast<sf::Uint8>((1 - t) * 255), 
        static_cast<sf::Uint8>(t * 255),
        0
    );
    bucketRect.setFillColor(color);
}

void Bucket::render(sf::RenderWindow& window) {
    if (count == 0) {
        return;
    }

    bucketRect.setSize(sf::Vector2f(width, 20.f));
    window.draw(bucketRect);

    // Format label to 2 decimal places
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << label;
    labelText.setString(stream.str());

    window.draw(labelText);
}

int Bucket::getCount() { return count; };