#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <vector>
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

    // --- Coordinate conversion ---
    auto toScreen = [&](sf::Vector2f gamePos) -> sf::Vector2f {
        return {gamePos.x * (W / GAME_W), gamePos.y * (H / GAME_H)};
    };
    auto toGame = [&](sf::Vector2f screenPos) -> sf::Vector2f {
        return {screenPos.x * (GAME_W / W), screenPos.y * (GAME_H / H)};
    };

    // --- Network ---
    Network net(isHost, isLocal, serverIP);

    // --- Game objects ---
    try {
        sf::Texture texture1("assets/hider.png");
        sf::Texture texture2("assets/seeker.png");

        sf::Font font("/System/Library/Fonts/Helvetica.ttc");
        sf::Text countdownText(font, "", 96);
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
        float     elapsedTime = 0.f;
        float     gameTime    = 0.f;
        GamePhase phase       = GamePhase::Waiting;
        float     phaseTimer  = 0.f;
        bool      peerReady   = false;

        // Position history for lag compensation
        struct PositionSnapshot {
            sf::Vector2f gamePos;
            float        timestamp;
        };
        std::vector<PositionSnapshot> myHistory;
        myHistory.reserve(120);

        // Game world bounds
        const float gameMinX = BORDER * (GAME_W / 1920.f);
        const float gameMinY = BORDER * (GAME_H / 1080.f);
        const float gameMaxX = GAME_W - gameMinX;
        const float gameMaxY = GAME_H - gameMinY;

        // Screen bounds for clamping
        const float minX = BORDER, minY = BORDER;
        const float maxX = W - BORDER, maxY = H - BORDER;

        auto reset = [&]() {
            sf::Vector2f theirGameSize = toGame(them.getBounds().size);
            me.setPosition  ((isHost || isLocal)
                ? toScreen({gameMinX, gameMinY})
                : toScreen({gameMaxX - theirGameSize.x, gameMaxY - theirGameSize.y}));
            them.setPosition((isHost || isLocal)
                ? toScreen({gameMaxX - theirGameSize.x, gameMaxY - theirGameSize.y})
                : toScreen({gameMinX, gameMinY}));
            me.alive    = true;
            them.alive  = true;
            phase       = GamePhase::Waiting;
            phaseTimer  = 0.f;
            peerReady   = false;
            elapsedTime = 0.f;
            gameTime    = 0.f;
            myHistory.clear();
            ui.resetTimer();
        };

        reset();

        sf::Vector2f theirTargetPos = them.getPosition();

        while (window.isOpen()) {

            // --- Network receive ---
            if (!isLocal) {
                GameState received{};
                if (net.receive(received)) {
                    theirTargetPos = toScreen({received.x, received.y});
                    them.alive     = (bool)received.alive;

                    // Lag compensated collision — host only
                    if ((isHost) && phase == GamePhase::Playing && me.alive) {
                        float theirTime = received.timestamp;

                        // Find where we were when their packet was sent
                        sf::Vector2f myPastGamePos = toGame(me.getPosition());
                        for (int i = (int)myHistory.size() - 1; i >= 0; i--) {
                            if (myHistory[i].timestamp <= theirTime) {
                                myPastGamePos = myHistory[i].gamePos;
                                break;
                            }
                        }

                        // Check collision at past position
                        sf::FloatRect myPastBounds   = me.getBounds();
                        myPastBounds.position        = toScreen(myPastGamePos);
                        sf::FloatRect theirBounds    = them.getBounds();
                        theirBounds.position         = toScreen({received.x, received.y});

                        if (myPastBounds.findIntersection(theirBounds)) {
                            me.alive = false;
                            phase    = GamePhase::GameOver;
                            ui.showGameOver("Game Over! Netanyahu got you!\nPress R to restart");
                        }
                    }

                    if (!isHost) {
                        GamePhase newPhase = (GamePhase)received.phase;
                        if (newPhase == GamePhase::GameOver && phase != GamePhase::GameOver) {
                            phase = GamePhase::GameOver;
                            ui.showGameOver("You caught him!\nPress R to restart");
                        } else {
                            phase = newPhase;
                        }
                        if (phase == GamePhase::Countdown) {
                            countdownText.setString(std::to_string(received.countdown));
                            centerText(countdownText);
                        }
                        if (phase == GamePhase::Playing)
                            countdownText.setString("");
                    } else {
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

                // Record position history for lag compensation
                gameTime += dt;
                myHistory.push_back({toGame(me.getPosition()), gameTime});
                if (myHistory.size() > 120)
                    myHistory.erase(myHistory.begin());

                if (isLocal) {
                    them.handleInputP2(SPEED2, dt);
                    them.clamp(minX, minY, maxX, maxY);
                }

                if (me.alive) {
                    elapsedTime += dt;
                    ui.updateTimer(elapsedTime);
                }

                // Local collision (for local mode only)
                if (isLocal && me.alive && me.getBounds().findIntersection(them.getBounds())) {
                    me.alive = false;
                    phase    = GamePhase::GameOver;
                    ui.showGameOver("Netanyahu got Ye!\nPress R to restart");
                }
            }

            // --- Network send ---
            if (!isLocal) {
                sf::Vector2f gamePos = toGame(me.getPosition());
                uint8_t cd = (phase == GamePhase::Countdown)
                             ? (uint8_t)std::ceil(phaseTimer) : 0;
                net.send({gamePos.x, gamePos.y, gameTime,
                          (uint8_t)me.alive, (uint8_t)phase, cd});
            }

            // --- Draw ---
            window.clear(sf::Color::Black);
            ui.drawBorder(window);
            if (me.alive)   me.draw(window);
            if (them.alive) them.draw(window);
            ui.draw(window, phase == GamePhase::Playing, phase == GamePhase::GameOver);

            if (phase == GamePhase::Waiting)
                window.draw(ui.getStartText());
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