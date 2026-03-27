#ifndef DUNECITY_CITYSCANNER_H
#define DUNECITY_CITYSCANNER_H

#include <dunecity/CityMapLayer.h>
#include <cstdint>

namespace DuneCity {

class CitySimulation;

/**
 * Analysis passes ported from Micropolis scan.cpp.
 *
 * Computes pollution, land value, crime, population density, fire
 * coverage, and commercial rate maps by iterating overlay grids.
 */
class CityScanner {
public:
    CityScanner();

    void init(CitySimulation* sim);

    void pollutionTerrainLandValueScan();
    void crimeScan();
    void populationDensityScan();
    void fireAnalysis();
    void computeComRateMap();

private:
    void smoothMap(CityMapByte2& src, CityMapByte2& dest);
    void smoothStationMap(CityMapShort8& map);

    int getCityCenterDistance(int x, int y) const;

    CitySimulation* sim_ = nullptr;
    int cityCenterX_ = 0;
    int cityCenterY_ = 0;
    int32_t pollutionAverage_ = 0;
    int32_t landValueAverage_ = 0;
    int32_t crimeAverage_ = 0;
};

} // namespace DuneCity

#endif // DUNECITY_CITYSCANNER_H
