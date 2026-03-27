#ifndef DUNECITY_CITYSIMULATION_H
#define DUNECITY_CITYSIMULATION_H

#include <cstdint>
#include <dunecity/CityConstants.h>
#include <dunecity/CityMapLayer.h>
#include <dunecity/CityBudget.h>

class InputStream;
class OutputStream;

namespace DuneCity {

class CitySimulation {
public:
    CitySimulation();
    ~CitySimulation();

    void init(int width, int height);
    void load(InputStream& stream);
    void save(OutputStream& stream) const;
    void advancePhase(uint32_t gameCycleCount);

    bool isInitialized() const { return initialized_; }

    static void executeCityCommand(int playerID, int commandID,
                                   uint32_t p0, uint32_t p1, uint32_t p2);

    void registerPowerSource(int x, int y, int power);

    int getResPop() const { return resPop_; }
    int getComPop() const { return comPop_; }
    int getIndPop() const { return indPop_; }
    int getTotalPop() const { return resPop_ + comPop_ + indPop_; }

    int16_t getResValve() const { return resValve_; }
    int16_t getComValve() const { return comValve_; }
    int16_t getIndValve() const { return indValve_; }

    int32_t getTotalFunds() const { return totalFunds_; }
    int16_t getCityTax() const { return cityTax_; }
    int getCityYear() const { return cityYear_; }

    int getMapWidth() const { return mapWidth_; }
    int getMapHeight() const { return mapHeight_; }

    bool isPowerShortageJustStarted() const { return false; }
    int32_t getUnpoweredZoneCount() const { return 0; }
    int32_t getEconomicVictoryThreshold() const { return economicVictoryThreshold_; }

    const CityMapLayer<uint8_t>& getPowerGridMap() const { return powerGridMap_; }
    const CityMapLayer<uint8_t>& getTrafficDensityMap() const { return trafficDensityMap_; }
    const CityMapLayer<uint8_t>& getPollutionDensityMap() const { return pollutionDensityMap_; }
    const CityMapLayer<uint8_t>& getLandValueMap() const { return landValueMap_; }
    const CityMapLayer<uint8_t>& getCrimeRateMap() const { return crimeRateMap_; }
    const CityMapLayer<uint8_t>& getPopulationDensityMap() const { return populationDensityMap_; }

    CityBudget& getCityBudget() { return budget_; }
    const CityBudget& getCityBudget() const { return budget_; }

private:
    bool initialized_ = false;
    int mapWidth_  = 0;
    int mapHeight_ = 0;

    int resPop_ = 0;
    int comPop_ = 0;
    int indPop_ = 0;
    int16_t resValve_ = 0;
    int16_t comValve_ = 0;
    int16_t indValve_ = 0;
    int32_t totalFunds_ = 0;
    int16_t cityTax_ = 7;
    int cityYear_ = 0;
    int32_t economicVictoryThreshold_ = 500;

    CityMapLayer<uint8_t> powerGridMap_;
    CityMapLayer<uint8_t> trafficDensityMap_;
    CityMapLayer<uint8_t> pollutionDensityMap_;
    CityMapLayer<uint8_t> landValueMap_;
    CityMapLayer<uint8_t> crimeRateMap_;
    CityMapLayer<uint8_t> populationDensityMap_;

    CityBudget budget_;
};

} // namespace DuneCity

#endif // DUNECITY_CITYSIMULATION_H
