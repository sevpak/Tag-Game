CXX      = g++
CXXFLAGS = -std=c++17 -I/opt/homebrew/include
LDFLAGS  = -L/opt/homebrew/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network
SRCS     = main.cpp player.cpp network.cpp ui.cpp
HDRS     = constants.h player.h network.h ui.h
TARGET   = game

$(TARGET): $(SRCS) $(HDRS)
	$(CXX) $(SRCS) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGET)