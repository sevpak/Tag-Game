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
    float   timestamp;  // when this packet was sent
    uint8_t alive;
    uint8_t phase;
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