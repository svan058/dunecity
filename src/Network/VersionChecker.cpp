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

#include <Network/VersionChecker.h>
#include <Network/ENetHttp.h>
#include <misc/string_util.h>
#include <misc/exceptions.h>
#include <config.h>

#include <sstream>
#include <map>
#include <vector>

VersionChecker::VersionChecker(const std::string& metaServerURL)
 : metaServerURL(metaServerURL) {
    mutex = SDL_CreateMutex();
    if(mutex == nullptr) {
        THROW(std::runtime_error, "VersionChecker: Unable to create mutex");
    }
}

VersionChecker::~VersionChecker() {
    if(checkThread != nullptr) {
        SDL_WaitThread(checkThread, nullptr);
    }
    if(mutex != nullptr) {
        SDL_DestroyMutex(mutex);
    }
}

void VersionChecker::checkForUpdates() {
    if(bChecking) {
        return; // Already checking
    }

    SDL_LockMutex(mutex);
    bChecking = true;
    bCheckComplete = false;
    SDL_UnlockMutex(mutex);

    checkThread = SDL_CreateThread(checkThreadMain, "VersionCheck", (void*)this);
    if(checkThread == nullptr) {
        SDL_LockMutex(mutex);
        bChecking = false;
        SDL_UnlockMutex(mutex);
        SDL_Log("VersionChecker: Failed to create check thread");
    }
}

void VersionChecker::update() {
    if(!bChecking) {
        return;
    }

    SDL_LockMutex(mutex);
    bool complete = bCheckComplete;
    VersionInfo info = versionInfo;
    if(complete) {
        bChecking = false;
        bCheckComplete = false;
    }
    SDL_UnlockMutex(mutex);

    if(complete && onVersionCheckComplete) {
        onVersionCheckComplete(info);
    }
}

int VersionChecker::compareVersions(const std::string& v1, const std::string& v2) {
    std::vector<int> parts1, parts2;

    // Parse v1
    std::stringstream ss1(v1);
    std::string segment;
    while(std::getline(ss1, segment, '.')) {
        try {
            parts1.push_back(std::stoi(segment));
        } catch(...) {
            parts1.push_back(0);
        }
    }

    // Parse v2
    std::stringstream ss2(v2);
    while(std::getline(ss2, segment, '.')) {
        try {
            parts2.push_back(std::stoi(segment));
        } catch(...) {
            parts2.push_back(0);
        }
    }

    // Compare component by component
    size_t maxLen = std::max(parts1.size(), parts2.size());
    for(size_t i = 0; i < maxLen; ++i) {
        int p1 = (i < parts1.size()) ? parts1[i] : 0;
        int p2 = (i < parts2.size()) ? parts2[i] : 0;

        if(p1 > p2) return 1;
        if(p1 < p2) return -1;
    }

    return 0; // Equal
}

int VersionChecker::checkThreadMain(void* data) {
    VersionChecker* pChecker = static_cast<VersionChecker*>(data);

    VersionInfo info;
    info.updateAvailable = false;
    info.latestVersion = VERSION;
    info.downloadURL = "https://dunelegacy.com/#download";

    try {
        std::map<std::string, std::string> parameters;
        parameters["command"] = "version";
        parameters["gameversion"] = VERSION;

        std::string result = loadFromHttp(pChecker->metaServerURL, parameters);

        // Parse response: "OK\n<version>\t<download_url>\n"
        std::istringstream resultstream(result);
        std::string status;
        resultstream >> status;

        if(status == "OK") {
            resultstream >> std::ws; // Skip whitespace

            std::string line;
            if(std::getline(resultstream, line)) {
                std::vector<std::string> parts = splitStringToStringVector(line, "\\t");
                if(parts.size() >= 1) {
                    info.latestVersion = parts[0];
                    if(parts.size() >= 2) {
                        info.downloadURL = parts[1];
                    }

                    // Check if update is available
                    info.updateAvailable = (compareVersions(info.latestVersion, VERSION) > 0);

                    SDL_Log("VersionChecker: Current=%s, Latest=%s, UpdateAvailable=%s",
                            VERSION, info.latestVersion.c_str(),
                            info.updateAvailable ? "yes" : "no");
                }
            }
        } else {
            SDL_Log("VersionChecker: Server returned non-OK status: %s", status.c_str());
        }

    } catch(std::exception& e) {
        SDL_Log("VersionChecker: Error checking for updates: %s", e.what());
    }

    // Store result
    SDL_LockMutex(pChecker->mutex);
    pChecker->versionInfo = info;
    pChecker->bCheckComplete = true;
    SDL_UnlockMutex(pChecker->mutex);

    return 0;
}

