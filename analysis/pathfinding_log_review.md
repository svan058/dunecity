# 128×128 Pathfinding Regression – Log Review

Source logs: `analysis/pc/Dune Legacy-Performance.log` and `analysis/pc/Dune Legacy.log` captured during a 128×128 skirmish after enabling the new path validation + memory pool changes.

---

## 1. Observed Performance

- **Pathfinding dominates the frame:** The latest performance dump shows pathfinding averaging **55.75 ms** per frame (min 53.42 ms, max 58.53 ms) with overall FPS collapsing to ~9–10 (`analysis/pc/Dune Legacy-Performance.log:7300-7313`). The queue grew beyond 720 requests despite the guardrail forcing only 10 cycles/frame (`analysis/pc/Dune Legacy-Performance.log:7305-7328`).
- **Token budget saturated:** Even after forcing the deterministic budget down to 8 k tokens/cycle, the logs report **11.5 k tokens consumed per cycle (145% utilization)** and 375 “budget exhausted” events (`analysis/pc/Dune Legacy-Performance.log:7305-7309`), signalling that each dequeued A* search expands 5–6 k nodes on average (`analysis/pc/Dune Legacy-Performance.log:7301-7304`).
- **High-cost searches dominate:** 374 of the sampled paths land in the 8k–16k token histogram bucket (`analysis/pc/Dune Legacy-Performance.log:7304`), suggesting large detours even on a 128×128 layout.
- **Guardrail is constantly tripping:** Repeated `[GUARDRAIL]` entries show the engine pegged at the 10-cycles-per-frame limit (e.g., cycles 49272 and 49472 in `analysis/pc/Dune Legacy-Performance.log:7327-7349`), confirming that token overruns translate directly into frame stalls.
- **Budget controller stuck at minimum:** The gameplay log shows the negotiated budget ramping from 15 k → 25 k while FPS stayed high, then repeatedly chopping down once the battle begins. Eventually it bottoms out at 8 k, after which every attempt to reduce further logs “Already at minimum” while guardrail crashes continue (`analysis/pc/Dune Legacy.log:1862-1908`).
- **Earlier phase looked fine:** Before combat heats up the same session reports 0.02 ms/cycle pathfinding, 0.13 paths per cycle, and a queue depth ≤3 (`analysis/pc/Dune Legacy-Performance.log:1568-1607`). The regression therefore only manifests once lots of ground units are moving simultaneously.

## 2. Diagnostics & Hypotheses

1. **Revision counter invalidates every cached path.** We now bump the global `pathingRevision` every time a tile assigns/unassigns a non-infantry ground object. Because units enter and leave tiles every ~15 frames, the revision never stays stable long enough to reuse any path. This lines up with the observed workload spike: every unit now recalculates from scratch whenever *any* unit moves, so the “reuse” optimization backfires.
2. **Instrumentation is missing.** There are zero `[PathInstrumentation]` lines in either log. That means the telemetry we added never fires, depriving us of actual reuse hit/miss rates and pooled-allocation stats. Either the 30 s timer never elapses (unlikely—these logs cover ~8 minutes) or the logger never runs because `logPathInstrumentationIfNeeded()` is gated behind `logPerformance`, which might write to a different file. Without this data we cannot quantify reuse efficacy.
3. **Token accounting exceeds budget because budgets are applied after-the-fact.** Even at the minimum 8 k tokens/cycle, each dequeued search often consumes ≥6 k nodes, so processing just two requests blows the budget. The loop continues until the queue is empty, so actual usage always exceeds negotiated totals. This explains the 148% utilization line and why reducing the budget fails to fix frame times.
4. **Queue starvation causes the guardrail to trip.** The guardrail warnings coincide with queue depths >700; since we only process up to the (blown) budget each cycle, the backlog never clears. Carry-over tokens stay at zero, so even temporarily low-load frames can’t recover.

## 3. Recommendations

1. **Decouple map-revision invalidation from every unit move.**
   - Track revisions at a coarser granularity (e.g., sector maps or “structure placement only” events) or only bump when a tile’s passability state changes (mountain, structure, destroyed structure, worm). Units moving through tiles should not invalidate global revisions.
2. **Revisit validation strategy.**
   - Instead of invalidating everything when the revision changes, combine the “first N nodes canPass” probe with a per-path checksum (e.g., hash of tile coordinates). That lets us detect local obstructions without requiring a global revision change.
3. **Make the instrumentation visible.**
   - Ensure `[PathInstrumentation]` logs are emitted to the gameplay log (or another known sink) every 30 seconds regardless of queue state so we can confirm reuse hit rates and pool behavior.
4. **Throttle actual per-cycle work.**
   - Honor the negotiated budget by pausing after spending the allotted tokens and re-queueing unfinished searches. Without incremental A*, lowering the budget can’t affect runtime because each call still runs to completion.
5. **Collect before/after data once the above are addressed.**
   - Focus on: queue depth distribution, reuse hit/miss rates, guardrail trips, and path tokens per cycle. These metrics will confirm whether path validation provides net gains on 128×128 maps.

Add follow-up logs to this folder once another test run is available so we can track progress without re-running the match.
