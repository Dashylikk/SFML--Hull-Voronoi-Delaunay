#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

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

int main() {
    std::vector<Point> points;
    double x, y;

    // Введення точок з файлу
    std::ifstream inputFile("points.txt");
    if (!inputFile.is_open()) {
        std::cerr << "Unable to open file!" << std::endl;
        return 1;
    }

    while (inputFile >> x >> y) {
        points.push_back({ x, y });
    }
    inputFile.close();

    // Перевірка наявності точок
    if (points.empty()) {
        std::cerr << "No points to display!" << std::endl;
        return 1;
    }

    // Побудова опуклої оболонки
    std::vector<Point> convexHullPoints = convexHull(points);

    // Відображення результату за допомогою SFML
    sf::RenderWindow window(sf::VideoMode(800, 600), "Convex Hull Visualization");
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();

        // Відображення точок
        for (const auto& point : points) {
            sf::CircleShape circle(3);
            circle.setFillColor(sf::Color::Blue);
            circle.setPosition(point.x, point.y);
            window.draw(circle);
        }

        // Відображення опуклої оболонки
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

    return 0;
}
