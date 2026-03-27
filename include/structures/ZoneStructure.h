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

// Forward declarations
class House;
class Game;

/// A base class for city zone structures (Residential, Commercial, Industrial)
class ZoneStructure : public StructureBase {
public:
    explicit ZoneStructure(House* newOwner, DuneCity::ZoneType zoneType);
    explicit ZoneStructure(InputStream& stream);
    virtual ~ZoneStructure() override;

    void save(OutputStream& stream) const override;

    /// Enables placement on sand tiles
    bool canBePlacedAt(int x, int y, bool torch = false) const;

    DuneCity::ZoneType getZoneType() const { return zoneType_; }

    void destroy() override;

private:
    DuneCity::ZoneType zoneType_;  // The type of zone this structure represents
};

/// A residential zone structure
class ResidentialZone : public ZoneStructure {
public:
    explicit ResidentialZone(House* newOwner);
    explicit ResidentialZone(InputStream& stream);
    virtual ~ResidentialZone() override = default;
};

/// A commercial zone structure
class CommercialZone : public ZoneStructure {
public:
    explicit CommercialZone(House* newOwner);
    explicit CommercialZone(InputStream& stream);
    virtual ~CommercialZone() override = default;
};

/// An industrial zone structure
class IndustrialZone : public ZoneStructure {
public:
    explicit IndustrialZone(House* newOwner);
    explicit IndustrialZone(InputStream& stream);
    virtual ~IndustrialZone() override = default;
};

#endif // ZONESTRUCTURE_H
