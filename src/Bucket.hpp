#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Bucket {
public:
    Bucket(float position, float width, float label, int count, sf::Font& font);

    void increment();
    void update(float targetWinnings, float max, float min);
    void render(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
    int getCount();

    float position;
    float width;
    float label;
    int count;
    
    sf::RectangleShape bucketRect;
    sf::Text labelText;
};
