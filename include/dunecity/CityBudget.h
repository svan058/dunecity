#ifndef DUNECITY_CITYBUDGET_H
#define DUNECITY_CITYBUDGET_H

#include <cstdint>

namespace DuneCity {

class CityBudget {
public:
    int32_t getLastTaxRevenue() const { return lastTaxRevenue_; }
    void setLastTaxRevenue(int32_t v) { lastTaxRevenue_ = v; }

    int32_t getRoadPercent() const { return roadPercent_; }
    void setRoadPercent(int32_t v) { roadPercent_ = v; }
    int32_t getPolicePercent() const { return policePercent_; }
    void setPolicePercent(int32_t v) { policePercent_ = v; }
    int32_t getFirePercent() const { return firePercent_; }
    void setFirePercent(int32_t v) { firePercent_ = v; }

    void init(class CitySimulation* sim) { citySim_ = sim; }
    void collectTax() { lastTaxRevenue_ = 0; }

private:
    int32_t lastTaxRevenue_ = 0;
    int32_t roadPercent_ = 100;
    int32_t policePercent_ = 100;
    int32_t firePercent_ = 100;
    class CitySimulation* citySim_ = nullptr;
};

} // namespace DuneCity

#endif // DUNECITY_CITYBUDGET_H
