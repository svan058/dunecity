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

#ifndef DISCORDMANAGER_H
#define DISCORDMANAGER_H

#include <string>
#include <cstdint>

#ifdef HAS_DISCORD_RPC

#include <thread>

class DiscordManager {
public:
    static DiscordManager& instance();

    void initialize();
    void shutdown();
    void update();
    void setMainMenu();
    void setInGame(const std::string& houseName, const std::string& mapName, bool isCampaign = false);
    void setHostingGame(const std::string& mapName, int currentPlayers, int maxPlayers);
    void setInLobby(const std::string& hostName, const std::string& mapName);
    void setMultiplayerGame(const std::string& houseName, const std::string& mapName,
                            int currentPlayers, int maxPlayers);
    void setGameStarting(const std::string& mapName, const std::string& modName,
                         const std::string& playerDetails, int playerCount);
    void setMapEditor(const std::string& mapName = "");
    void clear();
    bool isConnected() const { return connected; }
    void setWebhookUrl(const std::string& url) { webhookUrl = url; }
    void sendWebhookMessage(const std::string& title, const std::string& description,
                            int color = 0x3498db);
    void sendGameStartingNotification(const std::string& mapName, const std::string& modName,
                                      const std::string& playerDetails);

private:
    DiscordManager() = default;
    ~DiscordManager();
    DiscordManager(const DiscordManager&) = delete;
    DiscordManager& operator=(const DiscordManager&) = delete;

    bool initialized = false;
    bool connected = false;
    int64_t startTimestamp = 0;
    std::string webhookUrl;

    void updatePresence(const std::string& state, const std::string& details,
                        const std::string& largeImageKey = "logo",
                        const std::string& largeImageText = "Dune Legacy",
                        const std::string& smallImageKey = "",
                        const std::string& smallImageText = "",
                        int partySize = 0, int partyMax = 0);
};

#else // !HAS_DISCORD_RPC

class DiscordManager {
public:
    static DiscordManager& instance() { static DiscordManager dm; return dm; }

    void initialize() {}
    void shutdown() {}
    void update() {}
    void setMainMenu() {}
    void setInGame(const std::string&, const std::string&, bool = false) {}
    void setHostingGame(const std::string&, int, int) {}
    void setInLobby(const std::string&, const std::string&) {}
    void setMultiplayerGame(const std::string&, const std::string&, int, int) {}
    void setGameStarting(const std::string&, const std::string&, const std::string&, int) {}
    void setMapEditor(const std::string& = "") {}
    void clear() {}
    bool isConnected() const { return false; }
    void setWebhookUrl(const std::string&) {}
    void sendWebhookMessage(const std::string&, const std::string&, int = 0) {}
    void sendGameStartingNotification(const std::string&, const std::string&, const std::string&) {}

private:
    DiscordManager() = default;
    DiscordManager(const DiscordManager&) = delete;
    DiscordManager& operator=(const DiscordManager&) = delete;
};

#endif // HAS_DISCORD_RPC

#endif // DISCORDMANAGER_H

