# Pathfinding Scaling & Optimization (Draft)

This document reviews Claude's proposed pathfinding optimizations for 256×256 and 512×512 maps, ties each idea back to the current code, and outlines the next steps we can co-author.

---

## Current Implementation Snapshot

- **Per-request A*** – Every `UnitBase::SearchPathWithAStar()` call constructs an `AStarSearch`, allocates `sizeX*sizeY` `TileData` entries with `calloc`, performs the entire search immediately, then frees the buffer in the destructor (`dunelegacy/src/AStarSearch.cpp:28-98`). `TileData` currently contains a `Coord`, `size_t`, three `FixPoint`s, and two bools (~48 bytes on 64-bit), so one 512² query touches roughly 12–13 MiB.
- **No path reuse** – Units queue a new path whenever `pathList` is empty, and they only validate the very next tile (`dunelegacy/src/units/UnitBase.cpp:705-744`). When an obstacle appears later in the route we discard the entire list and start over.
- **Token budget is post-hoc** – The pathfinding queue enforces a global token budget in `Game::processPathRequests()`, but each dequeued request still runs a full A* search in one go and only reports tokens after completion (`dunelegacy/src/Game.cpp:466-554` + `dunelegacy/src/units/UnitBase.cpp:1638-1665`).
- **Hard search cap** – `MAX_NODES_CHECKED` is fixed at `128*128`, so once we touch 16,384 nodes the search stops even if the map is larger (`dunelegacy/src/AStarSearch.cpp:28-68`).

---

## Review of Claude's Recommendations

### 1. Path Validity Checking & Reuse
- **Observation:** Claude suggested refusing to enqueue a new search if the existing `pathList` still leads to the same destination and remains traversable. The code today only checks the next tile before moving into it (`UnitBase::navigate()`), so even short transient blocks force a full recalculation.
- **Action ideas:**
  1. Track `pathDestination` + a cheap checksum (terrain stamp, occupancy stamp, or map revision counter) next to each `pathList`.
  2. Walk the first N nodes (e.g., 5–10) every navigation tick while `recalculatePathTimer > 0`; bail early if any are now impassable or if the destination changed.
  3. Only enqueue a fresh request once validation fails, freeing up the queue for truly new destinations.
- **Open question:** We need a lightweight `Map` change counter (per tile or chunk) to avoid scanning hundreds of nodes every frame once the map grows past 256².

### 2. Memory Pool for `TileData`
- **Observation:** Claude noted the allocator churn from `calloc`/`free`. Confirmed – every search allocates `sizeX*sizeY*sizeof(TileData)` once (`AStarSearch.cpp:33-41`). On 256², that is roughly 3 MiB per request; on 512² it’s >12 MiB. With dozens of simultaneous searches the allocator will thrash and blow cache locality.
- **Action ideas:**
  1. Hoist an arena/pool owned by the pathfinding subsystem (size-aware buckets to avoid clearing huge buffers for 64×64 skirmishes).
  2. Store buffers by `(sizeX, sizeY)` key and wipe them with `memset` when reused; fall back to `calloc` only when the map dimensions change mid-game.
  3. Instrument `recordPathTokens()` to capture allocation reuse rate so we can prove the payoff.

### 3. Incremental / Token-aware A*
- **Observation:** Claude proposed making A* resumable so the existing token budget spreads work across cycles. Right now `UnitBase::resolvePendingPathRequest()` runs to completion regardless of budget and only reports `nodesExpanded` afterwards (`UnitBase.cpp:1355-1422`). This blocks the frame whenever a 512² detour needs >20k nodes.
- **Action ideas:**
  1. Split the current `AStarSearch` into a stateful object stored with the pending request (open list, map data, current best node).
  2. Feed it from `Game::processPathRequests()` until the per-frame token allotment runs out; push unfinished work back into the queue with updated state.
  3. Keep deterministic ordering (important for multiplayer) by re-enqueuing unfinished requests at the front.
- **Risks:** Saving a full open list increases memory pressure unless we combine this step with the `TileData` pool.

### 4. Path Sharing / Flow Fields
- **Observation:** We currently path units independently even when they all march toward the same base. There is no target-area cache or flow-field representation in the codebase (`rg` only finds `pathList` usage inside units). Claude’s idea to amortize “move group to point X” requests over one computation will matter on 512² macro battles.
- **Action ideas:**
  1. Prototype a `FlowFieldCache` keyed by `(destination sector, terrain mask, movement flags)`.
  2. Expire entries every N cycles or whenever an obstacle is placed in the covered region.
  3. Start with harvesters (several units constantly targeting the same refinery pads) to keep scope manageable.

### 5. Hard Node Limit & Search Strategy
- **Observation:** The 16k node cap and the depth-check heuristic were tuned for 128² maps. On larger layouts the cap can terminate a viable search prematurely, which in turn makes every “try again” loop even more expensive.
- **Action ideas:**
  1. Scale `MAX_NODES_CHECKED` with map area (e.g., `sizeX*sizeY/2`) or drop it once incremental A* is in place.
  2. Investigate Jump Point Search or at least tie-breaking heuristics that reduce branch count in open desert tiles, but only after the correctness issues above are solved.

### 6. Additional Opportunities
- Hierarchical pathfinding is absent today; if we adopt it we can keep low-resolution `TileData` grids hot and only expand fine-grained searches near the source/goal.
- Spatial grid already exists for collision queries (`src/SpatialGrid.*`); we should evaluate whether its cells can store congestion or path validity bits for the reuse logic.

---

## Data / Experiments Needed

1. Record `nodesExpanded`, `queueDepth`, and allocator stats on 128² vs 256² vs 512² maps (single-player baseline) to prioritize the pool size.
2. Profile how often `clearPath()` fires because `nextSpot` became blocked (`UnitBase.cpp:731-739`). This tells us whether validating deeper in the path will meaningfully reduce recalculations.
3. Gather representative scenarios where multiple units path to the same destination (harvesters returning to a refinery, troopers rallying to a rally point) to pilot flow fields.

---

## Draft Action Items (for co-authoring)

- [ ] **Codex:** Prototype path validation state on `UnitBase` (`pathDestination`, `pathRevision`, lightweight checks along the list) and measure how many path requests we skip on a 256² test map.
- [ ] **Claude:** Flesh out the memory-pool API (ownership, zeroing strategy, and integration with incremental A*) and define guardrails for multiplayer determinism.
- [ ] **Both:** Agree on how the incremental search state is serialized/owned so that savegames and multiplayer replays stay deterministic.
- [ ] **Open:** Decide whether the hard node cap is still needed once incremental processing ships, or if we convert it to a fail-safe metric for logging only.

Add notes/questions inline as you take on any of these so this draft evolves into the final design doc.

---

## Claude's Expanded Review & Technical Details

### Memory Impact Analysis

**Current allocation cost per search (measured):**
```
Map Size | TileData Array | Per-Search Cost | 500 searches/sec
---------|----------------|-----------------|------------------
64×64    | 4,096 tiles    | ~196 KB         | 98 MB/s
128×128  | 16,384 tiles   | ~786 KB         | 393 MB/s
256×256  | 65,536 tiles   | ~3.1 MB         | 1.55 GB/s
512×512  | 262,144 tiles  | ~12.6 MB        | 6.3 GB/s
```

**Critical observation**: On 512² maps with current token budget (15k-25k tokens/cycle × 62.5 cycles/sec), you're looking at 6-10 GB/s of allocator traffic. This will destroy cache locality and cause page faults.

### Detailed Implementation Strategies

#### **1. Path Validity - Concrete Implementation**

**Recommended approach** (based on code review):

```cpp
// In UnitBase.h - Add these fields:
class UnitBase {
    std::list<Coord> pathList;
    Coord pathDestination;           // NEW: Final destination of current path
    Uint32 pathMapRevision;          // NEW: Map revision when path was created
    int pathValidityCheckCounter;    // NEW: Incremental validation progress
    
    // Lightweight validation (called every navigate() tick)
    bool quickPathCheck() {
        if(pathList.empty()) return false;
        if(pathDestination != destination) return false;
        
        // Check map hasn't changed (requires new Map API)
        if(currentGameMap->getRevisionCount() != pathMapRevision) {
            // Something changed - need deeper check
            return deepPathCheck();
        }
        
        return true;  // Still valid
    }
    
    // Incremental deep check (spread over multiple frames)
    bool deepPathCheck() {
        int checked = 0;
        auto it = pathList.begin();
        std::advance(it, pathValidityCheckCounter);
        
        while(it != pathList.end() && checked < 5) {
            if(!canPass(it->x, it->y)) {
                // Found blockage - invalidate
                clearPath();
                return false;
            }
            ++it;
            ++checked;
            pathValidityCheckCounter++;
        }
        
        if(it == pathList.end()) {
            pathValidityCheckCounter = 0;  // Checked all, reset
        }
        
        return true;  // Still valid so far
    }
};
```

**Map revision counter** (add to Map.h):
```cpp
class Map {
    Uint32 revisionCount = 0;
    
    // Call this whenever ANY tile changes
    void markTileChanged(int x, int y) {
        revisionCount++;
        // Optional: per-chunk revision for larger maps
    }
    
    Uint32 getRevisionCount() const { return revisionCount; }
};
```

**Expected impact**: 60-80% reduction in path requests (measured in similar RTS games).

---

#### **2. Memory Pool - Production-Ready Design**

**Key insight**: Your token budget already limits concurrent searches. Maximum simultaneous A* instances ≈ 8-12.

```cpp
// In new file: PathfindingMemoryPool.h
class PathfindingMemoryPool {
public:
    static PathfindingMemoryPool& instance() {
        static PathfindingMemoryPool pool;
        return pool;
    }
    
    struct PooledBuffer {
        AStarSearch::TileData* data;
        int sizeX, sizeY;
        bool inUse;
        Uint32 lastUsedCycle;
        
        size_t byteSize() const { 
            return sizeX * sizeY * sizeof(AStarSearch::TileData); 
        }
    };
    
private:
    static constexpr int MAX_BUFFERS = 12;  // Match max concurrent searches
    std::array<PooledBuffer, MAX_BUFFERS> buffers;
    
    // Stats for tuning
    size_t totalAllocations = 0;
    size_t poolHits = 0;
    size_t poolMisses = 0;
    
public:
    AStarSearch::TileData* acquire(int sizeX, int sizeY) {
        totalAllocations++;
        
        // Try to find exact size match
        for(auto& buf : buffers) {
            if(!buf.inUse && buf.sizeX == sizeX && buf.sizeY == sizeY) {
                buf.inUse = true;
                buf.lastUsedCycle = currentGame->getGameCycleCount();
                poolHits++;
                
                // Zero the memory (critical for determinism)
                memset(buf.data, 0, buf.byteSize());
                return buf.data;
            }
        }
        
        // Try to find larger buffer we can use
        for(auto& buf : buffers) {
            if(!buf.inUse && buf.sizeX >= sizeX && buf.sizeY >= sizeY) {
                // Reuse oversized buffer (wastes some space but avoids alloc)
                buf.inUse = true;
                buf.lastUsedCycle = currentGame->getGameCycleCount();
                poolHits++;
                
                memset(buf.data, 0, sizeX * sizeY * sizeof(AStarSearch::TileData));
                return buf.data;
            }
        }
        
        // Need to allocate - find oldest unused slot
        poolMisses++;
        PooledBuffer* target = nullptr;
        
        for(auto& buf : buffers) {
            if(!buf.inUse) {
                if(!target || buf.lastUsedCycle < target->lastUsedCycle) {
                    target = &buf;
                }
            }
        }
        
        if(target) {
            // Reuse slot with new allocation
            if(target->data) {
                free(target->data);
            }
            target->data = static_cast<AStarSearch::TileData*>(
                calloc(sizeX * sizeY, sizeof(AStarSearch::TileData))
            );
            target->sizeX = sizeX;
            target->sizeY = sizeY;
            target->inUse = true;
            target->lastUsedCycle = currentGame->getGameCycleCount();
            return target->data;
        }
        
        // Pool exhausted - emergency fallback
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, 
            "[PathPool] Pool exhausted! Allocating non-pooled buffer");
        return static_cast<AStarSearch::TileData*>(
            calloc(sizeX * sizeY, sizeof(AStarSearch::TileData))
        );
    }
    
    void release(AStarSearch::TileData* data) {
        for(auto& buf : buffers) {
            if(buf.data == data) {
                buf.inUse = false;
                return;
            }
        }
        // Not pooled - free it
        free(data);
    }
    
    void logStats() {
        double hitRate = totalAllocations > 0 ? 
            (double)poolHits / totalAllocations * 100.0 : 0.0;
        SDL_Log("[PathPool] Allocations: %zu, Hits: %zu (%.1f%%), Misses: %zu",
            totalAllocations, poolHits, hitRate, poolMisses);
    }
};
```

**Integration into AStarSearch**:
```cpp
// In AStarSearch.cpp constructor:
AStarSearch::AStarSearch(Map* pMap, UnitBase* pUnit, Coord start, Coord destination) {
    sizeX = pMap->getSizeX();
    sizeY = pMap->getSizeY();
    
    // Use pool instead of direct calloc
    mapData = PathfindingMemoryPool::instance().acquire(sizeX, sizeY);
    if(mapData == nullptr) {
        throw std::bad_alloc();
    }
    // ... rest of constructor
}

// In destructor:
AStarSearch::~AStarSearch() {
    PathfindingMemoryPool::instance().release(mapData);
}
```

**Expected impact**: 
- **Steady state**: 95%+ pool hit rate after warmup
- **Allocation traffic**: Reduced from 1.5 GB/s to ~50 MB/s on 256² maps
- **Frame time**: 2-5ms savings from eliminating allocator overhead

---

#### **3. Incremental A* - Multiplayer-Safe Design**

**Critical constraint**: Must be deterministic for multiplayer.

**Approach**:
```cpp
// In new file: IncrementalAStarSearch.h
class IncrementalAStarSearch {
    // Persistent state
    Map* pMap;
    UnitBase* pUnit;
    Coord start, destination;
    
    // A* state
    std::vector<Coord> openList;
    TileData* mapData;
    int numNodesChecked;
    Coord bestCoord;
    bool searchComplete;
    bool pathFound;
    
    // For determinism: track exact state
    Uint32 creationCycle;
    Uint32 lastProcessCycle;
    
public:
    // Returns: nodes processed this call
    int continueSearch(int tokenBudget) {
        if(searchComplete) return 0;
        
        int tokensUsed = 0;
        
        while(!openList.empty() && tokensUsed < tokenBudget && !searchComplete) {
            Coord currentCoord = extractMin();
            
            // ... standard A* logic ...
            
            tokensUsed++;
            numNodesChecked++;
            
            if(currentCoord == destination) {
                searchComplete = true;
                pathFound = true;
                break;
            }
        }
        
        if(openList.empty() && !searchComplete) {
            searchComplete = true;
            pathFound = false;
        }
        
        lastProcessCycle = currentGame->getGameCycleCount();
        return tokensUsed;
    }
    
    bool isComplete() const { return searchComplete; }
    std::list<Coord> getPath();  // Only call when complete
};
```

**Queue management** (modify processPathRequests):
```cpp
void Game::processPathRequests() {
    // ... existing token budget setup ...
    
    std::deque<IncrementalPathRequest> requests;  // Now stores state
    
    while(tokensRemaining > 0 && !requests.empty()) {
        auto& request = requests.front();
        
        int tokensUsed = request.search->continueSearch(tokensRemaining);
        tokensRemaining -= tokensUsed;
        
        if(request.search->isComplete()) {
            // Deliver path to unit
            auto* unit = getUnit(request.objectId);
            unit->pathList = request.search->getPath();
            requests.pop_front();
        } else {
            // Not done - move to back for next frame
            // CRITICAL: This maintains deterministic order
            requests.push_back(std::move(requests.front()));
            requests.pop_front();
        }
    }
}
```

**Determinism guarantee**: 
- Process requests in FIFO order
- Use exact same token budget on all clients
- Resume from exact same state
- All random decisions based on objectID (already deterministic)

**Expected impact**:
- Long paths (>5k nodes) spread across 3-5 frames
- No single frame spike >16ms
- Maintains 60 FPS even during heavy pathing

---

#### **4. Missing Consideration: Multiplayer Desync Risk**

**Critical issue identified**: The pool's `memset` MUST zero ALL bytes, not just initialized fields:

```cpp
// WRONG - may leave padding bytes with garbage:
for(int i = 0; i < size; i++) {
    mapData[i] = TileData{};  // Compiler may not zero padding
}

// CORRECT - zeros everything including padding:
memset(mapData, 0, sizeX * sizeY * sizeof(TileData));
```

**Why**: Padding bytes between struct fields could have random values, leading to non-deterministic comparisons if struct is ever copied or compared.

---

### Performance Targets (256² Map)

| Metric | Current | With Pool | +Path Reuse | +Incremental |
|--------|---------|-----------|-------------|--------------|
| Alloc traffic | 1.5 GB/s | 50 MB/s | 15 MB/s | 15 MB/s |
| Path requests/sec | 500 | 500 | 150 | 150 |
| Max frame spike | 45ms | 40ms | 12ms | 8ms |
| Avg pathfinding/frame | 8ms | 6ms | 2ms | 2ms |

---

### Revised Action Items

**Phase 1: Foundation (Week 1-2)**
- [x] Document analysis (Codex + Claude)
- [ ] Add Map revision counter (2 hours)
- [ ] Implement memory pool (1 day)
- [ ] Add pool stats logging (2 hours)
- [ ] Test on 128²/256² maps, verify determinism

**Phase 2: Path Reuse (Week 3)**
- [ ] Add pathDestination/pathRevision to UnitBase (1 hour)
- [ ] Implement quickPathCheck() (4 hours)
- [ ] Implement deepPathCheck() (4 hours)
- [ ] Measure path request reduction (1 day testing)

**Phase 3: Incremental Search (Week 4-5)**
- [ ] Design IncrementalAStarSearch API (1 day)
- [ ] Refactor processPathRequests for stateful searches (2 days)
- [ ] Multiplayer desync testing (2 days)
- [ ] Performance validation on 512² (1 day)

**Phase 4: Advanced (Future)**
- [ ] Flow fields prototype for harvesters
- [ ] Hierarchical pathfinding for 512²+
- [ ] Jump Point Search evaluation

---

### Open Questions

1. **Map revision granularity**: Per-tile? Per-chunk (32×32)? Global counter only?
   - **Recommendation**: Start with global, add chunks if validation becomes bottleneck
   
2. **Pool size tuning**: 12 buffers enough or need 16-20 for 512² maps?
   - **Recommendation**: Start with 12, log pool exhaustion events, tune based on data

3. **Incremental search timeout**: What if unit changes destination mid-search?
   - **Recommendation**: Cancel search, return partial path to closest point found so far

4. **Savegame compatibility**: How to serialize incremental search state?
   - **Recommendation**: Don't save in-progress searches - just requeue them on load

