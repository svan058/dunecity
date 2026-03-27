#ifndef DUNECITY_CITYBUDGET_H
#define DUNECITY_CITYBUDGET_H

#include <cstdint>

namespace DuneCity {

class CityBudget {
public:
    int32_t getLastTaxRevenue() const { return lastTaxRevenue_; }
    void setLastTaxRevenue(int32_t v) { lastTaxRevenue_ = v; }

private:
    int32_t lastTaxRevenue_ = 0;
};

} // namespace DuneCity

#endif // DUNECITY_CITYBUDGET_H
