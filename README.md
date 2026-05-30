# Install dependencies
brew install sfml cmake

# Clone your repo
git clone https://github.com/YOUR_USERNAME/YOUR_REPO.git
cd YOUR_REPO

# Build
mkdir build && cd build
cmake ..
make

# Play
cd ..
./build/game join 34.20.157.190
