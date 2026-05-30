#pragma once
#include <SFML/Graphics.hpp>

class Player {
public:
    Player(sf::Texture& texture, float scale);

    void handleInput(float speed, float dt);
    void handleInputP2(float speed, float dt);
    void clamp(float minX, float minY, float maxX, float maxY);
    void setPosition(sf::Vector2f pos);
    sf::Vector2f  getPosition() const;
    sf::FloatRect getBounds() const;
    void draw(sf::RenderWindow& window);

    bool alive = true;

private:
    sf::Sprite sprite;
};