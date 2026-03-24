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

#ifndef STUNCLIENT_H
#define STUNCLIENT_H

#include <enet/enet.h>
#include <string>
#include <cstdint>

/**
 * STUN (Session Traversal Utilities for NAT) client for NAT traversal.
 * 
 * Performs STUN Binding Request to discover the external IP and port
 * as seen by a STUN server. Used for NAT hole punching coordination.
 * 
 * IMPORTANT: STUN queries MUST only be run on the ENet socket when no
 * ENet peers exist (pre-connection) to avoid consuming ENet packets.
 */
class StunClient {
public:
    /**
     * Result of a STUN query
     */
    struct StunResult {
        bool success = false;
        std::string externalIP;
        uint16_t externalPort = 0;
        std::string errorMessage;
    };

    /**
     * Perform a STUN Binding Request using the provided ENet socket.
     * 
     * SAFETY: Only call this when the ENet host has no peers (peerList empty).
     * The function will receive packets on the socket, which would consume
     * ENet traffic if peers were connected.
     * 
     * @param socket    ENet socket to use (typically host->socket)
     * @param stunHost  STUN server hostname (default: Google's STUN server)
     * @param stunPort  STUN server port (default: 19302)
     * @param timeoutMs Timeout in milliseconds (default: 3000)
     * @return          StunResult with external IP:port or error
     */
    static StunResult performStunQuery(
        ENetSocket socket,
        const std::string& stunHost = "stun.l.google.com",
        uint16_t stunPort = 19302,
        int timeoutMs = 3000
    );

private:
    // STUN message types (RFC 5389)
    static constexpr uint16_t STUN_BINDING_REQUEST = 0x0001;
    static constexpr uint16_t STUN_BINDING_RESPONSE = 0x0101;
    static constexpr uint16_t STUN_BINDING_ERROR = 0x0111;

    // STUN attribute types
    static constexpr uint16_t STUN_ATTR_MAPPED_ADDRESS = 0x0001;
    static constexpr uint16_t STUN_ATTR_XOR_MAPPED_ADDRESS = 0x0020;

    // STUN magic cookie (RFC 5389)
    static constexpr uint32_t STUN_MAGIC_COOKIE = 0x2112A442;

    // STUN header size
    static constexpr size_t STUN_HEADER_SIZE = 20;

    /**
     * Build a STUN Binding Request packet
     */
    static void buildBindingRequest(uint8_t* buffer, size_t& length, uint8_t* transactionId);

    /**
     * Parse a STUN Binding Response and extract the external address
     */
    static bool parseBindingResponse(
        const uint8_t* buffer, 
        size_t length,
        const uint8_t* expectedTransactionId,
        std::string& externalIP,
        uint16_t& externalPort
    );

    /**
     * Parse XOR-MAPPED-ADDRESS attribute
     */
    static bool parseXorMappedAddress(
        const uint8_t* attrData,
        size_t attrLength,
        const uint8_t* transactionId,
        std::string& ip,
        uint16_t& port
    );

    /**
     * Parse MAPPED-ADDRESS attribute (legacy, non-XOR)
     */
    static bool parseMappedAddress(
        const uint8_t* attrData,
        size_t attrLength,
        std::string& ip,
        uint16_t& port
    );
};

#endif // STUNCLIENT_H

