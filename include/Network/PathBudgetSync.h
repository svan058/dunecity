/*
 *  PathBudgetSync.h - Testable pure functions for PathBudget synchronization
 *
 *  See: .analysis/features/pathbudget-sync/design.md for protocol contract
 */

#ifndef PATHBUDGETSYNC_H
#define PATHBUDGETSYNC_H

#include <cstdint>
#include <cstddef>
#include <vector>

namespace PathBudgetSync {

/**
 * Represents a pending budget change queued for future application.
 */
struct PendingBudgetChange {
    size_t newBudget;
    uint32_t applyCycle;
};

/**
 * Determines if a client's reported budget is valid given the current state.
 * 
 * Protocol contract:
 * - Budget is valid if it matches current budget (normal case)
 * - Budget is valid if client's report is from before the last change AND matches previous budget
 * 
 * @param clientBudget      Budget reported by the client
 * @param hostBudget        Host's current negotiated budget
 * @param previousBudget    Budget before the last change (for grace period)
 * @param clientReportCycle Cycle when client's stats were captured
 * @param lastChangeCycle   Cycle when the last budget change was applied
 * @return true if budget is considered valid (no desync)
 */
inline bool isBudgetValid(
    uint32_t clientBudget,
    size_t hostBudget,
    size_t previousBudget,
    uint32_t clientReportCycle,
    uint32_t lastChangeCycle
) {
    // Case 1: Budget matches current - normal case
    if (clientBudget == hostBudget) {
        return true;
    }
    
    // Case 2: Defense-in-depth - allow previous budget if client report is from before change
    bool clientReportFromBeforeChange = (clientReportCycle < lastChangeCycle);
    bool matchesPrevious = (clientBudget == previousBudget);
    
    return clientReportFromBeforeChange && matchesPrevious;
}

/**
 * Calculates the budget to report for a given decision cycle.
 * 
 * Protocol contract:
 * - Stats are sent 1 cycle before decision (cycle N-1 for decision at N)
 * - Must report budget-at-decision-time, not current budget
 * - Check pending changes for the decision cycle
 * 
 * @param currentBudget     Current negotiated budget
 * @param pendingChanges    List of pending budget changes
 * @param currentCycle      Current game cycle (stats are sent at decisionCycle - 1)
 * @return Budget to include in stats packet (budget-at-decision-time)
 */
inline size_t getBudgetForDecisionTime(
    size_t currentBudget,
    const std::vector<PendingBudgetChange>& pendingChanges,
    uint32_t currentCycle
) {
    uint32_t decisionCycle = currentCycle + 1;
    
    for (const auto& pending : pendingChanges) {
        if (pending.applyCycle == decisionCycle) {
            return pending.newBudget;
        }
    }
    
    return currentBudget;
}

/**
 * Determines if stats should be sent this cycle.
 * 
 * Stats are sent 1 cycle BEFORE the check interval to ensure host has fresh data.
 * 
 * @param currentCycle      Current game cycle
 * @param checkInterval     Budget check interval (typically 375)
 * @return true if stats should be sent this cycle
 */
inline bool shouldSendStats(uint32_t currentCycle, uint32_t checkInterval) {
    return ((currentCycle + 1) % checkInterval) == 0;
}

/**
 * Determines if budget decision should be made this cycle.
 * 
 * @param currentCycle      Current game cycle
 * @param checkInterval     Budget check interval (typically 375)
 * @return true if budget decision should be made this cycle
 */
inline bool shouldMakeDecision(uint32_t currentCycle, uint32_t checkInterval) {
    return (currentCycle % checkInterval) == 0;
}

/**
 * Calculates the apply cycle for a new budget change.
 * 
 * Budget changes are scheduled for the NEXT interval boundary,
 * giving clients a full interval to receive the change.
 * 
 * @param currentCycle      Current game cycle
 * @param checkInterval     Budget check interval (typically 375)
 * @return Cycle at which the budget change should be applied
 */
inline uint32_t calculateApplyCycle(uint32_t currentCycle, uint32_t checkInterval) {
    return ((currentCycle / checkInterval) + 1) * checkInterval;
}

} // namespace PathBudgetSync

#endif // PATHBUDGETSYNC_H

