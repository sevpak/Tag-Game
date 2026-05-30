#!/bin/bash
set -e

echo "Building game..."

mkdir -p build
cd build

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
sudo xattr -cr "$SCRIPT_DIR"
echo "Done! Run play.sh to launch."
EOF

# Write play.sh
cat > GamePackage/play.sh << 'EOF'
#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

RELAY_IP="34.20.157.190"

echo ""
echo "================================"
echo "  Ye vs Netanyahu"
echo "================================"
echo ""
echo "How do you want to play?"
echo "  1) Local  (both players on this machine)"
echo "  2) Online (via relay server)"
echo ""
read -p "Enter 1 or 2: " mode

if [ "$mode" = "1" ]; then
    ./game local

else
    echo ""
    echo "Are you hosting or joining?"
    echo "  1) Host"
    echo "  2) Join"
    echo ""
    read -p "Enter 1 or 2: " choice

    if [ "$choice" = "1" ]; then
        ./game host "$RELAY_IP"
    else
        ./game join "$RELAY_IP"
    fi
fi
EOF

chmod +x GamePackage/install.sh GamePackage/play.sh

# Zip it up
zip -r GamePackage.zip GamePackage/
rm -rf GamePackage

echo ""
echo "Done! GamePackage.zip is ready to send."
echo "Your friend should:"
echo "  1. Unzip it"
echo "  2. Run install.sh once"
echo "  3. Run play.sh to play"