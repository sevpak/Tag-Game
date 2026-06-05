#pragma once
#include <SFML/Network.hpp>
#include <optional>
#include <string>

enum class GamePhase : uint8_t {
    Waiting,
    Countdown,
    Playing,
    GameOver
};

#pragma pack(push, 1)
struct GameState {
    float   x, y;
    uint8_t alive;      // use uint8_t instead of bool
    uint8_t phase;      // use uint8_t instead of GamePhase enum
    uint8_t countdown;
};
#pragma pack(pop)

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