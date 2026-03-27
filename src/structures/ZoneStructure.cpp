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

#include <structures/ZoneStructure.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Game.h>
#include <Map.h>
#include <Command.h>
#include <CommandManager.h>

// --- ZoneStructure base ---

ZoneStructure::ZoneStructure(House* newOwner) : StructureBase(newOwner) {
}

ZoneStructure::ZoneStructure(InputStream& stream) : StructureBase(stream) {
}

ZoneStructure::~ZoneStructure() = default;

void ZoneStructure::save(OutputStream& stream) const {
    StructureBase::save(stream);
}

void ZoneStructure::initZoneStructure() {
    structureSize.x = 2;
    structureSize.y = 2;
}

void ZoneStructure::registerZoneWithCity() {
    if(!currentGameMap) return;

    DuneCity::ZoneType zt = getZoneType();
    for(int i = 0; i < structureSize.x; i++) {
        for(int j = 0; j < structureSize.y; j++) {
            Tile* pTile = currentGameMap->getTile(location.x + i, location.y + j);
            if(pTile) {
                pTile->setCityZoneType(zt);
                pTile->setCityZoneDensity(1);
            }
        }
    }
}

void ZoneStructure::unregisterZoneFromCity() {
    if(!currentGameMap) return;

    for(int i = 0; i < structureSize.x; i++) {
        for(int j = 0; j < structureSize.y; j++) {
            Tile* pTile = currentGameMap->getTile(location.x + i, location.y + j);
            if(pTile) {
                pTile->setCityZoneType(DuneCity::ZoneType::None);
                pTile->setCityZoneDensity(0);
            }
        }
    }
}

void ZoneStructure::setLocation(int xPos, int yPos) {
    StructureBase::setLocation(xPos, yPos);

    if(xPos != INVALID_POS && yPos != INVALID_POS) {
        registerZoneWithCity();
    }
}

void ZoneStructure::destroy() {
    unregisterZoneFromCity();
    StructureBase::destroy();
}

// --- ResidentialZone ---

ResidentialZone::ResidentialZone(House* newOwner) : ZoneStructure(newOwner) {
    ResidentialZone::init();
    setHealth(getMaxHealth());
}

ResidentialZone::ResidentialZone(InputStream& stream) : ZoneStructure(stream) {
    ResidentialZone::init();
}

void ResidentialZone::init() {
    itemID = Structure_ZoneResidential;
    owner->incrementStructures(itemID);

    initZoneStructure();

    // TODO: DuneCity zone-specific sprites — using WindTrap as placeholder for Residential
    graphicID = ObjPic_Windtrap;
    graphic = pGFXManager->getObjPic(graphicID, getOwner()->getHouseID());
    numImagesX = NUM_WINDTRAP_ANIMATIONS_PER_ROW;
    numImagesY = (2 + NUM_WINDTRAP_ANIMATIONS + NUM_WINDTRAP_ANIMATIONS_PER_ROW - 1) / NUM_WINDTRAP_ANIMATIONS_PER_ROW;
    firstAnimFrame = 2;
    lastAnimFrame = 2 + NUM_WINDTRAP_ANIMATIONS - 1;
}

ResidentialZone::~ResidentialZone() = default;

// --- CommercialZone ---

CommercialZone::CommercialZone(House* newOwner) : ZoneStructure(newOwner) {
    CommercialZone::init();
    setHealth(getMaxHealth());
}

CommercialZone::CommercialZone(InputStream& stream) : ZoneStructure(stream) {
    CommercialZone::init();
}

void CommercialZone::init() {
    itemID = Structure_ZoneCommercial;
    owner->incrementStructures(itemID);

    initZoneStructure();

    // TODO: DuneCity zone-specific sprites — using Silo as placeholder for Commercial
    graphicID = ObjPic_Silo;
    graphic = pGFXManager->getObjPic(graphicID, getOwner()->getHouseID());
    numImagesX = 4;
    numImagesY = 1;
    firstAnimFrame = 2;
    lastAnimFrame = 3;
}

CommercialZone::~CommercialZone() = default;

// --- IndustrialZone ---

IndustrialZone::IndustrialZone(House* newOwner) : ZoneStructure(newOwner) {
    IndustrialZone::init();
    setHealth(getMaxHealth());
}

IndustrialZone::IndustrialZone(InputStream& stream) : ZoneStructure(stream) {
    IndustrialZone::init();
}

void IndustrialZone::init() {
    itemID = Structure_ZoneIndustrial;
    owner->incrementStructures(itemID);

    initZoneStructure();

    // TODO: DuneCity zone-specific sprites — using Radar as placeholder for Industrial
    graphicID = ObjPic_Radar;
    graphic = pGFXManager->getObjPic(graphicID, getOwner()->getHouseID());
    numImagesX = 6;
    numImagesY = 1;
    firstAnimFrame = 2;
    lastAnimFrame = 5;
}

IndustrialZone::~IndustrialZone() = default;
