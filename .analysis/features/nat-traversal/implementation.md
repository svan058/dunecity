# NAT Traversal via STUN + Hole Punching - Implementation Log

## State
Branch: `master` (not started)
Commit: N/A

## Current Step
**Ready for deployment and testing:**

1. ✅ Metaserver protocol changes complete
2. ✅ STUN client implemented and integrated
3. ✅ MetaServerClient extended for list2, session_id, and punch endpoints
4. ✅ Client-side hole punch flow in `MultiPlayerMenu::onJoin()`
5. ✅ Host-side punch polling in `NetworkManager::update()`
6. ✅ Host-side punching refactored to non-blocking state machine

Build verified: `cmake --build build -j8` succeeds

**Next:** 
1. Deploy dunelegacy.com (metaserver) first
2. Manual testing between two internet-connected hosts

## How To Validate
Design phase - no code to validate yet.

After implementation begins:
```bash
# Build (no new dependencies - STUN client is custom)
cd dunelegacy/build && cmake --build . -j8

# Test STUN client
./bin/dunelegacy.app/Contents/MacOS/dunelegacy --test-stun

# Run and check hole punch logs
./bin/dunelegacy.app/Contents/MacOS/dunelegacy
# Create Internet Game, check logs for STUN/punch messages
tail -f "~/Library/Application Support/Dune Legacy/Dune Legacy.log" | grep -iE "stun|punch"
```

## Changes Made
- `dunelegacy.com/metaserver/metaserver.php`:
  - Added `PUNCH_FILE`, `PUNCH_REQUEST_TTL`, `PUNCH_READY_TTL`, `MAX_PUNCH_REQUESTS_PER_SESSION`, `PUNCH_RATE_LIMIT_PER_IP` constants
  - Extended `handleAdd()` to accept `stun_port`, return `session_id` (preserved on re-add)
  - Added `handleList2()` - 14 tab-separated fields with `session_id` and `stun_port`
  - Added `handlePunchRequest()` - client requests punch coordination
  - Added `handlePunchPoll()` - host polls for pending requests
  - Added `handlePunchReady()` - host signals ready to punch
  - Added `handlePunchStatus()` - client polls for readiness
  - Added punch data storage functions with TTL cleanup
  - Added rate limiting for punch requests (10/min per IP)
  - Updated `handleRemove()` to clean up punch data

- `include/Network/StunClient.h`, `src/Network/StunClient.cpp`:
  - New STUN client implementation (no external library)
  - Performs STUN Binding Request/Response on ENet socket
  - Parses XOR-MAPPED-ADDRESS and MAPPED-ADDRESS attributes
  - Returns external IP and port

- `include/Network/GameServerInfo.h`:
  - Added `sessionId`, `stunPort`, `holePunchAvailable` fields

- `include/Network/MetaServerCommands.h`:
  - Extended `MetaServerAdd` with `stunPort` parameter

- `include/Network/MetaServerClient.h`:
  - Extended `startAnnounce()` with `stunPort` parameter
  - Added `getSessionId()` accessor
  - Added `stunPort` and `sessionId` member variables

- `src/Network/MetaServerClient.cpp`:
  - Updated `startAnnounce()` to pass `stunPort`
  - Updated METASERVERCOMMAND_ADD to send `stun_port` and parse `session_id`
  - Updated METASERVERCOMMAND_LIST to use `list2` and parse NAT traversal fields

- `src/Network/NetworkManager.cpp`:
  - Added STUN query in `startServer()` before announcing
  - Passes discovered `stunPort` to metaserver
  - Added `sendHolePunchPackets()` - sends DLHP packets to create NAT mappings
  - Added `performStunQuery()` - public method for client-side STUN
  - Added host-side punch polling in `update()` - polls every 1s, responds to requests

- `src/Menu/MultiPlayerMenu.cpp`:
  - Extended `onJoin()` with hole punch flow for internet games:
    - STUN query, punch_request, poll punch_status, send punch packets
  - Falls back to direct connect if hole punch unavailable or fails

- `src/CMakeLists.txt`:
  - Added `Network/StunClient.cpp` to build

## Tests Added / Updated
None yet.

## Review Notes (Codex)

### Implementation Review (Post 4d47cc8)
**Metaserver review:** Consistent with Rev 5 - add accepts stun_port + returns sessionId, list2 adds 2 fields, punch endpoints are GET + line-based, IP derived server-side.

**Non-blocking concerns (metaserver):**
1. `punch_request` only checks "sessionId exists", not "game still active" (lastUpdate within SERVER_TIMEOUT). Consider rejecting punch for expired servers.
2. `punch.dat` read/modify/write isn't locked end-to-end (writes are LOCK_EX but no flock around whole transaction). Probably fine at low volume.

**Client review:**
- StunClient is IPv4-only - acknowledged as fine
- ~~**BLOCKER:** Host-side punching blocks main loop~~ **FIXED**
  - Refactored to time-sliced state machine:
    - `PendingPunch` struct stores scheduled punches
    - No SDL_Delay() - sends 1 packet per 50ms across update() calls
    - Punch polling happens in background, punches scheduled for T+2000ms

### Rev 5 Outcome
APPROVE (design is implementable in this repo with no new dependencies).

### Rev 5 Non-blocking Notes
- Add an explicit “fallback to direct connect” section: if `holePunchAvailable == false`, or any punch step times out/errors, attempt direct `ip:port` connect as today.
- Clarify the STUN call sites to match the current code: ENet host/socket is created in `NetworkManager::NetworkManager()`, so STUN should run in `startServer()` before `pMetaServerClient->startAnnounce(...)`, and on the client just before calling `NetworkManager::connect(...)`.
- Consider making punch polling + punch burst time-sliced (state machine) to avoid UI stalls from `SDL_Delay()`, but OK for v1.

### Rev 4 Outcome
REJECT (major blockers fixed; remaining gaps are “can we implement this safely with current codebase”).

### Rev 4 Blocking Issues
1. **`session_id` must be stable for the lifetime of a game (and your snippet currently regenerates it).** In `handleAdd()` you generate a new `session_id` unconditionally. But the existing client code can call `command=add` again (e.g., `update` failure fallback), and any regeneration will break in-flight punch flows:
   - host polls by `secret` → replies are stored by `session_id`
   - client polls by the `session_id` it saw in `list2`
   - if `session_id` changes mid-lobby, `punch_status` will never reach “ready”.
   - Required: only generate `session_id` when the game is first created for that `secret`; otherwise preserve the existing `session_id` (and `stun_port`) for that record.
2. **`list2` JSON parsing plan is missing (and there is no JSON lib in the repo today).** `design.md` assumes JSON (`command=list2`) and JSON bodies for POST endpoints, but the game currently has no JSON parser dependency. You need to pick one:
   - Add a JSON library via vcpkg (and document exact dependency + CMake integration), or
   - Avoid JSON entirely for v1: make `list2` be a tab-separated “list v2” format and make POST endpoints return line-based plain text (`OK\nclient_id\n...`) so the client can parse without a JSON library.
3. **Client-side HTTP POST details are missing.** The current HTTP helper (`src/Network/ENetHttp.cpp`) only supports GET with query params. Rev 4 introduces POST endpoints (`punch_request`, `punch_ready`) but doesn’t specify the exact C++ changes (new `loadFromHttpPost(...)`, headers, timeout, error handling) or how the body is serialized.

### Rev 4 Non-blocking Notes
- The “STUN only pre-connection” safety story is acceptable if the implementation ensures the STUN query runs synchronously in a code path where `NetworkManager::update()` is not executing (single-threaded main loop).
- Consider moving the blocking `SDL_Delay` loops (polling/punch burst) into a time-sliced state machine to avoid UI stalls.

### Rev 3 Outcome
REJECT (much closer; remaining issues are fixable but correctness-critical).

### Rev 3 Blocking Issues
1. **The STUN receive story is still incorrect/dangerous.** Rev 3 says STUN runs on `host->socket` (good), but the proposed `enet_socket_receive()` loop cannot “leave non‑STUN packets in the socket buffer for ENet”. Once you `receive()`, the datagram is consumed. If this runs while ENet traffic exists, you will drop real ENet packets and/or race `enet_host_service()`.
   - Required: pick one safe mechanism and document it precisely:
     - **Preferred:** use ENet’s intercept callback (`host->intercept`) to capture STUN responses while continuing to service ENet; consume only STUN packets.
     - **Acceptable:** guarantee STUN queries only occur when no ENet traffic can exist (pre-connection / no peers) and suspend `enet_host_service()` during the STUN query window.
2. **Host vs client responsibilities during punching need to be explicit.** The current `executeHolePunch()` snippet ends with `enet_host_connect()`, which is client-side behavior. The host should generally *not* call `enet_host_connect()`; it should only send punch packets and then accept the incoming ENet connect.
   - Required: spell out “client does X, host does Y” and where each is triggered in the existing code paths.
3. **Metaserver endpoint consistency:** the flow mentions “POST /announce”, but the endpoint list does not define it, and the current system uses `command=add`/`update` via query params. Decide whether:
   - `command=add` accepts optional `stun_port` (recommended; minimal change), or
   - you introduce a new JSON-based announce endpoint and update the client accordingly.

### Rev 3 Non-blocking Notes
- `stunPort` range check: consider allowing `1–65535` (not `1024+`) since NATs can theoretically map to low ports; the anti-abuse story is already handled by “IP derived from connection”.
- The `list2` selection snippet uses `starts_with` which is C++20; Dune Legacy is C++17—use a C++17-friendly check in implementation.

### Rev 2 Outcome
REJECT (design is close, but two core assumptions are still wrong/unsafe).

### Rev 2 Blocking Issues
1. **STUN on a separate socket likely returns the wrong external port.** The design says STUN uses a temporary socket and “binds to same local port as ENet if possible”, but ENet is already bound (clients use `settings.network.serverPort`), so this will usually be impossible. If STUN is not performed on the *same UDP socket/port* that will send/receive gameplay traffic, the discovered external `IP:port` may not match ENet’s NAT mapping and hole punching will fail.
   - Required: define a STUN query path that uses ENet’s socket (`host->socket`) for the Binding Request/Response, or restructure startup so the same local port is used deterministically without competing binds.
2. **Adding `session_id` as a 13th `list` field breaks compatibility.** Current client parsing expects 9–12 tab-separated fields; a 13th field requires code changes in this repo (fine), but older shipped clients will likely reject the response. This is avoidable.
   - Required: keep `command=list` output stable and add a new endpoint (e.g. `command=list2` or `command=list_json`) for the new fields (`session_id`, `external_port`, etc.), or otherwise provide a backward-compatible mechanism.
3. **Signaling allows IP spoofing / abuse unless metaserver overwrites IPs.** `client_addr`/`host_addr` are client-provided strings. If accepted as-is, an attacker can make the host send UDP to arbitrary victims.
   - Required: metaserver must derive IP from `getRealClientIP()` and only accept the *port* (from STUN) from the client/host; reject mismatched IPs and enforce tight validation.

### Rev 2 Non-blocking Notes
- The hole-punch “DLHP” raw UDP packet is fine for NAT mapping creation, but it may generate ENet “invalid packet” noise; call that out and ensure it cannot destabilize ENet processing.
- Prefer `punch_in_seconds` over absolute `punch_at` to avoid clock skew, as already noted in open questions.
- Rate limiting via APCu is environment-dependent; specify the fallback concretely (even if “none for v1”).

### Blocking issues
1. **ICE/ENet integration is not defined (likely incorrect as written).** The design assumes “ICE negotiates an address, then ENet connects to it”. ICE hole punching typically depends on the *same UDP socket/port mapping* that ran connectivity checks; switching to a different socket (ENet’s) can invalidate the NAT mapping. The design must specify exactly how packets flow during negotiation and how ENet reuses (or is layered on) the ICE-established transport.
2. **Metaserver API uses `secret`, but clients don’t have it.** The current server list response does not include `secret` (only IP/port/name/etc). Any client→metaserver signaling must use a public game/session identifier, not the host’s secret, and must prevent arbitrary third parties from injecting/polling payloads.
3. **SDP/candidate payload size + transport mismatch.** Current metaserver input sanitization truncates to 255 chars; SDP/local descriptions are larger. Current client HTTP helper only supports GET query params; URLs will exceed safe limits. Design must specify POST (or another transport) + explicit payload size limits + storage/TTL.
4. **Abuse/rate-limit and cleanup plan missing.** Candidate exchange introduces new write/poll endpoints. Need: per-session TTL cleanup, bounded storage, per-IP rate limiting, and validation of payloads (size/content). Otherwise `servers.dat` can balloon and polling can become a DoS vector.

### High-impact design questions (answer in `design.md`)
- What is the *exact* socket ownership model?
  - Option A: libjuice drives a UDP socket and ENet must be adapted to use that same socket/port (ENet changes likely).
  - Option B: ENet owns the socket; ICE uses callbacks to send/recv on ENet’s socket and ENet must demux ICE vs ENet traffic during negotiation.
  - Option C: Don’t do ICE: implement STUN + coordinated UDP hole punching on the existing ENet socket (smaller scope, still no TURN).
- What is the public identifier that clients use for signaling, and how is it obtained from `list` results?
- What are the exact metaserver endpoints (method, params/body, responses), size limits, and TTLs?

### Non-blocking suggestions
- Put STUN servers behind config (`Dune Legacy.ini`) with sane defaults; avoid hard-coding to Google.
- Define timeouts/retries per phase (gather, exchange, punch, fallback) and how they surface in UI/logs.
- Clarify privacy implications: local candidates may leak LAN IPs; decide if/when to redact host candidates.

## Open Issues / Follow-ups
- [x] Update `design.md` per Codex review (Rev 1 → Rev 2)
- [x] Decide ICE/ENet integration option - **Option C (STUN + hole punch)**
- [x] Update Rev 2 per Codex blockers - **Done Rev 3**
  - [x] STUN on ENet socket (not temp socket)
  - [x] New `list2` endpoint for backward compat
  - [x] Metaserver derives IP, only accepts port
- [x] Update Rev 3 per Codex blockers - **Done Rev 4**
  - [x] STUN only pre-connection (no packet interleaving)
  - [x] Explicit host/client punch roles
  - [x] `command=add` extended (not new endpoint)
- [x] Update Rev 4 per Codex blockers - **Done Rev 5**
  - [x] session_id preserved on re-add
  - [x] No JSON - all GET + line-based
  - [x] Existing loadFromHttp() works
- [x] Codex re-review of Rev 5 - **APPROVED**
- [x] Owner approval of revised design - **APPROVED**
- [x] Added fallback to direct connect section
- [x] Clarified STUN call sites
- [ ] **NEXT:** Implement metaserver protocol changes
- [ ] Implement STUN client (no external library)
- [ ] Wire up hole punch flow in game client
