#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <functional>

// Struct and functions for convex hull construction
struct Point {
    double x, y;
};

bool compare(Point a, Point b) {
    return (a.x < b.x) || (a.x == b.x && a.y < b.y);
}

double crossProduct(Point O, Point A, Point B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

std::vector<Point> convexHull(std::vector<Point>& points) {
    int n = points.size();
    if (n <= 1)
        return points;

    sort(points.begin(), points.end(), compare);

    std::vector<Point> lowerHull;
    for (int i = 0; i < n; ++i) {
        while (lowerHull.size() >= 2 &&
            crossProduct(lowerHull[lowerHull.size() - 2], lowerHull.back(), points[i]) <= 0) {
            lowerHull.pop_back();
        }
        lowerHull.push_back(points[i]);
    }

    std::vector<Point> upperHull;
    for (int i = n - 1; i >= 0; --i) {
        while (upperHull.size() >= 2 &&
            crossProduct(upperHull[upperHull.size() - 2], upperHull.back(), points[i]) <= 0) {
            upperHull.pop_back();
        }
        upperHull.push_back(points[i]);
    }

    lowerHull.pop_back();
    upperHull.pop_back();
    lowerHull.insert(lowerHull.end(), upperHull.begin(), upperHull.end());

    return lowerHull;
}

class Button {
public:
    Button(sf::Texture* normal, sf::Texture* clicked, std::string text, sf::Vector2f location);
    void checkClick(sf::Vector2f mousePos);
    void setState(bool);
    bool getState();
    sf::Sprite* getSprite();
    sf::Text* getText();
    void setAction(std::function<void()> action);
    void toggleVisibility(); // New method to toggle visibility
    bool isVisible(); // New method to check visibility
private:
    sf::Sprite normalSprite;
    sf::Sprite clickedSprite;
    sf::Sprite* currentSprite;
    sf::Text buttonText;
    bool currentState;
    bool visible; // New member to track visibility
    std::function<void()> buttonAction;
};

Button::Button(sf::Texture* normal, sf::Texture* clicked, std::string text, sf::Vector2f location) {
    normalSprite.setTexture(*normal);
    clickedSprite.setTexture(*clicked);
    currentSprite = &normalSprite;
    currentState = false;
    visible = true; // Initially visible

    // Set button text properties
    buttonText.setString(text);
    buttonText.setCharacterSize(18);
    buttonText.setFillColor(sf::Color::Black);
    buttonText.setPosition(location.x + (normalSprite.getLocalBounds().width - buttonText.getLocalBounds().width) / 2, location.y + (normalSprite.getLocalBounds().height - buttonText.getLocalBounds().height) / 2);

    // Set button positions
    normalSprite.setPosition(location);
    clickedSprite.setPosition(location);

    // Set button size (smaller)
    sf::Vector2f newSize(100, 40);
    normalSprite.setScale(newSize.x / normalSprite.getLocalBounds().width, newSize.y / normalSprite.getLocalBounds().height);
    clickedSprite.setScale(newSize.x / clickedSprite.getLocalBounds().width, newSize.y / clickedSprite.getLocalBounds().height);

    buttonAction = nullptr;
}

void Button::checkClick(sf::Vector2f mousePos) {
    if (normalSprite.getGlobalBounds().contains(mousePos)) {
        currentState = !currentState;
        currentSprite = currentState ? &clickedSprite : &normalSprite;

        // Call button action if it's set
        if (buttonAction) {
            buttonAction();
        }
    }
}

void Button::setState(bool state) {
    currentState = state;
    currentSprite = currentState ? &clickedSprite : &normalSprite;
}

bool Button::getState() {
    return currentState;
}

sf::Sprite* Button::getSprite() {
    return currentSprite;
}

sf::Text* Button::getText() {
    return &buttonText;
}

void Button::setAction(std::function<void()> action) {
    buttonAction = action;
}

void Button::toggleVisibility() {
    visible = !visible; // Toggle visibility
}

bool Button::isVisible() {
    return visible;
}


// Function to build convex hull
void buildConvexHull(std::vector<Point>& convexHullPoints, std::vector<Point>& points, sf::RenderWindow& window) {
    convexHullPoints = convexHull(points);

    window.clear();

    // Display points
    for (const auto& point : points) {
        sf::CircleShape circle(5);
        circle.setFillColor(sf::Color::Blue);
        circle.setPosition(point.x, point.y);
        window.draw(circle);
    }

    // Display convex hull
    for (std::size_t i = 1; i < convexHullPoints.size(); ++i) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(convexHullPoints[i - 1].x, convexHullPoints[i - 1].y), sf::Color::Red),
            sf::Vertex(sf::Vector2f(convexHullPoints[i].x, convexHullPoints[i].y), sf::Color::Red)
        };
        sf::Vertex lastLine[] = {
            sf::Vertex(sf::Vector2f(convexHullPoints.back().x, convexHullPoints.back().y), sf::Color::Red),
            sf::Vertex(sf::Vector2f(convexHullPoints.front().x, convexHullPoints.front().y), sf::Color::Red)
        };
        window.draw(line, 2, sf::Lines);
        window.draw(lastLine, 2, sf::Lines);
    }

    window.display();
}

int main() {
    std::vector<Point> points;
    double x, y;

    // Input points from file
    std::ifstream inputFile("points.txt");
    if (!inputFile.is_open()) {
        std::cerr << "Unable to open file!" << std::endl;
        return 1;
    }

    while (inputFile >> x >> y) {
        points.push_back({ x, y });
    }
    inputFile.close();

    // Check if points are available
    if (points.empty()) {
        std::cerr << "No points to display!" << std::endl;
        return 1;
    }

    std::vector<Point> convexHullPoints;

    sf::RenderWindow window(sf::VideoMode(1200, 1200), "Convex Hull Visualization");
    window.setFramerateLimit(60);

    // Create button for building convex hull
    sf::Texture normalTexture, clickedTexture;
    if (!normalTexture.loadFromFile("normal_button.png") || !clickedTexture.loadFromFile("clicked_button.png")) {
        std::cerr << "Failed to load button textures." << std::endl;
        return 1;
    }

    Button buildConvexHullButton(&normalTexture, &clickedTexture, "Build Convex Hull", sf::Vector2f(50, 1000));

    // Set action for the button
    buildConvexHullButton.setAction([&convexHullPoints, &points, &window]() {
        buildConvexHull(convexHullPoints, points, window);
        });

    bool convexHullBuilt = false; // Flag to check if convex hull is built

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // Handle button click
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                    buildConvexHullButton.checkClick(mousePos);
                }
            }
        }

        // Clearing the window and redrawing everything
        // window.clear(sf::Color::White);

        // Display button
        window.draw(*buildConvexHullButton.getSprite());
        window.draw(*buildConvexHullButton.getText());

        // Displaying convex hull if it's already built
        if (!convexHullPoints.empty()) {
            // Display points
            for (const auto& point : points) {
                sf::CircleShape circle(5);
                circle.setFillColor(sf::Color::Blue);
                circle.setPosition(point.x, point.y);
                window.draw(circle);
            }

            // Display convex hull
            for (std::size_t i = 1; i < convexHullPoints.size(); ++i) {
                sf::Vertex line[] = {
                    sf::Vertex(sf::Vector2f(convexHullPoints[i - 1].x, convexHullPoints[i - 1].y), sf::Color::Red),
                    sf::Vertex(sf::Vector2f(convexHullPoints[i].x, convexHullPoints[i].y), sf::Color::Red)
                };
                sf::Vertex lastLine[] = {
                    sf::Vertex(sf::Vector2f(convexHullPoints.back().x, convexHullPoints.back().y), sf::Color::Red),
                    sf::Vertex(sf::Vector2f(convexHullPoints.front().x, convexHullPoints.front().y), sf::Color::Red)
                };
                window.draw(line, 2, sf::Lines);
                window.draw(lastLine, 2, sf::Lines);
            }
        }

        window.display();
    }


    return 0;
}
