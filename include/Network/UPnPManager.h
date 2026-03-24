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

#ifndef UPNPMANAGER_H
#define UPNPMANAGER_H

#include <string>
#include <cstdint>

/**
 * UPnP Manager - Handles automatic port forwarding via UPnP
 * 
 * This class uses miniupnpc to automatically configure router port forwarding
 * when hosting internet multiplayer games, eliminating the need for users to
 * manually configure their routers.
 */
class UPnPManager {
public:
    enum class Status {
        NotInitialized,
        Discovering,
        Available,
        Unavailable,
        PortMapped,
        Error
    };

    UPnPManager();
    ~UPnPManager();

    // Non-copyable
    UPnPManager(const UPnPManager&) = delete;
    UPnPManager& operator=(const UPnPManager&) = delete;

    /**
     * Discover UPnP devices on the network
     * @param timeoutMs Discovery timeout in milliseconds (default 2000)
     * @return true if a UPnP IGD (Internet Gateway Device) was found
     */
    bool discover(int timeoutMs = 2000);

    /**
     * Add a port mapping for the game server
     * @param internalPort The local port the game is listening on
     * @param externalPort The external port to map (usually same as internal)
     * @param protocol "UDP" or "TCP" (Dune Legacy uses UDP)
     * @param description Description for the port mapping
     * @param leaseDuration Lease duration in seconds (0 = permanent until removed)
     * @return true if mapping was successful
     */
    bool addPortMapping(uint16_t internalPort, uint16_t externalPort = 0,
                        const std::string& protocol = "UDP",
                        const std::string& description = "Dune Legacy",
                        int leaseDuration = 0);

    /**
     * Remove a previously added port mapping
     * @param externalPort The external port to unmap
     * @param protocol "UDP" or "TCP"
     * @return true if unmapping was successful
     */
    bool removePortMapping(uint16_t externalPort, const std::string& protocol = "UDP");

    /**
     * Get the external (public) IP address from the router
     * @return External IP address string, or empty if not available
     */
    std::string getExternalIPAddress();

    /**
     * Get current status
     */
    Status getStatus() const { return status; }

    /**
     * Get status as human-readable string
     */
    std::string getStatusString() const;

    /**
     * Get last error message
     */
    const std::string& getLastError() const { return lastError; }

    /**
     * Check if UPnP is available on this network
     */
    bool isAvailable() const { return status == Status::Available || status == Status::PortMapped; }

    /**
     * Check if discovery has been attempted (regardless of success)
     */
    bool wasDiscoveryAttempted() const { return status != Status::NotInitialized; }

    /**
     * Check if a port is currently mapped
     */
    bool isPortMapped() const { return status == Status::PortMapped; }

private:
    struct UPnPData;
    UPnPData* data = nullptr;
    
    Status status = Status::NotInitialized;
    std::string lastError;
    uint16_t mappedPort = 0;
    std::string mappedProtocol;
};

#endif // UPNPMANAGER_H

