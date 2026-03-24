#!/bin/bash
# Helper script to build Dune Legacy with installer
# Usage: ./scripts/build.sh [clean]

set -e

cd "$(dirname "$0")/.."

if [ "$1" == "clean" ]; then
    echo "🧹 Cleaning build directory..."
    rm -rf build-vcpkg
fi

if [ ! -d "build-vcpkg" ]; then
    echo "🔧 Configuring CMake with vcpkg..."
    cmake -B build-vcpkg -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
        -DVCPKG_TARGET_TRIPLET=arm64-osx-dynamic
fi

echo "🚀 Building Dune Legacy + DMG installer..."
echo "   (CPack cache will be cleaned automatically)"
cmake --build build-vcpkg --target dmg --config Release

echo ""
echo "✅ Build complete!"
echo "📦 DMG: $(ls -lh build-vcpkg/*.dmg 2>/dev/null | awk '{print $9, "(" $5 ")"}')"
echo ""
echo "💡 The DMG is guaranteed fresh (CPack cache cleaned during build)"
echo ""
echo "To install: cp -R build-vcpkg/install/dunelegacy.app /Applications/"
echo "To run:     open build-vcpkg/install/dunelegacy.app"

