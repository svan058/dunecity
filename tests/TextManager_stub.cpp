/*
 *  TextManager_stub.cpp - Stub for TextManager::postProcessString used in tests.
 *
 *  CitySimulation.cpp includes TextManager.h which defines getLocalized() inline.
 *  getLocalized() calls postProcessString() (defined in TextManager.cpp).
 *  Since TextManager.cpp is too heavy for the test build (requires FileManager,
 *  POFile, etc.), we provide a minimal stub here.
 */

#include <FileClasses/TextManager.h>

// Stub: returns the input unchanged (no @-prefix post-processing in test context)
const std::string& TextManager::postProcessString(const std::string& unprocessedString) const {
    static const std::string result = unprocessedString;
    return result;
}
