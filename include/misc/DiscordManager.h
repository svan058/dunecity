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
#include <thread>

/**
 * Discord Rich Presence Manager
 * 
 * Shows game activity in Discord (what map you're playing, multiplayer status, etc.)
 */
class DiscordManager {
public:
    static DiscordManager& instance();
    
    // Initialize Discord RPC connection
    void initialize();
    
    // Shutdown Discord RPC
    void shutdown();
    
    // Update presence - call periodically (e.g., every few seconds)
    void update();
    
    // Set presence for main menu
    void setMainMenu();
    
    // Set presence for single player game
    void setInGame(const std::string& houseName, const std::string& mapName, bool isCampaign = false);
    
    // Set presence for multiplayer lobby (hosting)
    void setHostingGame(const std::string& mapName, int currentPlayers, int maxPlayers);
    
    // Set presence for multiplayer lobby (joining)
    void setInLobby(const std::string& hostName, const std::string& mapName);
    
    // Set presence for multiplayer game in progress
    void setMultiplayerGame(const std::string& houseName, const std::string& mapName, 
                            int currentPlayers, int maxPlayers);
    
    // Set presence for game starting (countdown) with player details
    // playerDetails format: "Atreides: Player1, Harkonnen: QuantBot, ..."
    void setGameStarting(const std::string& mapName, const std::string& modName, 
                         const std::string& playerDetails, int playerCount);
    
    // Set presence for map editor
    void setMapEditor(const std::string& mapName = "");
    
    // Clear presence
    void clear();
    
    // Check if Discord is connected
    bool isConnected() const { return connected; }
    
    // Set webhook URL for posting game notifications to a Discord channel
    void setWebhookUrl(const std::string& url) { webhookUrl = url; }
    
    // Send a webhook message to the configured Discord channel (async, non-blocking)
    void sendWebhookMessage(const std::string& title, const std::string& description, 
                            int color = 0x3498db);  // Default: blue color
    
    // Send game starting notification to Discord channel
    void sendGameStartingNotification(const std::string& mapName, const std::string& modName,
                                      const std::string& playerDetails);

private:
    DiscordManager() = default;
    ~DiscordManager();
    
    // Non-copyable
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

#endif // DISCORDMANAGER_H

