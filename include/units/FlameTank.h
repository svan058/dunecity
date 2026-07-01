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

#ifndef FLAMETANK_H
#define FLAMETANK_H

#include <units/TrackedUnit.h>

/// Flame Tank (Tornie mod) — sonic-line flame weapon, Heavy Factory TechLevel 9 UpgradeLevel 4.
/// Fires Bullet_Flame: line propagation from unit like Sonic Tank, visual flame explosions on path.
/// Instant-kills light infantry, 2x damage to Troopers, no aircraft damage.
class FlameTank final : public TrackedUnit
{
public:
    explicit FlameTank(House* newOwner);
    explicit FlameTank(InputStream& stream);
    void init();
    virtual ~FlameTank();

    void destroy() override;
    void playAttackSound() override;
};

#endif // FLAMETANK_H
