#include <SFML/Network.hpp>
#include <iostream>
#include <optional>

int main() {
    const unsigned short PORT = 54000;

    sf::UdpSocket socket;
    socket.setBlocking(true);

    if (socket.bind(PORT) != sf::Socket::Status::Done) {
        std::cerr << "Failed to bind port " << PORT << std::endl;
        return -1;
    }

    std::cout << "Relay server running on port " << PORT << std::endl;

    std::optional<sf::IpAddress> player1Addr, player2Addr;
    unsigned short player1Port = 0, player2Port = 0;

    char buffer[1024];

    while (true) {
        std::size_t received;
        sf::IpAddress sender;        // plain IpAddress for SFML 2
        unsigned short senderPort;

        if (socket.receive(buffer, sizeof(buffer), received, sender, senderPort)
            != sf::Socket::Status::Done) continue;

        // First packet = player 1
        // Replace the player registration block with this:
        bool isPlayer1 = player1Addr.has_value() && 
                        sender == player1Addr.value() && 
                        senderPort == player1Port;

        bool isPlayer2 = player2Addr.has_value() && 
                        sender == player2Addr.value() && 
                        senderPort == player2Port;

        if (!player1Addr.has_value()) {
            player1Addr = sender;
            player1Port = senderPort;
            std::cout << "Player 1 connected: " << sender << ":" << senderPort << std::endl;
        } else if (!player2Addr.has_value() && !isPlayer1) {
            player2Addr = sender;
            player2Port = senderPort;
            std::cout << "Player 2 connected: " << sender << ":" << senderPort << std::endl;
        }

        // Forward to the other player
        if (player1Addr.has_value() && player2Addr.has_value()) {
            if (isPlayer1)
                socket.send(buffer, received, player2Addr.value(), player2Port);
            else
                socket.send(buffer, received, player1Addr.value(), player1Port);
        }
    }
}