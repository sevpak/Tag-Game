#include "player.h"

Player::Player(sf::Texture& texture, float scale)
    : sprite(texture) {
    sprite.setScale({scale, scale});
}

void Player::handleInput(float speed, float dt) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) sprite.move({0.f, -speed * dt});
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) sprite.move({0.f,  speed * dt});
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) sprite.move({-speed * dt, 0.f});
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) sprite.move({ speed * dt, 0.f});
}

void Player::handleInputP2(float speed, float dt) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::I)) sprite.move({0.f, -speed * dt});
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K)) sprite.move({0.f,  speed * dt});
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J)) sprite.move({-speed * dt, 0.f});
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::L)) sprite.move({ speed * dt, 0.f});
}

void Player::clamp(float minX, float minY, float maxX, float maxY) {
    sf::FloatRect b   = sprite.getGlobalBounds();
    sf::Vector2f  pos = sprite.getPosition();
    if (b.position.x < minX)             sprite.setPosition({minX, pos.y});
    if (b.position.y < minY)             sprite.setPosition({pos.x, minY});
    if (b.position.x + b.size.x > maxX) sprite.setPosition({maxX - b.size.x, pos.y});
    if (b.position.y + b.size.y > maxY) sprite.setPosition({pos.x, maxY - b.size.y});
}

void Player::setPosition(sf::Vector2f pos) { sprite.setPosition(pos); }
sf::Vector2f  Player::getPosition() const  { return sprite.getPosition(); }
sf::FloatRect Player::getBounds()   const  { return sprite.getGlobalBounds(); }
void Player::draw(sf::RenderWindow& w)     { w.draw(sprite); }