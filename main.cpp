#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include "constants.h"
#include "player.h"
#include "network.h"
#include "ui.h"

int main(int argc, char* argv[]) {

    // --- Mode ---
    bool isHost  = (argc >= 2 && std::string(argv[1]) == "host");
    bool isLocal = (argc >= 2 && std::string(argv[1]) == "local");
    std::string serverIP = (argc >= 3) ? argv[2] : "127.0.0.1";

    // --- Window ---
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(desktop, "Ye vs Netanyahu", sf::State::Fullscreen);
    window.setFramerateLimit(60);
    bool isFullscreen = true;

    float W = static_cast<float>(desktop.size.x);
    float H = static_cast<float>(desktop.size.y);

    // --- Network ---
    Network net(isHost, isLocal, serverIP);

    // --- Game objects ---
    try {
        sf::Texture texture1("assets/hider.png");
        sf::Texture texture2("assets/seeker.png");

        Player me   ((isHost || isLocal) ? texture1 : texture2, (isHost || isLocal) ? 0.08f : 0.18f);
        Player them ((isHost || isLocal) ? texture2 : texture1, (isHost || isLocal) ? 0.18f : 0.08f);

        UI ui(W, H);

        sf::Clock clock;
        float elapsedTime  = 0.f;
        bool  roundStarted = false;
        bool  gameOver     = false;

        const float minX = BORDER, minY = BORDER;
        const float maxX = W - BORDER, maxY = H - BORDER;

        auto reset = [&]() {
            me.setPosition  ((isHost || isLocal) ? sf::Vector2f{minX, minY}
                                                : sf::Vector2f{maxX - them.getBounds().size.x,
                                                               maxY - them.getBounds().size.y});
            them.setPosition((isHost || isLocal) ? sf::Vector2f{maxX - them.getBounds().size.x,
                                                               maxY - them.getBounds().size.y}
                                                : sf::Vector2f{minX, minY});
            me.alive      = true;
            them.alive    = true;
            roundStarted  = false;
            gameOver      = false;
            elapsedTime   = 0.f;
            ui.resetTimer();
        };

        reset();

        sf::Vector2f theirTargetPos = them.getPosition();
        while (window.isOpen()) {

            // --- Network receive ---
            if (!isLocal) {
                GameState received{};
                if (net.receive(received)) {
                    theirTargetPos = {received.x, received.y};
                    them.alive = received.alive;
                }
                sf::Vector2f current = them.getPosition();
                sf::Vector2f delta   = theirTargetPos - current;
                them.setPosition(current + delta * 0.3f);
            }

            // --- Events ---
            while (const auto event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) window.close();
                if (event->is<sf::Event::KeyPressed>()) {
                    auto& code = event->getIf<sf::Event::KeyPressed>()->code;
                    if      (code == sf::Keyboard::Key::Escape) window.close();
                    else if (code == sf::Keyboard::Key::R)      reset();
                    else if (code == sf::Keyboard::Key::T) {
                        isFullscreen = !isFullscreen;
                        if (isFullscreen)
                            window.create(desktop, "Ye vs Netanyahu", sf::State::Fullscreen);
                        else
                            window.create(sf::VideoMode({1280, 720}), "Ye vs Netanyahu");
                        window.setFramerateLimit(60);
                    }
                    else if (!roundStarted && !gameOver) roundStarted = true;
                }
            }

            float dt = clock.restart().asSeconds();

            // --- Update ---
            if (roundStarted && !gameOver) {
                me.handleInput(SPEED1, dt);
                me.clamp(minX, minY, maxX, maxY);

                if (isLocal) {
                    them.handleInputP2(SPEED2, dt);
                    them.clamp(minX, minY, maxX, maxY);
                }

                if (me.alive) {
                    elapsedTime += dt;
                    ui.updateTimer(elapsedTime);
                }

                // Collision
                if (me.alive && me.getBounds().findIntersection(them.getBounds())) {
                    if (isHost || isLocal) {
                        me.alive = false;
                        gameOver = true;
                        ui.showGameOver(isLocal ? "Netanyahu got Ye!\nPress R to restart"
                                                : "Game Over! Netanyahu got you!\nPress R to restart");
                    }
                }
                if (!isLocal && !isHost && !them.alive) {
                    gameOver = true;
                    ui.showGameOver("You caught him!\nPress R to restart");
                }
            }

            // --- Network send ---
            if (!isLocal) {
                net.send({me.getPosition().x, me.getPosition().y, me.alive});
            }

            // --- Draw ---
            window.clear(sf::Color::Black);
            ui.drawBorder(window);
            if (me.alive)   me.draw(window);
            if (them.alive) them.draw(window);
            ui.draw(window, roundStarted, gameOver);
            window.setTitle("Ye vs Netanyahu | FPS: " + std::to_string((int)(1.f / dt)));
            window.display();
        }

    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return -1;
    }
}