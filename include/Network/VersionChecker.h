/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VERSIONCHECKER_H
#define VERSIONCHECKER_H

#include <misc/SDL2pp.h>
#include <string>
#include <functional>

/**
 * Version information returned from the metaserver
 */
struct VersionInfo {
    std::string latestVersion;   ///< The latest available version
    std::string downloadURL;     ///< URL to download the latest version
    bool updateAvailable;        ///< True if an update is available
};

/**
 * Checks for updates from the metaserver in a background thread.
 * Usage:
 *   1. Create instance
 *   2. Set callback via setOnVersionCheckComplete
 *   3. Call checkForUpdates()
 *   4. Call update() periodically (e.g., in game loop) to receive callback
 */
class VersionChecker {
public:
    /**
     * Create a version checker
     * \param metaServerURL The URL of the metaserver (e.g., "https://dunelegacy.com/metaserver/metaserver.php")
     */
    explicit VersionChecker(const std::string& metaServerURL);
    ~VersionChecker();

    /**
     * Start checking for updates in the background
     */
    void checkForUpdates();

    /**
     * Set callback to be called when version check completes
     * \param callback Function that receives the version info
     */
    void setOnVersionCheckComplete(std::function<void(const VersionInfo&)> callback) {
        this->onVersionCheckComplete = callback;
    }

    /**
     * Call periodically to process completed version checks and trigger callbacks
     */
    void update();

    /**
     * Check if an update check is currently in progress
     */
    bool isChecking() const { return bChecking; }

    /**
     * Compare two version strings (e.g., "0.99.1" vs "0.99")
     * \return positive if v1 > v2, negative if v1 < v2, 0 if equal
     */
    static int compareVersions(const std::string& v1, const std::string& v2);

private:
    static int checkThreadMain(void* data);

    const std::string metaServerURL;
    SDL_Thread* checkThread = nullptr;
    SDL_mutex* mutex = nullptr;

    bool bChecking = false;
    bool bCheckComplete = false;
    VersionInfo versionInfo;

    std::function<void(const VersionInfo&)> onVersionCheckComplete;
};

#endif // VERSIONCHECKER_H

