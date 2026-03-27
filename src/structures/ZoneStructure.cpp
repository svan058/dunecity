/*
 *  This file is part of Dune Legacy.
 */

#include <structures/ZoneStructure.h>

#include <globals.h>
#include <Game.h>
#include <Map.h>
#include <Tile.h>
#include <dunecity/CitySimulation.h>
#include <dunecity/ZoneSimulation.h>
#include <dunecity/CityConstants.h>

#include <ObjectBase.h>

ZoneStructure::ZoneStructure(House* newOwner, DuneCity::ZoneType zoneType)
 : StructureBase(newOwner), zoneType_(zoneType) {
    structureSize = Coord(2, 2);

    auto* citySim = currentGame->getCitySimulation();
    if (citySim) {
        Coord pos = getLocation();
        for (int dy = 0; dy < structureSize.y; dy++) {
            for (int dx = 0; dx < structureSize.x; dx++) {
                Tile* pTile = currentGameMap->getTile(pos.x + dx, pos.y + dy);
                if (pTile) {
                    pTile->setCityZoneType(zoneType_);
                    pTile->setCityZoneDensity(1);
                }
            }
        }
    }
}

ZoneStructure::ZoneStructure(InputStream& stream)
 : StructureBase(stream), zoneType_(DuneCity::ZoneType::None) {
    zoneType_ = static_cast<DuneCity::ZoneType>(stream.readUint8());
}

ZoneStructure::~ZoneStructure() = default;

void ZoneStructure::save(OutputStream& stream) const {
    StructureBase::save(stream);
    stream.writeUint8(static_cast<uint8_t>(zoneType_));
}

bool ZoneStructure::canBePlacedAt(int x, int y, bool torch) const {
    // Check bounds for all tiles in the structure
    for (int y1 = 0; y1 < structureSize.y; y1++) {
        for (int x1 = 0; x1 < structureSize.x; x1++) {
            if (!currentGameMap->tileExists(x + x1, y + y1)) {
                return false;
            }
        }
    }
    for (int y1 = 0; y1 < structureSize.y; y1++) {
        for (int x1 = 0; x1 < structureSize.x; x1++) {
            Tile* pTile = currentGameMap->getTile(x + x1, y + y1);
            if (pTile->hasANonInfantryGroundObject()) {
                return false;
            }
            auto terrain = pTile->getType();
            if (terrain != Terrain_Sand && terrain != Terrain_Rock && terrain != Terrain_Slab) {
                return false;
            }
        }
    }

    // Trigger milestone notification for first zone built
    if (currentGame && currentGame->getCitySimulation()) {
        currentGame->getCitySimulation()->onFirstZoneBuilt();
    }
    return true;
}

void ZoneStructure::destroy() {
    auto* citySim = currentGame->getCitySimulation();
    if (citySim) {
        Coord pos = getLocation();
        for (int dy = 0; dy < structureSize.y; dy++) {
            for (int dx = 0; dx < structureSize.x; dx++) {
                Tile* pTile = currentGameMap->getTile(pos.x + dx, pos.y + dy);
                if (pTile) {
                    pTile->setCityZoneType(DuneCity::ZoneType::None);
                    pTile->setCityZoneDensity(0);
                }
            }
        }
    }
    StructureBase::destroy();
}

// Subclass constructors - just pass through to base
ResidentialZone::ResidentialZone(House* newOwner)
 : ZoneStructure(newOwner, DuneCity::ZoneType::Residential) {}

ResidentialZone::ResidentialZone(InputStream& stream)
 : ZoneStructure(stream) {}

CommercialZone::CommercialZone(House* newOwner)
 : ZoneStructure(newOwner, DuneCity::ZoneType::Commercial) {}

CommercialZone::CommercialZone(InputStream& stream)
 : ZoneStructure(stream) {}

IndustrialZone::IndustrialZone(House* newOwner)
 : ZoneStructure(newOwner, DuneCity::ZoneType::Industrial) {}

IndustrialZone::IndustrialZone(InputStream& stream)
 : ZoneStructure(stream) {}
