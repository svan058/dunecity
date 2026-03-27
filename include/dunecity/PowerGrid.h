#ifndef DUNECITY_POWERGRID_H
#define DUNECITY_POWERGRID_H

#include <dunecity/CityMapLayer.h>
#include <DataTypes.h>

#include <vector>
#include <cstdint>

class Map;
class Tile;

namespace DuneCity {

/**
 * Power distribution via flood-fill through conductive tiles.
 *
 * Ported from Micropolis power.cpp. Power sources (WindTraps, etc.) push
 * seed positions onto a stack. doPowerScan() walks from each seed through
 * connected conductive tiles, marking them as powered in the powerGridMap.
 */
class PowerGrid {
public:
    PowerGrid();

    void init(int mapWidth, int mapHeight);

    /**
     * Register a power source at the given tile position.
     * Called during the map scan phase when a power-producing structure is found.
     * @param strength  Number of tiles this source can power.
     */
    void addPowerSource(int x, int y, int strength);

    /**
     * Run the flood-fill power scan. Clears the grid, then propagates
     * power from all registered sources through conductive tiles.
     * @param gameMap  The game map to read tile conductivity from.
     * @param powerGridMap  The overlay to write power state into.
     */
    void doPowerScan(const Map& gameMap, CityMapByte1& powerGridMap);

    /**
     * Reset source registrations for the next scan cycle.
     * Called from clearCensus().
     */
    void clearSources();

    int getTotalCapacity() const { return totalCapacity_; }
    int getPoweredTileCount() const { return poweredTileCount_; }

private:
    bool testForConductive(const Map& gameMap, const CityMapByte1& powerGridMap,
                           int x, int y, int dx, int dy) const;

    struct PowerSource {
        int x, y;
        int strength;
    };

    struct StackEntry {
        int x, y;
    };

    int mapWidth_ = 0;
    int mapHeight_ = 0;
    int totalCapacity_ = 0;
    int poweredTileCount_ = 0;

    std::vector<PowerSource> sources_;
    std::vector<StackEntry> stack_;
};

} // namespace DuneCity

#endif // DUNECITY_POWERGRID_H
