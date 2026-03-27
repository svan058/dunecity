#ifndef DUNECITY_ZONESIMULATION_H
#define DUNECITY_ZONESIMULATION_H

#include <dunecity/CityConstants.h>
#include <dunecity/CityMapLayer.h>

#include <cstdint>

class Map;
class Tile;

namespace DuneCity {

class CitySimulation;

/**
 * Zone growth and decline logic ported from Micropolis zone.cpp.
 *
 * Each zone type (Residential/Commercial/Industrial) has an evaluation
 * function that scores local conditions, combined with a global growth
 * valve. Probabilistic growth/decline adjusts zone density.
 */
class ZoneSimulation {
public:
    ZoneSimulation();

    void init(CitySimulation* sim);
    void setMap(const Map* map) { map_ = map; }

    void doZone(int x, int y);

    int getLandPollutionValue(int x, int y) const;
    void incRateOfGrowth(int x, int y, int amount);
    bool hasNearbyRoad(int x, int y) const;

    // Evaluation functions — public for testing and external callers
    int16_t evalRes(int x, int y, int trafGood) const;
    int16_t evalCom(int x, int y, int trafGood) const;
    int16_t evalInd(int x, int y, int trafGood) const;

    // Get zone counts for milestone tracking
    void getZoneCounts(int& resCount, int& comCount, int& indCount) const;

private:
    void doResidential(int x, int y, bool zonePower);
    void doCommercial(int x, int y, bool zonePower);
    void doIndustrial(int x, int y, bool zonePower);

    void doResIn(int x, int y, uint8_t pop, int landVal);
    void doResOut(int x, int y, uint8_t pop, int landVal);
    void doComIn(int x, int y, uint8_t pop, int landVal);
    void doComOut(int x, int y, uint8_t pop, int landVal);
    void doIndIn(int x, int y, uint8_t pop, int landVal);
    void doIndOut(int x, int y, uint8_t pop, int landVal);

    CitySimulation* sim_ = nullptr;
    const Map* map_ = nullptr;
};

} // namespace DuneCity

#endif // DUNECITY_ZONESIMULATION_H
