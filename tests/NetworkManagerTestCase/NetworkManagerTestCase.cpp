/*
 *  NetworkManagerTestCase.cpp - Unit tests for NetworkManager functionality
 *
 *  Tests packet serialization, address handling, and protocol helpers.
 *  Does NOT test actual network I/O (that requires integration tests).
 */

#include <catch2/catch_all.hpp>

#include <Network/ENetHelper.h>
#include <Network/ENetPacketOStream.h>
#include <Network/ENetPacketIStream.h>

#include <enet/enet.h>

// Packet type constants (copied from NetworkManager.h for testing)
// These are validated to ensure they don't change accidentally
#define TEST_NETWORKPACKET_SENDGAMEINFO         1
#define TEST_NETWORKPACKET_CLIENTSTATS          13
#define TEST_NETWORKPACKET_KEEPALIVE            19
#define TEST_NETWORK_PROTOCOL_VERSION           3

// ENet initialization fixture
struct ENetFixture {
    ENetFixture() {
        if (enet_initialize() != 0) {
            throw std::runtime_error("Failed to initialize ENet");
        }
    }
    ~ENetFixture() {
        enet_deinitialize();
    }
};

// =============================================================================
// Address Utility Tests (require ENet)
// =============================================================================

TEST_CASE_METHOD(ENetFixture, "NetworkManager: Address2String for IPv4", "[network][address]") {
    ENetAddress addr;
    addr.host = 0x0100007F;  // 127.0.0.1 in little-endian
    addr.port = 12345;
    
    REQUIRE(Address2String(addr) == "127.0.0.1");
}

TEST_CASE_METHOD(ENetFixture, "NetworkManager: Address2String for localhost", "[network][address]") {
    ENetAddress addr;
    enet_address_set_host(&addr, "localhost");
    addr.port = 8080;
    
    REQUIRE(Address2String(addr) == "127.0.0.1");
}

TEST_CASE_METHOD(ENetFixture, "NetworkManager: Address2String for broadcast", "[network][address]") {
    ENetAddress addr;
    addr.host = ENET_HOST_BROADCAST;
    addr.port = 12345;
    
    REQUIRE(Address2String(addr) == "255.255.255.255");
}

// =============================================================================
// ENet Packet Stream Tests (require ENet)
// =============================================================================

TEST_CASE_METHOD(ENetFixture, "NetworkManager: Packet stream write/read uint32", "[network][packet]") {
    ENetPacketOStream ostream(ENET_PACKET_FLAG_RELIABLE);
    ostream.writeUint32(0x12345678);
    ostream.writeUint32(0xDEADBEEF);
    
    ENetPacket* packet = ostream.getPacket();
    REQUIRE(packet != nullptr);
    REQUIRE(packet->dataLength == 8);
    
    ENetPacketIStream istream(packet);
    REQUIRE(istream.readUint32() == 0x12345678);
    REQUIRE(istream.readUint32() == 0xDEADBEEF);
}

TEST_CASE_METHOD(ENetFixture, "NetworkManager: Packet stream write/read string", "[network][packet]") {
    ENetPacketOStream ostream(ENET_PACKET_FLAG_RELIABLE);
    ostream.writeString("Hello, Dune Legacy!");
    
    ENetPacket* packet = ostream.getPacket();
    REQUIRE(packet != nullptr);
    
    ENetPacketIStream istream(packet);
    REQUIRE(istream.readString() == "Hello, Dune Legacy!");
}

TEST_CASE_METHOD(ENetFixture, "NetworkManager: Packet stream complex packet", "[network][packet]") {
    // Write a complex packet similar to NETWORKPACKET_CLIENTSTATS
    ENetPacketOStream ostream(ENET_PACKET_FLAG_RELIABLE);
    ostream.writeUint32(TEST_NETWORKPACKET_CLIENTSTATS);
    ostream.writeUint32(750);
    ostream.writeFloat(60.0f);
    ostream.writeFloat(0.5f);
    ostream.writeUint32(100);
    ostream.writeUint32(15000);
    
    ENetPacket* packet = ostream.getPacket();
    REQUIRE(packet != nullptr);
    
    ENetPacketIStream istream(packet);
    REQUIRE(istream.readUint32() == TEST_NETWORKPACKET_CLIENTSTATS);
    REQUIRE(istream.readUint32() == 750);
    REQUIRE(istream.readFloat() == Catch::Approx(60.0f));
    REQUIRE(istream.readFloat() == Catch::Approx(0.5f));
    REQUIRE(istream.readUint32() == 100);
    REQUIRE(istream.readUint32() == 15000);
}

TEST_CASE_METHOD(ENetFixture, "NetworkManager: Packet stream empty string", "[network][packet]") {
    ENetPacketOStream ostream(ENET_PACKET_FLAG_RELIABLE);
    ostream.writeString("");
    ostream.writeString("after empty");
    
    ENetPacket* packet = ostream.getPacket();
    REQUIRE(packet != nullptr);
    
    ENetPacketIStream istream(packet);
    REQUIRE(istream.readString() == "");
    REQUIRE(istream.readString() == "after empty");
}

TEST_CASE_METHOD(ENetFixture, "NetworkManager: Packet stream bool values", "[network][packet]") {
    ENetPacketOStream ostream(ENET_PACKET_FLAG_RELIABLE);
    ostream.writeBool(true);
    ostream.writeBool(false);
    ostream.writeBool(true);
    
    ENetPacket* packet = ostream.getPacket();
    
    ENetPacketIStream istream(packet);
    REQUIRE(istream.readBool() == true);
    REQUIRE(istream.readBool() == false);
    REQUIRE(istream.readBool() == true);
}

TEST_CASE_METHOD(ENetFixture, "NetworkManager: Packet stream various int sizes", "[network][packet]") {
    ENetPacketOStream ostream(ENET_PACKET_FLAG_RELIABLE);
    ostream.writeUint8(0xFF);
    ostream.writeUint16(0xABCD);
    ostream.writeUint32(0x12345678);
    ostream.writeUint64(0xDEADBEEFCAFEBABE);
    
    ENetPacket* packet = ostream.getPacket();
    
    ENetPacketIStream istream(packet);
    REQUIRE(istream.readUint8() == 0xFF);
    REQUIRE(istream.readUint16() == 0xABCD);
    REQUIRE(istream.readUint32() == 0x12345678);
    REQUIRE(istream.readUint64() == 0xDEADBEEFCAFEBABE);
}
