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

#ifndef ZONESTRUCTURE_H
#define ZONESTRUCTURE_H

#include <structures/StructureBase.h>
#include <dunecity/CityConstants.h>

/**
 * Base class for DuneCity zone structures (Residential, Commercial, Industrial).
 * These are 2x2 structures that can be placed on sand (unlike normal structures)
 * and register their zone type with the city simulation.
 */
class ZoneStructure : public StructureBase {
public:
    explicit ZoneStructure(House* newOwner);
    explicit ZoneStructure(InputStream& stream);
    virtual ~ZoneStructure();

    void save(OutputStream& stream) const override;

    void destroy() override;
    void setLocation(int xPos, int yPos) override;

    virtual DuneCity::ZoneType getZoneType() const = 0;

    bool canBeCaptured() const override { return false; }

protected:
    void initZoneStructure();
    void registerZoneWithCity();
    void unregisterZoneFromCity();
};

class ResidentialZone final : public ZoneStructure {
public:
    explicit ResidentialZone(House* newOwner);
    explicit ResidentialZone(InputStream& stream);
    void init();
    virtual ~ResidentialZone();

    DuneCity::ZoneType getZoneType() const override { return DuneCity::ZoneType::Residential; }
};

class CommercialZone final : public ZoneStructure {
public:
    explicit CommercialZone(House* newOwner);
    explicit CommercialZone(InputStream& stream);
    void init();
    virtual ~CommercialZone();

    DuneCity::ZoneType getZoneType() const override { return DuneCity::ZoneType::Commercial; }
};

class IndustrialZone final : public ZoneStructure {
public:
    explicit IndustrialZone(House* newOwner);
    explicit IndustrialZone(InputStream& stream);
    void init();
    virtual ~IndustrialZone();

    DuneCity::ZoneType getZoneType() const override { return DuneCity::ZoneType::Industrial; }
};

#endif // ZONESTRUCTURE_H
