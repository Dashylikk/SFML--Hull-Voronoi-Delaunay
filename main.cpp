#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <functional>
#include <random>
#include <vector>
#include <cmath>

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
float squaredDistance(sf::Vector2f p1, sf::Vector2f p2) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    return dx * dx + dy * dy;
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
    void draw(sf::RenderWindow& window);
    void addButton(Button* button);
    void setParent(Button* parent);
    void closeSubmenu();
private:
    sf::Sprite normalSprite;
    sf::Sprite clickedSprite;
    sf::Sprite* currentSprite;
    sf::Text buttonText;
    bool currentState;
    std::function<void()> buttonAction;
    std::vector<Button*> subButtons;
    Button* parentButton;
};

Button::Button(sf::Texture* normal, sf::Texture* clicked, std::string text, sf::Vector2f location) {
    normalSprite.setTexture(*normal);
    clickedSprite.setTexture(*clicked);
    currentSprite = &normalSprite;
    currentState = false;

    buttonText.setString(text);
    buttonText.setCharacterSize(18);
    buttonText.setFillColor(sf::Color::Black);
    buttonText.setPosition(location.x + (normalSprite.getLocalBounds().width - buttonText.getLocalBounds().width) / 2, location.y + (normalSprite.getLocalBounds().height - buttonText.getLocalBounds().height) / 2);

    normalSprite.setPosition(location);
    clickedSprite.setPosition(location);

    sf::Vector2f newSize(100, 40);
    normalSprite.setScale(newSize.x / normalSprite.getLocalBounds().width, newSize.y / normalSprite.getLocalBounds().height);
    clickedSprite.setScale(newSize.x / clickedSprite.getLocalBounds().width, newSize.y / clickedSprite.getLocalBounds().height);

    buttonAction = nullptr;
    parentButton = nullptr;
}

void Button::checkClick(sf::Vector2f mousePos) {
    if (currentSprite->getGlobalBounds().contains(mousePos)) {
        currentState = !currentState;
        currentSprite = currentState ? &clickedSprite : &normalSprite;

        if (buttonAction) {
            buttonAction();
        }
    }

    if (currentState) {
        for (Button* button : subButtons) {
            button->checkClick(mousePos);
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
void Button::draw(sf::RenderWindow& window) {
    window.draw(*currentSprite);
    window.draw(buttonText);

    if (currentState) {
        for (Button* button : subButtons) {
            button->draw(window);
        }
    }
}
void Button::addButton(Button* button) {
    button->setParent(this);
    subButtons.push_back(button);
}

void Button::setParent(Button* parent) {
    parentButton = parent;
}

void Button::closeSubmenu() {
    currentState = false;
    currentSprite = &normalSprite;
    for (Button* button : subButtons) {
        button->closeSubmenu();
    }
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
void readPointsFromFile(std::vector<Point>& points) {
    double x, y;
    std::ifstream inputFile("points.txt");
    if (!inputFile.is_open()) {
        std::cerr << "Unable to open file!" << std::endl;
        return;
    }

    while (inputFile >> x >> y) {
        points.push_back({ x, y });
    }
    inputFile.close();
}
void readPointsVoron(const std::string& filename, std::vector<sf::Vector2f>& points) {
    std::ifstream inputFile("points.txt");
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open file: " << std::endl;
        return;
    }

    float x, y;
    while (inputFile >> x >> y) {
        points.emplace_back(x, y);
    }

    inputFile.close();
}
void generateRandomPointsAndColors(int numPoints, int width, int height, std::vector<sf::Vector2f>& points, std::vector<sf::Color>& colors) {
    points.clear();
    colors.clear();

    for (int i = 0; i < numPoints; ++i) {
        points.push_back(sf::Vector2f(std::rand() % width, std::rand() % height));
        colors.push_back(sf::Color(std::rand() % 255, std::rand() % 255, std::rand() % 255));
    }
}

void drawVoronoiDiagram(sf::RenderWindow& window, int width, int height, const std::vector<sf::Vector2f>& points, const std::vector<sf::Color>& colors) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int closestIndex = 0;
            float minDist = squaredDistance(points[0], sf::Vector2f(x, y));

            for (size_t i = 1; i < points.size(); ++i) {
                float dist = squaredDistance(points[i], sf::Vector2f(x, y));
                if (dist < minDist) {
                    minDist = dist;
                    closestIndex = i;
                }
            }

            sf::RectangleShape pixel(sf::Vector2f(1, 1));
            pixel.setPosition(x, y);
            pixel.setFillColor(colors[closestIndex]);
            window.draw(pixel);
        }
    }
}

void drawPoints(sf::RenderWindow& window, const std::vector<sf::Vector2f>& points) {
    for (size_t i = 0; i < points.size(); ++i) {
        sf::CircleShape circle(3);
        circle.setPosition(points[i]);
        circle.setFillColor(sf::Color::Black);
        window.draw(circle);
    }
}
void calculateVoronoiDiagram3(const std::vector<Point>& points, std::vector<std::vector<Point>>& voronoiDiagram, int width, int height) {
    voronoiDiagram.clear();
    voronoiDiagram.resize(points.size());

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float minDist = std::numeric_limits<float>::max();
            int closestIndex = -1;

            for (size_t i = 0; i < points.size(); ++i) {
                sf::Vector2f currentPixel(static_cast<float>(x), static_cast<float>(y));
                sf::Vector2f currentPoint(points[i].x, points[i].y);

                float dist = squaredDistance(currentPoint, currentPixel);
                if (dist < minDist) {
                    minDist = dist;
                    closestIndex = i;
                }
            }

            voronoiDiagram[closestIndex].push_back({ static_cast<float>(x), static_cast<float>(y) });
        }
    }
}
void drawVoronoiDiagram3(sf::RenderWindow& window, const std::vector<std::vector<Point>>& voronoiDiagram, const std::vector<sf::Color>& colors) {
    for (size_t i = 0; i < voronoiDiagram.size(); ++i) {
        for (const auto& point : voronoiDiagram[i]) {
            sf::RectangleShape pixel(sf::Vector2f(1, 1));
            pixel.setPosition(point.x, point.y);
            pixel.setFillColor(colors[i]);
            window.draw(pixel);
        }
    }
}
void drawPoints3(sf::RenderWindow& window, const std::vector<Point>& points) {
    for (const auto& point : points) {
        sf::CircleShape circle(3);
        circle.setPosition(point.x, point.y);
        circle.setFillColor(sf::Color::Black);
        window.draw(circle);
    }
}

std::vector<Point> points;
std::vector<Point> convexHullPoints;


int main() {
    const int width = 2000;
    const int height = 1200;
    const int numPoints = 20;
    sf::RenderWindow window(sf::VideoMode(width, height), "Lab-1");

    sf::Texture normalTexture, clickedTexture;
    if (!normalTexture.loadFromFile("normal_button.png") || !clickedTexture.loadFromFile("clicked_button.png")) {
        std::cerr << "Failed to load button textures." << std::endl;
        return 1;
    }

    Button mainButton1(&normalTexture, &clickedTexture, "Main Button 1", sf::Vector2f(50, 50));
    Button mainButton2(&normalTexture, &clickedTexture, "Main Button 2", sf::Vector2f(50, 250));
    Button mainButton3(&normalTexture, &clickedTexture, "Main Button 3", sf::Vector2f(50, 450));

    sf::Texture submenuNormalTexture, submenuClickedTexture;
    if (!submenuNormalTexture.loadFromFile("submenu_normal_button.png") || !submenuClickedTexture.loadFromFile("submenu_clicked_button.png")) {
        std::cerr << "Failed to load submenu button textures." << std::endl;
        return 1;
    }

    Button submenuButton1(&submenuNormalTexture, &submenuClickedTexture, "Submenu Button 1", sf::Vector2f(50, 100));
    Button submenuButton2(&submenuNormalTexture, &submenuClickedTexture, "Submenu Button 2", sf::Vector2f(50, 150));
    Button submenuButton3(&submenuNormalTexture, &submenuClickedTexture, "Submenu Button 3", sf::Vector2f(50, 200));

    mainButton1.addButton(&submenuButton1);
    mainButton1.addButton(&submenuButton2);
    mainButton1.addButton(&submenuButton3);

    Button submenuButton4(&submenuNormalTexture, &submenuClickedTexture, "Submenu Button 4", sf::Vector2f(50, 300));
    Button submenuButton5(&submenuNormalTexture, &submenuClickedTexture, "Submenu Button 5", sf::Vector2f(50, 350));
    Button submenuButton6(&submenuNormalTexture, &submenuClickedTexture, "Submenu Button 6", sf::Vector2f(50, 400));

    mainButton2.addButton(&submenuButton4);
    mainButton2.addButton(&submenuButton5);
    mainButton2.addButton(&submenuButton6);

    Button submenuButton7(&submenuNormalTexture, &submenuClickedTexture, "Submenu Button 7", sf::Vector2f(50, 500));
    Button submenuButton8(&submenuNormalTexture, &submenuClickedTexture, "Submenu Button 8", sf::Vector2f(50, 550));
    Button submenuButton9(&submenuNormalTexture, &submenuClickedTexture, "Submenu Button 9", sf::Vector2f(50, 600));

    mainButton3.addButton(&submenuButton7);
    mainButton3.addButton(&submenuButton8);
    mainButton3.addButton(&submenuButton9);

    submenuButton1.setAction([&]() {
        points.clear();
        readPointsFromFile(points);
        buildConvexHull(convexHullPoints, points, window);
        });

    submenuButton2.setAction([&window]() {
        points.clear();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> disX(200, 1450);
        std::uniform_real_distribution<double> disY(100, 1100);
        for (int i = 0; i < numPoints; ++i) {
            points.push_back({ disX(gen), disY(gen) });
        }

        buildConvexHull(convexHullPoints, points, window);
        });

    submenuButton3.setAction([&window]() {
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
                else if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        sf::Vector2i position = sf::Mouse::getPosition(window);
                        points.push_back({ static_cast<double>(position.x), static_cast<double>(position.y) });
                    }
                }
            }

            window.clear(sf::Color::White);

            for (const auto& point : points) {
                sf::CircleShape circle(5);
                circle.setFillColor(sf::Color::Blue);
                circle.setPosition(point.x, point.y);
                window.draw(circle);
            }

            if (points.size() >= 3) {
                std::vector<Point> convexHullPoints = convexHull(points);
                for (std::size_t i = 1; i < convexHullPoints.size(); ++i) {
                    sf::Vertex line[] = {
                        sf::Vertex(sf::Vector2f(convexHullPoints[i - 1].x, convexHullPoints[i - 1].y), sf::Color::Red),
                        sf::Vertex(sf::Vector2f(convexHullPoints[i].x, convexHullPoints[i].y), sf::Color::Red)
                    };
                    window.draw(line, 2, sf::Lines);
                }
                if (!convexHullPoints.empty()) {
                    sf::Vertex lastLine[] = {
                        sf::Vertex(sf::Vector2f(convexHullPoints.back().x, convexHullPoints.back().y), sf::Color::Red),
                        sf::Vertex(sf::Vector2f(convexHullPoints.front().x, convexHullPoints.front().y), sf::Color::Red)
                    };
                    window.draw(lastLine, 2, sf::Lines);
                }
            }

            window.display();
        }
        });
    submenuButton4.setAction([&]() {
        std::vector<sf::Vector2f> points;
        std::vector<sf::Color> colors;
        points.clear();
        readPointsVoron("points.txt", points);
        for (size_t i = 0; i < points.size(); ++i) {
            colors.emplace_back(std::rand() % 255, std::rand() % 255, std::rand() % 255);
        }
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
            }
            window.clear(sf::Color::White);
            drawVoronoiDiagram(window, width, height, points, colors);
            drawPoints(window, points);
            window.display();
        }
        });

    submenuButton5.setAction([&]() {
        std::vector<sf::Vector2f> points;
        std::vector<sf::Color> colors;

        generateRandomPointsAndColors(numPoints, width, height, points, colors);

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            window.clear(sf::Color::White);

            drawVoronoiDiagram(window, width, height, points, colors);
            drawPoints(window, points);

            window.display();
        }
        });

    submenuButton6.setAction([&]() {
        std::vector<Point> points;
        std::vector<sf::Color> colors;
        std::vector<std::vector<Point>> voronoiDiagram3;

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();

                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        points.push_back({ static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y) });

                        sf::Color color(std::rand() % 255, std::rand() % 255, std::rand() % 255);
                        colors.push_back(color);

                        voronoiDiagram3.resize(points.size());

                        calculateVoronoiDiagram3(points, voronoiDiagram3, width, height);
                    }
                }
            }

            window.clear(sf::Color::White);

            drawVoronoiDiagram3(window, voronoiDiagram3, colors);
            drawPoints3(window, points);

            window.display();
        }
        });


    bool submenuButton1Clicked = false;
    bool submenuButton2Clicked = false;
    bool submenuButton3Clicked = false;
    bool submenuButton4Clicked = false;
    bool submenuButton5Clicked = false;
    bool submenuButton6Clicked = false;

    // Основний цикл програми
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                    mainButton1.checkClick(mousePos);
                    mainButton2.checkClick(mousePos);
                    mainButton3.checkClick(mousePos);
                }
            }
        }

        if (submenuButton1.getState()) {
            points.clear();
            submenuButton1Clicked = true;
        }

        if (submenuButton2.getState()) {
            points.clear();
            submenuButton2Clicked = true;
        }
        if (submenuButton3.getState()) {
            points.clear();
            submenuButton3Clicked = true;
        }
        if (submenuButton4.getState()) {
            points.clear();
            submenuButton4Clicked = true;
        }
        if (submenuButton5.getState()) {
            points.clear();
            submenuButton5Clicked = true;
        }
        if (submenuButton6.getState()) {
            points.clear();
            submenuButton6Clicked = true;
        }
        if (submenuButton1Clicked && submenuButton2Clicked && submenuButton3Clicked && submenuButton4Clicked && submenuButton5Clicked && submenuButton6Clicked) {
            submenuButton1Clicked = false;
            submenuButton2Clicked = false;
            submenuButton3Clicked = false;
            submenuButton4Clicked = false;
            submenuButton5Clicked = false;
            submenuButton6Clicked = false;
        }

        window.clear(sf::Color::White);
        mainButton1.draw(window);
        mainButton2.draw(window);
        mainButton3.draw(window);



        if (!submenuButton1Clicked || !submenuButton2Clicked || !submenuButton3Clicked || !submenuButton4Clicked || !submenuButton5Clicked || !submenuButton6Clicked) {
            for (const auto& point : points) {
                sf::CircleShape circle(5);
                circle.setFillColor(sf::Color::Blue);
                circle.setPosition(point.x, point.y);
                window.draw(circle);
            }

            if (points.size() >= 3) {
                std::vector<Point> convexHullPoints = convexHull(points);
                for (std::size_t i = 1; i < convexHullPoints.size(); ++i) {
                    sf::Vertex line[] = {
                        sf::Vertex(sf::Vector2f(convexHullPoints[i - 1].x, convexHullPoints[i - 1].y), sf::Color::Red),
                        sf::Vertex(sf::Vector2f(convexHullPoints[i].x, convexHullPoints[i].y), sf::Color::Red)
                    };
                    window.draw(line, 2, sf::Lines);
                }
                if (!convexHullPoints.empty()) {
                    sf::Vertex lastLine[] = {
                        sf::Vertex(sf::Vector2f(convexHullPoints.back().x, convexHullPoints.back().y), sf::Color::Red),
                        sf::Vertex(sf::Vector2f(convexHullPoints.front().x, convexHullPoints.front().y), sf::Color::Red)
                    };
                    window.draw(lastLine, 2, sf::Lines);
                }
            }
        }

        window.display();
    }


    return 0;
}
