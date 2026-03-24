# PathBudget Sync - Implementation Log

## State
Branch: `master`
Commit: `bf40a02` (docs + tightened grace), `d304b46` (client sends decision-time budget), `e4963b2` (initial false-desync fix)

## Current Step
Manual multiplayer validation:
- Reproduce a host budget change at an interval boundary and confirm no “false DESYNC” re-sync storms in logs.
- Confirm a real mismatch still triggers a re-sync.

## How To Validate
1. Run a 2-player multiplayer session long enough to hit at least one `kBudgetCheckInterval` boundary.
2. In logs, verify:
   - client outbound stats include `budgetAtDecisionTime`
   - host inbound stats budget matches host budget at decision cycle
   - no repeated “DESYNC DETECTED” spam after a normal budget change

## Changes Made
- `dunelegacy/src/Game.cpp`: client reports budget-at-decision-time; host DESYNC check compares budgets at consistent cycles with cycle-based allowance for “previous budget”.
- `dunelegacy/include/Game.h`: tracks `previousNegotiatedBudget` and `lastBudgetChangeCycle`.
- `dunelegacy/.analysis/features/pathbudget-sync/design.md`: protocol contract + regression scenarios.

## Tests Added / Updated
None (no existing automated harness for network/protocol simulation).

## Review Notes (Codex)
- Fix is the right shape: it resolves the “compare cycle N-1 vs N” mismatch and tightens the grace logic to a cycle-based rule.
- Follow-up if needed: add a deterministic protocol simulation harness to make these regressions automated.

## Open Issues / Follow-ups
- Consider aligning comments in code to reflect the cycle-based rule (not “375-cycle grace window”).
- Add a small non-interactive regression harness if/when tests are wired into CMake/CTest.

