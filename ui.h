#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class UI {
public:
    UI(float W, float H);

    sf::Text& getStartText();

    void resetTimer();
    void updateTimer(float elapsed);
    void showGameOver(const std::string& message);

    void drawBorder(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window, bool roundStarted, bool gameOver);

private:
    float W, H;

    sf::Font             font;
    sf::RectangleShape   border;
    sf::Text             timerText;
    sf::Text             winText;
    sf::Text             startText;
};