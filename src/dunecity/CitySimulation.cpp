#include <dunecity/CitySimulation.h>
#include <dunecity/CityConstants.h>

#include <globals.h>
#include <Game.h>
#include <Map.h>
#include <Tile.h>
#include <Command.h>

#include <SDL2/SDL_log.h>

namespace DuneCity {

CitySimulation::CitySimulation() = default;
CitySimulation::~CitySimulation() = default;

void CitySimulation::init(int width, int height) {
    mapWidth_  = width;
    mapHeight_ = height;

    powerGridMap_.init(width, height, 2);
    trafficDensityMap_.init(width, height, 2);
    pollutionDensityMap_.init(width, height, 2);
    landValueMap_.init(width, height, 2);
    crimeRateMap_.init(width, height, 2);
    populationDensityMap_.init(width, height, 2);

    initialized_ = true;
}

void CitySimulation::load(InputStream& /*stream*/) {
    // Stub — full deserialization not yet implemented
}

void CitySimulation::save(OutputStream& /*stream*/) const {
    // Stub — full serialization not yet implemented
}

void CitySimulation::advancePhase(uint32_t /*gameCycleCount*/) {
    // Stub — simulation tick not yet implemented
}

void CitySimulation::registerPowerSource(int /*x*/, int /*y*/, int /*power*/) {
    // Stub — power grid registration not yet implemented
}

void CitySimulation::executeCityCommand(int /*playerID*/, int commandID,
                                        uint32_t p0, uint32_t p1, uint32_t p2) {
    if(!currentGameMap) return;

    switch(commandID) {
        case CMD_CITY_TOOL: {
            int x = static_cast<int>(p0);
            int y = static_cast<int>(p1);
            uint32_t toolType = p2;

            if(!currentGameMap->tileExists(x, y)) return;

            Tile* tile = currentGameMap->getTile(x, y);

            switch(toolType) {
                case CityTool_Road: {
                    auto placementState = makeCityTilePlacementState(
                        tile->isRock(),
                        tile->isMountain(),
                        tile->hasAGroundObject(),
                        tile->hasCityZone(),
                        tile->isCityConductive());

                    if (!applyRoadPlacement(placementState)) {
                        return;
                    }

                    tile->setCityConductive(placementState.isCityConductive);
                    SDL_Log("CityTool: Road placed at (%d, %d)", x, y);
                } break;

                case CityTool_Bulldoze:
                case CityTool_PowerLine:
                default:
                    break;
            }
        } break;

        case CMD_CITY_PLACE_ZONE: {
            int x = static_cast<int>(p0);
            int y = static_cast<int>(p1);
            auto zoneType = static_cast<ZoneType>(p2);

            if(!currentGameMap->tileExists(x, y)) return;

            Tile* tile = currentGameMap->getTile(x, y);
            tile->setCityZoneType(zoneType);
            tile->setCityZoneDensity(1);
        } break;

        case CMD_CITY_SET_TAX_RATE:
        case CMD_CITY_SET_BUDGET:
        default:
            break;
    }
}

} // namespace DuneCity
