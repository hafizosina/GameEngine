#!/bin/bash
set -e

mkdir -p vendor
cd vendor

echo "Fetching nlohmann_json..."
if [ ! -d "nlohmann_json" ]; then
    git clone --depth 1 --branch v3.11.3 https://github.com/nlohmann/json.git nlohmann_json
fi

echo "Fetching EnTT..."
if [ ! -d "entt" ]; then
    git clone --depth 1 --branch v3.13.0 https://github.com/skypjack/entt.git entt
fi

echo "Fetching spdlog..."
if [ ! -d "spdlog" ]; then
    git clone --depth 1 --branch v1.13.0 https://github.com/gabime/spdlog.git spdlog
fi

echo "Fetching raylib..."
if [ ! -d "raylib" ]; then
    git clone --depth 1 --branch 5.0 https://github.com/raysan5/raylib.git raylib
    cd raylib/src
    make PLATFORM=PLATFORM_DESKTOP -j$(nproc)
    cd ../..
fi

echo "Fetching box2d..."
if [ ! -d "box2d" ]; then
    git clone --depth 1 --branch v2.4.1 https://github.com/erincatto/box2d.git box2d
    cd box2d
    mkdir -p build && cd build
    cmake -DBOX2D_BUILD_UNIT_TESTS=OFF -DBOX2D_BUILD_TESTBED=OFF ..
    make -j$(nproc)
    cd ../../../
fi

echo "Vendor setup complete!"
