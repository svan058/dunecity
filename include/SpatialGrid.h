#ifndef SPATIALGRID_H
#define SPATIALGRID_H

#include <DataTypes.h>
#include <SpatialGridHandle.h>

#include <vector>

class ObjectBase;

struct SpatialGridEntry {
    ObjectBase* object = nullptr;
    SpatialGridKey key{};
};

class SpatialGrid {
private:
    using Cell = std::vector<SpatialGridEntry>;

    std::vector<std::vector<Cell>> grid;
    int cellSize;
    int gridWidth;
    int gridHeight;
    Uint32 nextGeneration = 1;

    bool isValidCell(int x, int y) const noexcept;
    Coord toCellCoord(const Coord& tilePos) const noexcept;
    Cell& getCellRef(const Coord& cell);
    const Cell& getCellRef(const Coord& cell) const;
    void removeEntry(Cell& cell, const SpatialGridHandle& handle) noexcept;
    Uint32 acquireGeneration() noexcept;

public:
    SpatialGrid(int mapWidth, int mapHeight, int cellSize);

    bool registerObject(ObjectBase& obj, const Coord& tilePos, SpatialGridHandle& handle) noexcept;
    void unregister(SpatialGridHandle& handle) noexcept;
    bool move(ObjectBase& obj, SpatialGridHandle& handle, const Coord& oldPos, const Coord& newPos) noexcept;
    bool collectEntries(const Coord& cellCoord, std::vector<SpatialGridEntry>& out) const;
    Coord clampToCell(const Coord& tilePos) const noexcept { return toCellCoord(tilePos); }
    bool isCellCoordValid(const Coord& cellCoord) const noexcept { return isValidCell(cellCoord.x, cellCoord.y); }
    int getCellSize() const noexcept { return cellSize; }
    int getGridWidth() const noexcept { return gridWidth; }
    int getGridHeight() const noexcept { return gridHeight; }
    void clear() noexcept;

#ifdef DEBUG
    void audit() const;
#endif
};

#endif // SPATIALGRID_H
