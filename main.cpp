#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
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

void buildConvexHull(std::vector<Point>& convexHullPoints, std::vector<Point>& points, sf::RenderWindow& window) {
    convexHullPoints = convexHull(points);

    window.clear();

    for (const auto& point : points) {
        sf::CircleShape circle(5);
        circle.setFillColor(sf::Color::Blue);
        circle.setPosition(point.x, point.y);
        window.draw(circle);
    }

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
namespace delaunay {

    constexpr double eps = 1e-4;

    template <typename T>
    struct Point {
        T x, y;

        Point() : x{ 0 }, y{ 0 } {}
        Point(T _x, T _y) : x{ _x }, y{ _y } {}

        template <typename U>
        Point(U _x, U _y) : x{ static_cast<T>(_x) }, y{ static_cast<T>(_y) }
        {
        }

        friend std::ostream& operator<<(std::ostream& os, const Point<T>& p)
        {
            os << "x=" << p.x << "  y=" << p.y;
            return os;
        }

        bool operator==(const Point<T>& other) const
        {
            return (other.x == x && other.y == y);
        }

        bool operator!=(const Point<T>& other) const { return !operator==(other); }
    };

    template <typename T>
    struct Edge {
        using Node = Point<T>;
        Node p0, p1;

        Edge(Node const& _p0, Node const& _p1) : p0{ _p0 }, p1{ _p1 } {}

        friend std::ostream& operator<<(std::ostream& os, const Edge& e)
        {
            os << "p0: [" << e.p0 << " ] p1: [" << e.p1 << "]";
            return os;
        }

        bool operator==(const Edge& other) const
        {
            return ((other.p0 == p0 && other.p1 == p1) ||
                (other.p0 == p1 && other.p1 == p0));
        }
    };

    template <typename T>
    struct Circle {
        T x, y, radius;
        Circle() = default;
    };

    template <typename T>
    struct Triangle {
        using Node = Point<T>;
        Node p0, p1, p2;
        Edge<T> e0, e1, e2;
        Circle<T> circle;

        Triangle(const Node& _p0, const Node& _p1, const Node& _p2)
            : p0{ _p0 },
            p1{ _p1 },
            p2{ _p2 },
            e0{ _p0, _p1 },
            e1{ _p1, _p2 },
            e2{ _p0, _p2 },
            circle{}
        {
            const auto ax = p1.x - p0.x;
            const auto ay = p1.y - p0.y;
            const auto bx = p2.x - p0.x;
            const auto by = p2.y - p0.y;

            const auto m = p1.x * p1.x - p0.x * p0.x + p1.y * p1.y - p0.y * p0.y;
            const auto u = p2.x * p2.x - p0.x * p0.x + p2.y * p2.y - p0.y * p0.y;
            const auto s = 1. / (2. * (ax * by - ay * bx));

            circle.x = ((p2.y - p0.y) * m + (p0.y - p1.y) * u) * s;
            circle.y = ((p0.x - p2.x) * m + (p1.x - p0.x) * u) * s;

            const auto dx = p0.x - circle.x;
            const auto dy = p0.y - circle.y;
            circle.radius = dx * dx + dy * dy;
        }
    };

    template <typename T>
    struct Delaunay {
        std::vector<Triangle<T>> triangles;
        std::vector<Edge<T>> edges;
    };

    template <
        typename T,
        typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
    Delaunay<T> triangulate(const std::vector<Point<T>>& points)
    {
        using Node = Point<T>;
        if (points.size() < 3) {
            return Delaunay<T>{};
        }
        auto xmin = points[0].x;
        auto xmax = xmin;
        auto ymin = points[0].y;
        auto ymax = ymin;
        for (auto const& pt : points) {
            xmin = std::min(xmin, pt.x);
            xmax = std::max(xmax, pt.x);
            ymin = std::min(ymin, pt.y);
            ymax = std::max(ymax, pt.y);
        }

        const auto dx = xmax - xmin;
        const auto dy = ymax - ymin;
        const auto dmax = std::max(dx, dy);
        const auto midx = (xmin + xmax) / static_cast<T>(2.);
        const auto midy = (ymin + ymax) / static_cast<T>(2.);

        auto d = Delaunay<T>{};

        const auto p0 = Node{ midx - 20 * dmax, midy - dmax };
        const auto p1 = Node{ midx, midy + 20 * dmax };
        const auto p2 = Node{ midx + 20 * dmax, midy - dmax };
        d.triangles.emplace_back(Triangle<T>{p0, p1, p2});

        for (auto const& pt : points) {
            std::vector<Edge<T>> edges;
            std::vector<Triangle<T>> tmps;
            for (auto const& tri : d.triangles) {

                const auto dist = (tri.circle.x - pt.x) * (tri.circle.x - pt.x) +
                    (tri.circle.y - pt.y) * (tri.circle.y - pt.y);
                if ((dist - tri.circle.radius) <= eps) {
                    edges.push_back(tri.e0);
                    edges.push_back(tri.e1);
                    edges.push_back(tri.e2);
                }
                else {
                    tmps.push_back(tri);
                }
            }

            std::vector<bool> remove(edges.size(), false);
            for (auto it1 = edges.begin(); it1 != edges.end(); ++it1) {
                for (auto it2 = edges.begin(); it2 != edges.end(); ++it2) {
                    if (it1 == it2) {
                        continue;
                    }
                    if (*it1 == *it2) {
                        remove[std::distance(edges.begin(), it1)] = true;
                        remove[std::distance(edges.begin(), it2)] = true;
                    }
                }
            }

            edges.erase(
                std::remove_if(edges.begin(), edges.end(),
                    [&](auto const& e) { return remove[&e - &edges[0]]; }),
                edges.end());


            for (auto const& e : edges) {
                tmps.push_back({ e.p0, e.p1, {pt.x, pt.y} });
            }
            d.triangles = tmps;
        }

        d.triangles.erase(
            std::remove_if(d.triangles.begin(), d.triangles.end(),
                [&](auto const& tri) {
                    return ((tri.p0 == p0 || tri.p1 == p0 || tri.p2 == p0) ||
                        (tri.p0 == p1 || tri.p1 == p1 || tri.p2 == p1) ||
                        (tri.p0 == p2 || tri.p1 == p2 || tri.p2 == p2));
                }),
            d.triangles.end());

        for (auto const& tri : d.triangles) {
            d.edges.push_back(tri.e0);
            d.edges.push_back(tri.e1);
            d.edges.push_back(tri.e2);
        }
        return d;
    }

}

std::vector<delaunay::Point<double>> readPointsFromFile(const std::string& filename) {
    std::vector<delaunay::Point<double>> points;
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            double x, y;
            if (iss >> x >> y) {
                points.push_back(delaunay::Point<double>(x, y));
            }
        }
        file.close();
    }
    else {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }
    return points;
}
void drawTrianglesAndPoints(const delaunay::Delaunay<double>& triangulation, const std::vector<delaunay::Point<double>>& points, sf::RenderWindow& window) {
    for (const auto& triangle : triangulation.triangles) {
        sf::ConvexShape shape(3);
        shape.setPoint(0, sf::Vector2f(triangle.p0.x, triangle.p0.y));
        shape.setPoint(1, sf::Vector2f(triangle.p1.x, triangle.p1.y));
        shape.setPoint(2, sf::Vector2f(triangle.p2.x, triangle.p2.y));
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineColor(sf::Color::Black);
        shape.setOutlineThickness(1);
        window.draw(shape);
    }

    for (const auto& point : points) {
        sf::CircleShape circle(2);
        circle.setFillColor(sf::Color::Black);
        circle.setPosition(point.x - 2, point.y - 2);
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
        sf::RenderWindow window(sf::VideoMode(width, height), "Ñonvex Hull(clicking)");
        points.clear();
        readPointsFromFile(points);
        buildConvexHull(convexHullPoints, points, window);
        });

    submenuButton2.setAction([&window]() {
        sf::RenderWindow window(sf::VideoMode(width, height), "Ñonvex Hull(clicking)");
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

        sf::RenderWindow window(sf::VideoMode(width, height), "Ñonvex Hull(clicking)");
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

        sf::RenderWindow window(sf::VideoMode(width, height), "Voronoi Diagram(txt)");
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
        sf::RenderWindow window(sf::VideoMode(width, height), "Voronoi Diagram(random)");
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

        sf::RenderWindow window(sf::VideoMode(width, height), "Voronoi Diagram(clicking)");
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
    submenuButton7.setAction([&]() {
        std::vector<delaunay::Point<double>> points = readPointsFromFile("points.txt");
        delaunay::Delaunay<double> triangulation = delaunay::triangulate(points);
        sf::RenderWindow window(sf::VideoMode(width, height), "Delaunay Triangulation(txt)");

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            window.clear(sf::Color::White);

            for (const auto& triangle : triangulation.triangles) {
                sf::ConvexShape shape(3);
                shape.setPoint(0, sf::Vector2f(triangle.p0.x, triangle.p0.y));
                shape.setPoint(1, sf::Vector2f(triangle.p1.x, triangle.p1.y));
                shape.setPoint(2, sf::Vector2f(triangle.p2.x, triangle.p2.y));
                shape.setFillColor(sf::Color::Transparent);
                shape.setOutlineColor(sf::Color::Black);
                shape.setOutlineThickness(2);
                window.draw(shape);
            }

            for (const auto& point : points) {
                sf::CircleShape circle(4);
                circle.setFillColor(sf::Color::Black);
                circle.setPosition(point.x - 4, point.y - 4);
                window.draw(circle);
            }

            window.display();
        }
        });
    submenuButton8.setAction([&]() {
        std::vector<delaunay::Point<double>> points;
        srand(time(NULL));
        for (int i = 0; i < numPoints; ++i) {
            double x = rand() % width;
            double y = rand() % height;
            points.push_back(delaunay::Point<double>(x, y));
        }

        delaunay::Delaunay<double> triangulation = delaunay::triangulate(points);

        sf::RenderWindow window(sf::VideoMode(width, height), "Delaunay Triangulation(random)");

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            window.clear(sf::Color::White);

            drawTrianglesAndPoints(triangulation, points, window);

            window.display();
        }
        });
    submenuButton9.setAction([&]() {
        sf::RenderWindow window(sf::VideoMode(width, height), "Delaunay Triangulation(clicking)");

        std::vector<delaunay::Point<double>> points;

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        double x = event.mouseButton.x;
                        double y = event.mouseButton.y;
                        points.push_back(delaunay::Point<double>(x, y));
                    }
                }
            }

            delaunay::Delaunay<double> triangulation = delaunay::triangulate(points);
            window.clear(sf::Color::White);

            for (const auto& triangle : triangulation.triangles) {
                sf::ConvexShape shape(3);
                shape.setPoint(0, sf::Vector2f(triangle.p0.x, triangle.p0.y));
                shape.setPoint(1, sf::Vector2f(triangle.p1.x, triangle.p1.y));
                shape.setPoint(2, sf::Vector2f(triangle.p2.x, triangle.p2.y));
                shape.setFillColor(sf::Color::Transparent);
                shape.setOutlineColor(sf::Color::Black);
                shape.setOutlineThickness(2);
                window.draw(shape);
            }

            for (const auto& point : points) {
                sf::CircleShape circle(4);
                circle.setFillColor(sf::Color::Black);
                circle.setPosition(point.x - 4, point.y - 4);
                window.draw(circle);
            }

            window.display();
        }
        });
    bool submenuButton1Clicked = false;
    bool submenuButton2Clicked = false;
    bool submenuButton3Clicked = false;
    bool submenuButton4Clicked = false;
    bool submenuButton5Clicked = false;
    bool submenuButton6Clicked = false;

    // Îñíîâíèé öèêë ïðîãðàìè
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
