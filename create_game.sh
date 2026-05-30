#!/bin/bash
set -e

echo "Building game..."

# Create and enter build directory
mkdir -p build
cd build

# Run CMake and build
cmake .. -DCMAKE_BUILD_TYPE=Release
make

cd ..

echo "Creating package..."

# Create folder structure
rm -rf GamePackage
mkdir -p GamePackage/libs

# Copy binary and assets
cp build/game          GamePackage/
cp -r assets/          GamePackage/assets/
cp /System/Library/Fonts/Helvetica.ttc GamePackage/

# Copy SFML dylibs
cp /opt/homebrew/lib/libsfml-graphics.3.0.2.dylib  GamePackage/libs/
cp /opt/homebrew/lib/libsfml-window.3.0.2.dylib    GamePackage/libs/
cp /opt/homebrew/lib/libsfml-system.3.0.2.dylib    GamePackage/libs/
cp /opt/homebrew/lib/libsfml-network.3.0.2.dylib   GamePackage/libs/
cp /opt/homebrew/lib/libfreetype.6.dylib            GamePackage/libs/
cp /opt/homebrew/lib/libpng16.16.dylib              GamePackage/libs/

# Tell the binary to look for libs in ./libs at runtime
install_name_tool -add_rpath @executable_path/libs GamePackage/game

# Write install.sh
cat > GamePackage/install.sh << 'EOF'
#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
echo "Registering game with firewall (needs your password)..."
sudo /usr/libexec/ApplicationFirewall/socketfilterfw --add "$SCRIPT_DIR/game"
sudo /usr/libexec/ApplicationFirewall/socketfilterfw --unblockapp "$SCRIPT_DIR/game"
xattr -cr "$SCRIPT_DIR"
echo "Done! Run play.sh to launch."
EOF

# Write play.sh
cat > GamePackage/play.sh << 'EOF'
#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

RE