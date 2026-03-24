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

#include <Network/StunClient.h>
#include <SDL2/SDL.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iomanip>

// Network byte order helpers
static inline uint16_t readUint16BE(const uint8_t* data) {
    return (static_cast<uint16_t>(data[0]) << 8) | data[1];
}

static inline uint32_t readUint32BE(const uint8_t* data) {
    return (static_cast<uint32_t>(data[0]) << 24) |
           (static_cast<uint32_t>(data[1]) << 16) |
           (static_cast<uint32_t>(data[2]) << 8) |
           data[3];
}

static inline void writeUint16BE(uint8_t* data, uint16_t value) {
    data[0] = static_cast<uint8_t>(value >> 8);
    data[1] = static_cast<uint8_t>(value & 0xFF);
}

static inline void writeUint32BE(uint8_t* data, uint32_t value) {
    data[0] = static_cast<uint8_t>(value >> 24);
    data[1] = static_cast<uint8_t>((value >> 16) & 0xFF);
    data[2] = static_cast<uint8_t>((value >> 8) & 0xFF);
    data[3] = static_cast<uint8_t>(value & 0xFF);
}

StunClient::StunResult StunClient::performStunQuery(
    ENetSocket socket,
    const std::string& stunHost,
    uint16_t stunPort,
    int timeoutMs
) {
    StunResult result;
    
    if (socket == ENET_SOCKET_NULL) {
        result.errorMessage = "Invalid socket";
        return result;
    }

    SDL_Log("StunClient: Querying STUN server %s:%d", stunHost.c_str(), stunPort);

    // Resolve STUN server address
    ENetAddress stunAddress;
    if (enet_address_set_host(&stunAddress, stunHost.c_str()) < 0) {
        result.errorMessage = "Failed to resolve STUN server: " + stunHost;
        SDL_Log("StunClient: %s", result.errorMessage.c_str());
        return result;
    }
    stunAddress.port = stunPort;

    // Build STUN Binding Request
    uint8_t requestBuffer[STUN_HEADER_SIZE];
    size_t requestLength = 0;
    uint8_t transactionId[12];
    buildBindingRequest(requestBuffer, requestLength, transactionId);

    // Send the request using enet_socket_send (works on ENet socket)
    ENetBuffer sendBuffer;
    sendBuffer.data = requestBuffer;
    sendBuffer.dataLength = requestLength;

    int sentBytes = enet_socket_send(socket, &stunAddress, &sendBuffer, 1);
    if (sentBytes < 0) {
        result.errorMessage = "Failed to send STUN request";
        SDL_Log("StunClient: %s", result.errorMessage.c_str());
        return result;
    }

    SDL_Log("StunClient: Sent %d bytes to STUN server", sentBytes);

    // Wait for response with timeout
    // We use polling since enet_socket_wait may not work correctly on all platforms
    Uint32 startTime = SDL_GetTicks();
    uint8_t responseBuffer[1024];

    while (static_cast<int>(SDL_GetTicks() - startTime) < timeoutMs) {
        ENetBuffer recvBuffer;
        recvBuffer.data = responseBuffer;
        recvBuffer.dataLength = sizeof(responseBuffer);

        ENetAddress fromAddress;
        int recvBytes = enet_socket_receive(socket, &fromAddress, &recvBuffer, 1);

        if (recvBytes > 0) {
            SDL_Log("StunClient: Received %d bytes", recvBytes);

            // Validate it's from the STUN server
            if (fromAddress.host != stunAddress.host || fromAddress.port != stunAddress.port) {
                // Not from STUN server - might be game traffic, but since we're
                // pre-connection this shouldn't happen. Log and continue waiting.
                SDL_Log("StunClient: Received packet from unexpected source, ignoring");
                continue;
            }

            // Parse the response
            if (parseBindingResponse(responseBuffer, static_cast<size_t>(recvBytes), 
                                     transactionId, result.externalIP, result.externalPort)) {
                result.success = true;
                SDL_Log("StunClient: Discovered external address %s:%d", 
                        result.externalIP.c_str(), result.externalPort);
                return result;
            } else {
                SDL_Log("StunClient: Failed to parse STUN response");
                // Continue waiting in case we get a valid response
            }
        } else if (recvBytes < 0) {
            // Error (not just "no data")
            result.errorMessage = "Socket receive error";
            SDL_Log("StunClient: %s", result.errorMessage.c_str());
            return result;
        }

        // Small delay to avoid busy-waiting
        SDL_Delay(10);
    }

    result.errorMessage = "STUN request timed out";
    SDL_Log("StunClient: %s", result.errorMessage.c_str());
    return result;
}

void StunClient::buildBindingRequest(uint8_t* buffer, size_t& length, uint8_t* transactionId) {
    // Generate random transaction ID (12 bytes)
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned int>(time(nullptr) ^ SDL_GetTicks()));
        seeded = true;
    }
    
    for (int i = 0; i < 12; i++) {
        transactionId[i] = static_cast<uint8_t>(rand() & 0xFF);
    }

    // STUN header (20 bytes):
    // - Message Type: 2 bytes (Binding Request = 0x0001)
    // - Message Length: 2 bytes (0 for basic request)
    // - Magic Cookie: 4 bytes (0x2112A442)
    // - Transaction ID: 12 bytes
    
    writeUint16BE(buffer, STUN_BINDING_REQUEST);     // Message Type
    writeUint16BE(buffer + 2, 0);                     // Message Length (no attributes)
    writeUint32BE(buffer + 4, STUN_MAGIC_COOKIE);    // Magic Cookie
    memcpy(buffer + 8, transactionId, 12);           // Transaction ID

    length = STUN_HEADER_SIZE;
}

bool StunClient::parseBindingResponse(
    const uint8_t* buffer,
    size_t length,
    const uint8_t* expectedTransactionId,
    std::string& externalIP,
    uint16_t& externalPort
) {
    // Minimum header size check
    if (length < STUN_HEADER_SIZE) {
        SDL_Log("StunClient: Response too short (%zu bytes)", length);
        return false;
    }

    // Parse header
    uint16_t msgType = readUint16BE(buffer);
    uint16_t msgLength = readUint16BE(buffer + 2);
    uint32_t magicCookie = readUint32BE(buffer + 4);

    // Validate message type
    if (msgType != STUN_BINDING_RESPONSE) {
        if (msgType == STUN_BINDING_ERROR) {
            SDL_Log("StunClient: Received STUN error response");
        } else {
            SDL_Log("StunClient: Unexpected message type: 0x%04X", msgType);
        }
        return false;
    }

    // Validate magic cookie
    if (magicCookie != STUN_MAGIC_COOKIE) {
        SDL_Log("StunClient: Invalid magic cookie");
        return false;
    }

    // Validate transaction ID
    if (memcmp(buffer + 8, expectedTransactionId, 12) != 0) {
        SDL_Log("StunClient: Transaction ID mismatch");
        return false;
    }

    // Validate length
    if (STUN_HEADER_SIZE + msgLength > length) {
        SDL_Log("StunClient: Message length exceeds buffer");
        return false;
    }

    // Parse attributes
    size_t offset = STUN_HEADER_SIZE;
    const uint8_t* end = buffer + STUN_HEADER_SIZE + msgLength;

    while (buffer + offset + 4 <= end) {
        uint16_t attrType = readUint16BE(buffer + offset);
        uint16_t attrLength = readUint16BE(buffer + offset + 2);
        offset += 4;

        if (buffer + offset + attrLength > end) {
            SDL_Log("StunClient: Attribute length exceeds message");
            break;
        }

        const uint8_t* attrData = buffer + offset;

        // Prefer XOR-MAPPED-ADDRESS (more reliable with NAT)
        if (attrType == STUN_ATTR_XOR_MAPPED_ADDRESS) {
            if (parseXorMappedAddress(attrData, attrLength, expectedTransactionId, externalIP, externalPort)) {
                return true;
            }
        }
        // Fall back to MAPPED-ADDRESS
        else if (attrType == STUN_ATTR_MAPPED_ADDRESS && externalIP.empty()) {
            if (parseMappedAddress(attrData, attrLength, externalIP, externalPort)) {
                // Don't return yet - keep looking for XOR-MAPPED-ADDRESS
            }
        }

        // Move to next attribute (attributes are padded to 4-byte boundary)
        offset += attrLength;
        while (offset % 4 != 0 && buffer + offset < end) {
            offset++;
        }
    }

    // If we found a MAPPED-ADDRESS but no XOR-MAPPED-ADDRESS, use it
    return !externalIP.empty();
}

bool StunClient::parseXorMappedAddress(
    const uint8_t* attrData,
    size_t attrLength,
    const uint8_t* transactionId,
    std::string& ip,
    uint16_t& port
) {
    // Minimum length: 1 (reserved) + 1 (family) + 2 (port) + 4 (IPv4) = 8
    if (attrLength < 8) {
        return false;
    }

    uint8_t family = attrData[1];
    
    // Only support IPv4 for now
    if (family != 0x01) {
        SDL_Log("StunClient: Only IPv4 supported (got family 0x%02X)", family);
        return false;
    }

    // XOR the port with the top 16 bits of the magic cookie
    uint16_t xorPort = readUint16BE(attrData + 2);
    port = xorPort ^ static_cast<uint16_t>(STUN_MAGIC_COOKIE >> 16);

    // XOR the IP with the magic cookie
    uint32_t xorIP = readUint32BE(attrData + 4);
    uint32_t ipAddr = xorIP ^ STUN_MAGIC_COOKIE;

    // Convert to dotted decimal string
    std::ostringstream oss;
    oss << ((ipAddr >> 24) & 0xFF) << "."
        << ((ipAddr >> 16) & 0xFF) << "."
        << ((ipAddr >> 8) & 0xFF) << "."
        << (ipAddr & 0xFF);
    ip = oss.str();

    return true;
}

bool StunClient::parseMappedAddress(
    const uint8_t* attrData,
    size_t attrLength,
    std::string& ip,
    uint16_t& port
) {
    // Minimum length: 1 (reserved) + 1 (family) + 2 (port) + 4 (IPv4) = 8
    if (attrLength < 8) {
        return false;
    }

    uint8_t family = attrData[1];
    
    // Only support IPv4 for now
    if (family != 0x01) {
        SDL_Log("StunClient: Only IPv4 supported (got family 0x%02X)", family);
        return false;
    }

    port = readUint16BE(attrData + 2);
    uint32_t ipAddr = readUint32BE(attrData + 4);

    // Convert to dotted decimal string
    std::ostringstream oss;
    oss << ((ipAddr >> 24) & 0xFF) << "."
        << ((ipAddr >> 16) & 0xFF) << "."
        << ((ipAddr >> 8) & 0xFF) << "."
        << (ipAddr & 0xFF);
    ip = oss.str();

    return true;
}

