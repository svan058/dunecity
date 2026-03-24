#ifndef SPATIALGRIDHANDLE_H
#define SPATIALGRIDHANDLE_H

#include <DataTypes.h>
#include <Definitions.h>

class SpatialGrid;

struct SpatialGridKey {
    Uint32 objectId = NONE_ID;
    Uint32 generation = 0;

    bool matches(Uint32 id, Uint32 gen) const noexcept {
        return (objectId == id) && (generation == gen) && (objectId != NONE_ID);
    }
};

struct SpatialGridHandle {
    SpatialGrid* owner = nullptr;
    Coord cell = Coord::Invalid();
    SpatialGridKey key{};

    bool isValid() const noexcept {
        return owner != nullptr && key.objectId != NONE_ID && key.generation != 0 && cell.isValid();
    }

    void invalidate() noexcept {
        owner = nullptr;
        cell.invalidate();
        key.objectId = NONE_ID;
        key.generation = 0;
    }

    bool matches(Uint32 id, Uint32 generation) const noexcept {
        return isValid() && key.matches(id, generation);
    }
};

#endif // SPATIALGRIDHANDLE_H
