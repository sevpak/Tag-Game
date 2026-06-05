#include "network.h"
#include "constants.h"
#include <iostream>

Network::Network(bool isHost, bool isLocal, const std::string& serverIP)
    : isHost(isHost)
    , isLocal(isLocal)
    , remoteAddr(sf::IpAddress::Any)
    , remotePort(PORT_HOST)  // everyone uses the same port now
{
    if (isLocal) return;

    socket.setBlocking(false);

    // Everyone binds to any available local port
    if (socket.bind(sf::Socket::AnyPort) != sf::Socket::Status::Done) {
        std::cerr << "Failed to bind socket" << std::endl;
        return;
    }

    // Everyone connects to the relay server
    auto resolved = sf::IpAddress::resolve(serverIP);
    if (resolved.has_value())
        remoteAddr = resolved.value();
    else
        std::cerr << "Could not resolve relay server IP" << std::endl;

    std::cout << (isHost ? "Hosting" : "Joining") << " via relay: " << serverIP << std::endl;
}

void Network::send(const GameState& state) {
    if (isLocal) return;
    if (remoteAddr == sf::IpAddress::Any) return;

    std::cout << "Sending " << sizeof(GameState) << " bytes to relay" << std::endl;

    if (socket.send(&state, sizeof(GameState), remoteAddr, remotePort)
        != sf::Socket::Status::Done) {
        // UDP send failures are non-fatal — just skip the frame
    }
}

bool Network::receive(GameState& out) {
    if (isLocal) return false;

    std::size_t received;
    std::optional<sf::IpAddress> sender;
    unsigned short senderPort;

    auto status = socket.receive(&out, sizeof(GameState), received, sender, senderPort);
    
    if (status == sf::Socket::Status::Done) {
        std::cout << "Received " << received << " bytes, expected " << sizeof(GameState) << std::endl;
        return true;
    }
    return false;
}