/*
 *  globals_test_stub.cpp - Minimal global variable definitions for test binary
 *
 *  CitySimulation.cpp uses pTextManager and other globals via globals.h.
 *  This stub provides null-pointer definitions so the test binary links
 *  without needing the full game runtime (SDL, audio, file managers, etc.).
 *
 *  Key globals needed:
 *    - pTextManager: used via _() macro in milestone strings
 *    - currentGame / currentGameMap: null in test environment
 */

class Game;
class Map;

// Provide a minimal pTextManager stub that satisfies the linker.
// The real TextManager lives in src/FileClasses/TextManager.cpp (not compiled in tests).
// We define a minimal TextManager here so _() macro calls in CitySimulation.cpp compile.
#include <memory>
#include <string>

class TextManager {
public:
    // Minimal stub: identity function, matches the real TextManager signature
    std::string postProcessString(const std::string& unprocessedString) const {
        return unprocessedString;
    }
    std::string getLocalized(const char* msgid) const {
        return msgid ? std::string(msgid) : std::string();
    }
};

// Global game/map state (null in tests - simulation is not run)
Game* currentGame = nullptr;
Map* currentGameMap = nullptr;

// pTextManager stub - nullptr means _() returns the msgid unchanged
std::unique_ptr<TextManager> pTextManager = nullptr;

// Stub the localization macro so CitySimulation.cpp compiles without TextManager runtime
#define _(msgid) ((msgid) ? (msgid) : "")
