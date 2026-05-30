cmake_minimum_required(VERSION 3.16)
project(YeVsNetanyahu)

set(CMAKE_CXX_STANDARD 17)

# Find SFML automatically — works on both ARM and Intel Mac
find_package(SFML 3 COMPONENTS Graphics Window System Network REQUIRED)

# Your source files
add_executable(game
    main.cpp
    player.cpp
    network.cpp
    ui.cpp
)

# Link SFML
target_link_libraries(game
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Network
)

# Copy assets next to the binary automatically
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
file(COPY /System/Library/Fonts/Helvetica.ttc DESTINATION ${CMAKE_BINARY_DIR})