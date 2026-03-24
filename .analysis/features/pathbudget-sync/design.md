# PathBudget Synchronization Protocol

## Status
DONE

## Problem Statement

The PathBudget system dynamically adjusts pathfinding budget across multiplayer peers. A protocol ambiguity caused false DESYNC detections when:
1. Host broadcasts budget change with `applyCycle=N`
2. Client sends stats at cycle `N-1` with current budget (pre-change)
3. Host receives stats after applying change at cycle `N`
4. Host compares client's `budget@(N-1)` against host's `budget@N` → false DESYNC

This manifested as "re-sync storms" causing perceived network lag.

## Protocol Contract

### Budget Change Broadcast

```
Host → All Clients:
  NETWORKPACKET_SET_PATH_BUDGET {
    newBudget: uint32      // New budget value
    applyCycle: uint32     // Cycle at which to apply (always future)
  }
```

**Invariant:** `applyCycle` is always at a future interval boundary (`gameCycleCount + kBudgetCheckInterval`), giving clients 375 cycles (~7.5s at 50Hz) to receive and queue the change.

### Client Stats Report

```
Client → Host:
  NETWORKPACKET_CLIENT_STATS {
    gameCycle: uint32              // Cycle when stats were measured
    avgFps: float
    simMsAvg: float
    queueDepth: uint32
    budgetForDecisionCycle: uint32 // Budget client will have at decision time
  }
```

**Key contract:** The `budgetForDecisionCycle` field represents the budget the client **will have** at the next decision interval, NOT the budget at `gameCycle`. This is because:
- Stats are sent 1 cycle early (`gameCycle = decisionCycle - 1`)
- A pending change may apply at `decisionCycle`
- Host makes decision at `decisionCycle` and compares against this field

### Client Implementation

```cpp
// Stats sent at (decisionCycle - 1)
size_t budgetAtDecisionTime = negotiatedBudget;
for (const auto& pending : pendingBudgetChanges) {
    if (pending.applyCycle == gameCycleCount + 1) {  // Next cycle = decision cycle
        budgetAtDecisionTime = pending.newBudget;
        break;
    }
}
sendStatsToHost(..., budgetAtDecisionTime);
```

### Host DESYNC Detection

The host MUST compare budgets at the **same logical point in time**:

```cpp
// CORRECT: Compare client's decision-time budget against host's current budget
// (Host is at decision cycle when checking, client sent budget-at-decision-time)
bool budgetMatches = (clientStats.currentBudget == negotiatedBudget);

// DEFENSE-IN-DEPTH: Allow previous budget only if client's report cycle < last change cycle
bool withinGracePeriod = (clientStats.lastUpdateCycle < lastBudgetChangeCycle);
bool matchesPrevious = (clientStats.currentBudget == previousNegotiatedBudget);

bool budgetValid = budgetMatches || (withinGracePeriod && matchesPrevious);
```

**Key rule:** Never compare "client @ cycle N" to "host @ cycle N+K" where K > 0.

## Timing Diagram

```
Cycle 375: Host decides change 15000→15500, broadcasts applyCycle=750
           Client receives, queues pending change
           
Cycle 749: Client sends stats:
           - gameCycle = 749
           - budgetForDecisionCycle = 15500 (peeked from pending)
           
Cycle 750: Host applies pending change (15000→15500)
           Host runs makeHostBudgetDecision()
           Host receives client stats (gameCycle=749, budget=15500)
           Host compares: 15500 == 15500 ✓ (no DESYNC)
```

## Edge Cases

### 1. Delayed Packet Arrival

Stats for cycle 749 arriving after host is at cycle 751:
- Client sent `budgetForDecisionCycle=15500`
- Host's current budget is still 15500 (no change at 750→1125)
- Comparison: 15500 == 15500 ✓

### 2. Multiple Pending Changes

If multiple changes are pending, client uses the one for `gameCycle + 1`:
```cpp
for (const auto& pending : pendingBudgetChanges) {
    if (pending.applyCycle == gameCycleCount + 1) {
        budgetAtDecisionTime = pending.newBudget;
        break;  // First match wins
    }
}
```

### 3. Missed Budget Change

If a client genuinely missed a budget change (network error, not just timing):
- Their `budgetForDecisionCycle` won't match current OR previous
- `withinGracePeriod` check uses cycle comparison, not arbitrary window
- Real DESYNC is correctly detected and re-synced

## Grace Period Logic

The grace period is **NOT** an arbitrary time window. It's a cycle-based comparison:

```cpp
// Only allow previous budget if client's stats are from BEFORE the change
bool withinGracePeriod = (clientStats.lastUpdateCycle < lastBudgetChangeCycle);
```

This is tight and correct:
- If `clientStats.lastUpdateCycle >= lastBudgetChangeCycle`, client should have the new budget
- If `clientStats.lastUpdateCycle < lastBudgetChangeCycle`, client may legitimately have old budget

## Regression Test Scenarios

### Scenario 1: Normal Operation
```
Given: Host at cycle 750, client stats from cycle 749
And: Budget change 15000→15500 applied at cycle 750
And: Client sent budgetForDecisionCycle=15500 (peeked pending)
When: Host runs DESYNC check
Then: No DESYNC detected (15500 == 15500)
```

### Scenario 2: Delayed Packet (Defense-in-Depth)
```
Given: Host at cycle 751, client stats from cycle 749 just arrived
And: Budget change 15000→15500 applied at cycle 750
And: Client sent budgetForDecisionCycle=15000 (old implementation, bug)
When: Host runs DESYNC check
And: clientStats.lastUpdateCycle (749) < lastBudgetChangeCycle (750)
And: clientStats.currentBudget (15000) == previousNegotiatedBudget (15000)
Then: No DESYNC detected (grace period allows)
```

### Scenario 3: Real DESYNC
```
Given: Host at cycle 1125, client stats from cycle 1124
And: Budget change 15000→15500 applied at cycle 750
And: Budget change 15500→16000 applied at cycle 1125
And: Client reports budget=15000 (missed both changes)
When: Host runs DESYNC check
And: clientStats.currentBudget (15000) != negotiatedBudget (16000)
And: clientStats.currentBudget (15000) != previousNegotiatedBudget (15500)
Then: DESYNC detected, re-sync triggered
```

## Files Changed

- `src/Game.cpp`: Client sends `budgetForDecisionCycle`, host uses cycle-based grace period
- `include/Game.h`: Added `previousNegotiatedBudget`, `lastBudgetChangeCycle` tracking

## Future Improvements

1. Add explicit `reportedCycle` field to stats packet (currently inferred from `lastUpdateCycle`)
2. Add deterministic simulation harness for multiplayer protocol testing
3. Consider versioning the stats packet format for future protocol changes
