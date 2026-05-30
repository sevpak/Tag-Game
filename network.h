#pragma once
#include <SFML/Network.hpp>
#include <optional>
#include <string>

struct GameState {
    float x, y;
    bool  alive;
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