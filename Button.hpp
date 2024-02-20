#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <SFML/Graphics.hpp>
#include <string>

class Button {
public:
    Button(sf::Texture* normal, sf::Texture* clicked, std::string text, sf::Vector2f location);
    void checkClick(sf::Vector2f mousePos);
    void setState(bool state);
    void setText(std::string text);
    bool getState();
    sf::Sprite* getSprite();
    sf::Text* getText();
private:
    sf::Sprite normalSprite;
    sf::Sprite clickedSprite;
    sf::Sprite* currentSprite;
    sf::Text buttonText;
    bool currentState;
};

#endif
