#pragma once
#include <SFML/Network.hpp>
#include <optional>
#include <string>

enum class GamePhase : uint8_t {
    Waiting,    // waiting for both players
    Countdown,  // 3... 2... 1...
    Playing,    // game running
    GameOver
};

struct GameState {
    float x, y;
    bool      alive;
    GamePhase phase;
    uint8_t   countdown; // 3, 2, 1
};

class Network {
public:
    Network(bool isHost, bool isLocal, const std::string& serverIP);

    void send(const GameState& state);
    bool receive(GameState& out);

private:
    sf::UdpSocket  socket;
    sf::IpAddress  remoteAddr;
    unsigned short remotePort;
    bool           isHost;
    bool           isLocal;
};