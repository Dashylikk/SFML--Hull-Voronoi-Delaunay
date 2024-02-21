#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <functional>
#include <random>

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

std::vector<Point> points;
std::vector<Point> convexHullPoints; 

int main() {
    sf::RenderWindow window(sf::VideoMode(2000, 1200), "Lab-1");

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
        int numPoints = 50; // Number of random points to generate
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

            // Display points
            for (const auto& point : points) {
                sf::CircleShape circle(5);
                circle.setFillColor(sf::Color::Blue);
                circle.setPosition(point.x, point.y);
                window.draw(circle);
            }

            // Build convex hull
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




    bool submenuButton1Clicked = false;
    bool submenuButton2Clicked = false;
    bool submenuButton3Clicked = false;

    // �������� ���� ��������
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
        if (submenuButton1Clicked && submenuButton2Clicked && submenuButton3Clicked) {
            submenuButton1Clicked = false;
            submenuButton2Clicked = false;
            submenuButton3Clicked = false;
        }

        window.clear(sf::Color::White);
        mainButton1.draw(window);
        mainButton2.draw(window);
        mainButton3.draw(window);
    
    

        if (!submenuButton1Clicked || !submenuButton2Clicked || !submenuButton3Clicked) {
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
