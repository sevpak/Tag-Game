#include <string.h>
#include "ui.h"
#include "constants.h"

UI::UI(float W, float H)
    : W(W)
    , H(H)
    , font("/System/Library/Fonts/Helvetica.ttc")
    , timerText(font, "0.0s", 36)
    , winText(font, "", 48)
    , startText(font, "Press any key to start!", 36)
{
    // Border
    border.setSize({W - BORDER * 2, H - BORDER * 2});
    border.setPosition({BORDER, BORDER});
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(50, 50, 50));
    border.setOutlineThickness(BORDER);

    // Timer — centered at top
    timerText.setFillColor(sf::Color::White);
    sf::FloatRect tb = timerText.getLocalBounds();
    timerText.setOrigin({tb.size.x / 2.f, 0.f});
    timerText.setPosition({W / 2.f, BORDER + 22.f});

    // Win text — centered middle
    winText.setFillColor(sf::Color::Yellow);
    winText.setScale({0.5f, 0.5f});

    // Start text — centered middle
    startText.setFillColor(sf::Color::White);
    sf::FloatRect sb = startText.getLocalBounds();
    startText.setOrigin({sb.size.x / 2.f, sb.size.y / 2.f});
    startText.setPosition({W / 2.f, H / 2.f});
}

void UI::resetTimer() {
    timerText.setString("0.0s");
    sf::FloatRect tb = timerText.getLocalBounds();
    timerText.setOrigin({tb.size.x / 2.f, 0.f});
}

void UI::updateTimer(float elapsed) {
    std::string timeStr = std::to_string((int)elapsed) + "." +
                          std::to_string((int)(elapsed * 10) % 10) + "s";
    timerText.setString(timeStr);

    // Re-center since string width changes as digits increase
    sf::FloatRect tb = timerText.getLocalBounds();
    timerText.setOrigin({tb.size.x / 2.f, 0.f});
}

void UI::showGameOver(const std::string& message) {
    winText.setString(message);
    sf::FloatRect wb = winText.getLocalBounds();
    winText.setOrigin({wb.size.x / 2.f, wb.size.y / 2.f});
    winText.setPosition({W / 2.f, H / 2.f});
}

void UI::drawBorder(sf::RenderWindow& window) {
    window.draw(border);
}

void UI::draw(sf::RenderWindow& window, bool roundStarted, bool gameOver) {
    window.draw(timerText);

    if (gameOver)
        window.draw(winText);

    if (!roundStarted && !gameOver)
        window.draw(startText);
}

sf::Text& UI::getStartText() {
    startText.setString("Waiting for other player...");
    sf::FloatRect sb = startText.getLocalBounds();
    startText.setOrigin({sb.size.x / 2.f, sb.size.y / 2.f});
    startText.setPosition({W / 2.f, H / 2.f});
    return startText;
}