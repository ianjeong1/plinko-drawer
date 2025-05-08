#include "Game.hpp"
#include <iostream>
#include <sstream>
#include <cmath>

enum class InputField {
    None,
    NumBalls,
    expectedLoss,
    Variance
};

InputField activeInputField = InputField::None;

sf::RectangleShape topBar;
struct InputFieldUI {
    sf::RectangleShape box;
    sf::Text label;
    sf::Text value;
    sf::String strValue;
};

InputFieldUI numBallsField, expectedLossField, varianceField;

int numBalls = 1;
float expectedLoss = 0.0f;
float variance = 0.f;

std::vector<Ball> activeBalls;
int ballsDropped = 0;
sf::Clock ballDropClock;

std::vector<Bucket> buckets;

sf::RectangleShape startButton, clearButton, undoButton;
sf::Text startButtonLabel, clearButtonLabel, undoButtonLabel;
bool gameRunning = false;

std::vector<sf::Vector2f> currentCurve;
sf::VertexArray liveVisualLine{sf::TriangleStrip};
std::vector<sf::VertexArray> finalVisualLines;
sf::Vector2f clickStartPos;
std::vector<sf::CircleShape> circles;
int last_obj = 2; // 0 if last added object is a line, 1 if last added object is a circle, 2 if neither
int added_lines = 0;
std::vector<int> linesPerObject;
bool clickMoved = false;
bool drawing = false;

float distance(const sf::Vector2f& a, const sf::Vector2f& b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}

void addThickLineToVertexArray(sf::VertexArray& va, const sf::Vector2f& p1, const sf::Vector2f& p2, float thickness) {
    sf::Vector2f dir = p2 - p1;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len == 0) return;

    dir /= len;
    sf::Vector2f normal(-dir.y, dir.x); // Perpendicular

    sf::Vector2f offset = normal * (thickness / 3.f);
    va.append(sf::Vertex(p1 + offset, sf::Color::White));
    va.append(sf::Vertex(p1 - offset, sf::Color::White));
    va.append(sf::Vertex(p2 + offset, sf::Color::White));
    va.append(sf::Vertex(p2 - offset, sf::Color::White));
}

Game::Game() : window(sf::VideoMode(1920, 1080), "Plinko Drawer"), ball({960.f, 100.f}) {
    window.setFramerateLimit(120);
}

bool Game::init() {
    if (!font.loadFromFile("../arial.ttf")) {
        std::cout << "Failed to load font!" << std::endl;
        return false;
    }

    topBar.setSize(sf::Vector2f(window.getSize().x, 80.f));
    topBar.setFillColor(sf::Color(200, 200, 200));

    float x = 50.f;
    float spacing = 230.f;

    auto setupField = [&](InputFieldUI& field, const std::string& label, float xPos) {
        field.box.setSize({200.f, 30.f});
        field.box.setPosition(xPos, 40.f);
        field.box.setFillColor(sf::Color::White);
        field.box.setOutlineColor(sf::Color::Black);
        field.box.setOutlineThickness(2.f);

        field.label.setFont(font);
        field.label.setCharacterSize(14);
        field.label.setFillColor(sf::Color::Black);
        field.label.setString(label);
        field.label.setPosition(xPos, 20.f);

        field.value.setFont(font);
        field.value.setCharacterSize(16);
        field.value.setFillColor(sf::Color::Black);
        field.value.setPosition(xPos + 5.f, 45.f);
    };

    setupField(numBallsField, "Balls Dropped", x);
    setupField(expectedLossField, "Expected Loss", x + spacing);
    setupField(varianceField, "Variance", x + spacing * 2);

    numBallsField.strValue = std::to_string(numBalls);
    expectedLossField.strValue = (expectedLoss == 0.f) ? "0" : std::to_string(expectedLoss);
    varianceField.strValue = (variance == 0.f) ? "0" : std::to_string(variance);

    positionText.setFont(font);
    positionText.setCharacterSize(16);
    positionText.setFillColor(sf::Color::Black);
    positionText.setPosition(10.f, 50.f);

    float buttonWidth = 100.f;
    float buttonHeight = 30.f;
    float buttonSpacing = 10.f;

    // Start button
    startButton.setSize(sf::Vector2f(buttonWidth, buttonHeight));
    startButton.setPosition(window.getSize().x - buttonWidth - buttonSpacing, buttonSpacing);
    startButton.setFillColor(sf::Color::Green);

    startButtonLabel.setFont(font);
    startButtonLabel.setCharacterSize(14);
    startButtonLabel.setFillColor(sf::Color::Black);
    startButtonLabel.setString("Start");    
    startButtonLabel.setPosition(startButton.getPosition().x + 20.f, startButton.getPosition().y + 5.f);

    // Clear button
    clearButton.setSize(sf::Vector2f(buttonWidth, buttonHeight));
    clearButton.setPosition(window.getSize().x - 2 * buttonWidth - 2 * buttonSpacing, buttonSpacing);
    clearButton.setFillColor(sf::Color::Red);

    clearButtonLabel.setFont(font);
    clearButtonLabel.setCharacterSize(14);
    clearButtonLabel.setFillColor(sf::Color::Black);
    clearButtonLabel.setString("Clear");
    clearButtonLabel.setPosition(clearButton.getPosition().x + 20.f, clearButton.getPosition().y + 5.f);

    // Undo button
    undoButton.setSize(sf::Vector2f(buttonWidth, buttonHeight));
    undoButton.setPosition(window.getSize().x - 3 * buttonWidth - 3 * buttonSpacing, buttonSpacing);
    undoButton.setFillColor(sf::Color::Blue);

    undoButtonLabel.setFont(font);
    undoButtonLabel.setCharacterSize(14);
    undoButtonLabel.setFillColor(sf::Color::Black);
    undoButtonLabel.setString("Undo");
    undoButtonLabel.setPosition(undoButton.getPosition().x + 20.f, undoButton.getPosition().y + 5.f);

    createBuckets();

    return true;
}

void Game::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        processEvents();
        if (gameRunning) {
            update(dt);
        }
        render();
    }
}

void Game::createBuckets() {
    buckets.clear();

    float bucketWidth = 96.f;
    float numBuckets = 20;
    float startX = 0.f;

    for (float i = 0; i < numBuckets; ++i) {
        float winnings = expectedLoss + (variance * (i - (numBuckets / 2.0f)));
        buckets.push_back(Bucket(startX + i * bucketWidth, bucketWidth, i, winnings, font));
    }
}

void Game::updateBuckets() {
    std::vector<int> counts;
    counts.clear();
    for (auto& bucket : buckets) {
        counts.emplace_back(bucket.getCount());
    }

    std::vector<float> labels(20);
    for (int i = 0; i < 20; ++i) {
        labels[i] = static_cast<float>(i); 
    }

    assignLabels(counts, labels, expectedLoss, variance);

    for (int i = 0; i < 20; ++i) {
        buckets[i].update(labels[i], *std::min_element(labels.begin(), labels.end()), *std::max_element(labels.begin(), labels.end()));
    }

}

void Game::processEvents() {
    sf::Event event;
    sf::Vector2f mousePos;

    while (window.pollEvent(event)) {
        mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        if (event.type == sf::Event::Closed)
            window.close();

        if (event.type == sf::Event::MouseButtonPressed) {
            if (startButton.getGlobalBounds().contains(mousePos)) {
                if (gameRunning) {
                    gameRunning = false;
                    activeBalls.clear();
                    ballsDropped = 0;
                    startButtonLabel.setString("Start");
                } else {
                    gameRunning = true;
                    activeBalls.clear();
                    ballsDropped = 0;
                    startButtonLabel.setString("Stop");
                }
            }

            if (clearButton.getGlobalBounds().contains(mousePos)) {
                lines.clear();
                finalVisualLines.clear();
                circles.clear();
                last_obj = 2;
            }

            if (undoButton.getGlobalBounds().contains(mousePos)) {
                    if (last_obj == 0) {
                        // Undo the last added line
                        int linesToRemove = linesPerObject.back();
                        linesPerObject.pop_back();
                        finalVisualLines.pop_back();
                        for (int i = 0; i < linesToRemove; i++) {
                            lines.pop_back();
                        }
                    } else if (last_obj == 1) {
                        // Undo the last added circle
                        if (!circles.empty()) {
                            circles.pop_back();
                            int linesToRemove = 36;
                            finalVisualLines.pop_back();
                            for (int i = 0; i < linesToRemove; i++) {
                                lines.pop_back();
                            }
                        }
                        linesPerObject.pop_back();
                    }

                    added_lines = 0;
                }

            // Input field selection
            auto checkClick = [&](InputFieldUI& field, InputField type) {
                if (field.box.getGlobalBounds().contains(mousePos)) {
                    activeInputField = type;
                }
            };
            checkClick(numBallsField, InputField::NumBalls);
            checkClick(expectedLossField, InputField::expectedLoss);
            checkClick(varianceField, InputField::Variance);

            // Drawing
            bool onUI = startButton.getGlobalBounds().contains(mousePos) ||
                        clearButton.getGlobalBounds().contains(mousePos) ||
                        undoButton.getGlobalBounds().contains(mousePos) ||
                        numBallsField.box.getGlobalBounds().contains(mousePos) ||
                        expectedLossField.box.getGlobalBounds().contains(mousePos) ||
                        varianceField.box.getGlobalBounds().contains(mousePos);

            if (event.mouseButton.button == sf::Mouse::Left && !onUI) {
                currentCurve.clear();
                liveVisualLine.clear();
                drawing = true;
                clickMoved = false;
                clickStartPos = mousePos;
            }
        }

        if (event.type == sf::Event::TextEntered) {
            char c = static_cast<char>(event.text.unicode);
            if ((std::isdigit(c) || c == '.') && event.text.unicode < 128) {
                switch (activeInputField) {
                    case InputField::NumBalls: numBallsField.strValue += c; break;
                    case InputField::expectedLoss: expectedLossField.strValue += c; break;
                    case InputField::Variance: varianceField.strValue += c; break;
                    default: break;
                }
            } else if (c == 8) {
                auto eraseLast = [](sf::String& s) {
                    if (!s.isEmpty()) s.erase(s.getSize() - 1, 1);
                };
                switch (activeInputField) {
                    case InputField::NumBalls: eraseLast(numBallsField.strValue); break;
                    case InputField::expectedLoss: eraseLast(expectedLossField.strValue); break;
                    case InputField::Variance: eraseLast(varianceField.strValue); break;
                    default: break;
                }
            }
        }

        if (event.type == sf::Event::MouseButtonReleased && drawing) {
            drawing = false;
            if (!clickMoved) {
                float radius = 10.f;
                sf::Vector2f center = clickStartPos;

                sf::CircleShape circle(radius + 0.4f);
                circle.setOrigin(radius, radius);
                circle.setPosition(center);
                circle.setFillColor(sf::Color::White);
                circles.push_back(circle);

                for (int i = 0; i < 36; ++i) {
                    float angle1 = 2 * M_PI * i / 36;
                    float angle2 = 2 * M_PI * (i + 1) / 36;

                    sf::Vector2f p1(center.x + radius * std::cos(angle1) / 2.5f, center.y + radius * std::sin(angle1) / 2.5f);
                    sf::Vector2f p2(center.x + radius * std::cos(angle2) / 2.5f, center.y + radius * std::sin(angle2) / 2.5f);

                    lines.emplace_back(p1, p2);
                    sf::VertexArray va(sf::TrianglesStrip);
                    addThickLineToVertexArray(va, p1, p2, 20.f);
                    finalVisualLines.push_back(va);
                }

                linesPerObject.push_back(36);
                last_obj = 1;
            } else {
                finalVisualLines.push_back(liveVisualLine);
                linesPerObject.push_back(added_lines);
                added_lines = 0;
                last_obj = 0;
            }

            liveVisualLine.clear();
        }

        if (event.type == sf::Event::MouseMoved && drawing) {
            sf::Vector2f pos = mousePos;

            if (!clickMoved && distance(pos, clickStartPos) > 5.f) {
                clickMoved = true;
            }

            if (!currentCurve.empty()) {
                sf::Vector2f last = currentCurve.back();
                float d = distance(last, pos);

                if (d > 1.f) {
                    int steps = static_cast<int>(d / 3.f);
                    for (int i = 1; i <= steps; ++i) {
                        sf::Vector2f interp = last + (pos - last) * (i / static_cast<float>(steps));
                        lines.emplace_back(last, interp);
                        added_lines++;
                        currentCurve.push_back(interp);
                        last = interp;

                        addThickLineToVertexArray(liveVisualLine, currentCurve[currentCurve.size() - 2], interp, 20.f);
                    }
                }
            }

            if (currentCurve.empty() || distance(currentCurve.back(), pos) > 1.f) {
                if (!currentCurve.empty()) {
                    lines.emplace_back(currentCurve.back(), pos);
                    added_lines++;
                    addThickLineToVertexArray(liveVisualLine, currentCurve.back(), pos, 20.f);
                }
                currentCurve.push_back(pos);
            }
        }
    }
}

void Game::update(float dt) {
    try {
        numBalls = std::stoi(numBallsField.strValue.toAnsiString());
        expectedLoss = std::stof(expectedLossField.strValue.toAnsiString()) * -1;
        variance = std::stof(varianceField.strValue.toAnsiString());
    } catch (...) {}

    if (gameRunning) {
        if (ballsDropped < numBalls && ballDropClock.getElapsedTime().asMilliseconds() > 150) {
            activeBalls.emplace_back(sf::Vector2f(960.f, 100.f));  // Add a new ball
            ballDropClock.restart();
            ballsDropped++;
        }

        // Update each ball and check for collisions with walls/lines
        for (auto& b : activeBalls) {
            b.update(dt);
            bool touching = false;

            for (auto& line : lines) {
                if (b.checkCollision(line)) touching = true;
            }

            if (b.getPosition().y > 1080) {
                int bucket = floor(b.getPosition().x / 96);
                buckets[bucket].increment();
                b.reset();
                updateBuckets();
            }

            b.onGround = touching;
        }
    }
}

void Game::render() {
    window.clear(sf::Color::Black);
    window.draw(topBar);

    window.draw(startButton); window.draw(startButtonLabel);
    window.draw(clearButton); window.draw(clearButtonLabel);
    window.draw(undoButton); window.draw(undoButtonLabel);

    auto drawField = [&](InputFieldUI& field, InputField type) {
        field.box.setOutlineColor(activeInputField == type ? sf::Color::Red : sf::Color::Black);
        field.value.setString(field.strValue);
        field.box.setOutlineThickness(2.f);
        window.draw(field.box);
        window.draw(field.label);
        window.draw(field.value);
    };

    drawField(numBallsField, InputField::NumBalls);
    drawField(expectedLossField, InputField::expectedLoss);
    drawField(varianceField, InputField::Variance);

    for (auto& va : finalVisualLines) window.draw(va);
    for (auto& circle : circles) window.draw(circle);
    if (drawing && liveVisualLine.getVertexCount() > 3) window.draw(liveVisualLine);
    window.draw(positionText);

    for (auto& b : activeBalls) {
        window.draw(b);
    }

    for (auto& bucket : buckets) {
        bucket.render(window);
    }

    window.display();
}