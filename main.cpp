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

        sf::Font  font("/System/Library/Fonts/Helvetica.ttc");
        sf::Text  countdownText(font, "", 96);
        countdownText.setFillColor(sf::Color::White);

        auto centerText = [&](sf::Text& t) {
            sf::FloatRect b = t.getLocalBounds();
            t.setOrigin({b.size.x / 2.f, b.size.y / 2.f});
            t.setPosition({W / 2.f, H / 2.f - 60.f});
        };

        Player me   ((isHost || isLocal) ? texture1 : texture2, (isHost || isLocal) ? 0.08f : 0.18f);
        Player them ((isHost || isLocal) ? texture2 : texture1, (isHost || isLocal) ? 0.18f : 0.08f);

        UI ui(W, H);

        sf::Clock clock;
        float elapsedTime  = 0.f;
        GamePhase phase       = GamePhase::Waiting;
        float     phaseTimer  = 0.f;
        bool      peerReady   = false; 

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
            phase           = GamePhase::Waiting;
            phaseTimer      = 0.f;
            peerReady       = false;
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
                    them.alive     = received.alive;

                    if (!isHost) {
                        // joiner just follows host's phase
                        phase           = received.phase;
                        uint8_t cd      = received.countdown;
                        if (phase == GamePhase::Countdown) {
                            countdownText.setString(std::to_string(cd));
                            centerText(countdownText);
                        }
                    } else {
                        // host just seeing a packet means peer is connected
                        peerReady = true;
                    }
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
                }
            }

            float dt = clock.restart().asSeconds();

            // --- Phase logic (host drives, joiner follows) ---
            if (isHost || isLocal) {
                if (phase == GamePhase::Waiting && (peerReady || isLocal)) {
                    phase      = GamePhase::Countdown;
                    phaseTimer = 3.f;
                    countdownText.setString("3");
                    centerText(countdownText);
                }

                if (phase == GamePhase::Countdown) {
                    phaseTimer -= dt;
                    uint8_t cd = (uint8_t)std::ceil(phaseTimer);
                    countdownText.setString(std::to_string(cd));
                    centerText(countdownText);
                    if (phaseTimer <= 0.f) {
                        phase = GamePhase::Playing;
                        countdownText.setString("");
                    }
                }
            }

            // --- Update ---
            if (phase == GamePhase::Playing) {
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

                if (me.alive && me.getBounds().findIntersection(them.getBounds())) {
                    if (isHost || isLocal) {
                        me.alive = false;
                        phase    = GamePhase::GameOver;
                        ui.showGameOver(isLocal ? "Netanyahu got Ye!\nPress R to restart"
                                                : "Game Over! Netanyahu got you!\nPress R to restart");
                    }
                }
                if (!isLocal && !isHost && !them.alive) {
                    phase = GamePhase::GameOver;
                    ui.showGameOver("You caught him!\nPress R to restart");
                }
            }

            // --- Network send ---
            if (!isLocal) {
                uint8_t cd = (phase == GamePhase::Countdown) 
                            ? (uint8_t)std::ceil(phaseTimer) : 0;
                net.send({me.getPosition().x, me.getPosition().y, me.alive, phase, cd});
            }

            // --- Draw ---
            window.clear(sf::Color::Black);
            ui.drawBorder(window);
            if (me.alive)   me.draw(window);
            if (them.alive) them.draw(window);
            ui.draw(window, phase == GamePhase::Playing, phase == GamePhase::GameOver);

            if (phase == GamePhase::Waiting)
                window.draw(ui.getStartText()); // waiting for other player
            if (phase == GamePhase::Countdown)
                window.draw(countdownText);

            window.setTitle("Ye vs Netanyahu | FPS: " + std::to_string((int)(1.f / dt)));
            window.display();
        }

    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return -1;
    }
}