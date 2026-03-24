# NAT Traversal via STUN + Coordinated Hole Punching

## Status
IN_PROGRESS (Implementation complete, pending testing)

## Revision History
| Rev | Date | Changes |
|-----|------|---------|
| 1 | 2024-12-15 | Initial design with libjuice/ICE |
| 2 | 2024-12-20 | Dropped ICE, STUN + hole punch |
| 3 | 2024-12-20 | STUN on ENet socket, `list2` endpoint, IP derived server-side |
| 4 | 2024-12-20 | STUN only pre-connection, explicit host/client roles, extend `command=add` |
| 5 | 2024-12-20 | **Fixes**: (1) session_id stable per secret, (2) No JSON - all GET + line-based responses |
| 5a | 2024-12-20 | **APPROVED** - Added: fallback to direct connect, STUN call site clarification |

## Problem / Goal

**Problem:** Internet multiplayer games fail to connect when the host is behind a NAT router without UPnP support or manual port forwarding.

**Goal:** Enable peer-to-peer connections through NAT using STUN for address discovery and coordinated UDP hole punching.

## Non-Goals

- Full ICE implementation
- TURN relay server (future work)
- Modifying ENet internals
- Adding JSON parsing library (v1 keeps line-based protocols)

## session_id Stability (Rev 5 - CORRECTED)

### Problem (from Rev 4 review)
The `handleAdd()` snippet regenerated `session_id` unconditionally. But clients can call `command=add` again (e.g., on `update` failure fallback), which would break in-flight punch flows:
- Host polls by `secret` → replies stored by `session_id`
- Client polls by `session_id` seen in `list2`
- If `session_id` changes mid-lobby, `punch_status` never reaches "ready"

### Solution: Preserve session_id per secret

```php
function handleAdd() {
    $secret = $_GET['secret'] ?? '';
    $stunPort = isset($_GET['stun_port']) ? intval($_GET['stun_port']) : null;
    
    // Check if this secret already has a game registered
    $existingGame = getGameBySecret($secret);
    
    if ($existingGame) {
        // RE-ADD: Preserve existing session_id and stun_port (unless new stun_port provided)
        $sessionId = $existingGame['session_id'];
        $stunPort = $stunPort ?? $existingGame['stun_port'];
        
        // Update other fields (name, map, players, etc.) but keep IDs stable
        updateGame($secret, [
            // ... updated fields from request ...
            'session_id' => $sessionId,  // PRESERVED
            'stun_port' => $stunPort,    // PRESERVED or updated
            'lastUpdate' => time()
        ]);
    } else {
        // NEW GAME: Generate new session_id
        $sessionId = bin2hex(random_bytes(6));  // 12 hex chars
        
        createGame([
            'secret' => $secret,
            'session_id' => $sessionId,  // NEW
            'stun_port' => $stunPort ?? $port,
            // ... other fields ...
        ]);
    }
    
    echo "OK\n";
    echo $secret . "\n";
    echo $sessionId . "\n";
}
```

### Lifecycle Guarantees

| Event | session_id | stun_port |
|-------|------------|-----------|
| First `command=add` | Generated | From STUN or port |
| Re-add (same secret) | **Preserved** | Preserved (unless new value) |
| `command=remove` | Deleted | Deleted |
| Game timeout | Deleted | Deleted |

## No JSON - All GET + Line-Based Responses (Rev 5 - SIMPLIFIED)

### Problem (from Rev 4 review)
- Game has no JSON parsing library
- `ENetHttp.cpp` only supports GET with query params
- Adding JSON library is scope creep for v1

### Solution: Keep everything GET + line-based

All punch endpoints use:
- **Request:** GET with query parameters
- **Response:** Line-based text (`OK\n` or `ERROR\n` followed by data lines)

This matches existing metaserver protocol style and requires no new dependencies.

## Metaserver Endpoints (Rev 5 - Complete Specification)

### Existing Endpoints (Unchanged)
| Endpoint | Format |
|----------|--------|
| `command=list` | Tab-separated, 12 fields |
| `command=update` | `OK\n` or `ERROR\n` |
| `command=remove` | `OK\n` or `ERROR\n` |

### Modified Endpoints

#### `command=add` (Extended)

**Request:**
```
GET /metaserver.php?command=add&port=28747&name=...&stun_port=28747&...
```

New optional parameter: `stun_port` (external port from STUN discovery)

**Response:**
```
OK
<secret>
<session_id>
```

Line 3 (`session_id`) is new. Old clients ignore extra lines.

#### `command=list2` (New - Tab-Separated, NOT JSON)

**Request:**
```
GET /metaserver.php?command=list2
```

**Response:**
```
OK
<ip>\t<port>\t<name>\t<version>\t<map>\t<numplayers>\t<maxplayers>\t<pwd>\t<lastupdate>\t<localip>\t<modname>\t<modversion>\t<session_id>\t<stun_port>
...
```

Same format as `list`, but with 2 additional fields (14 total):
- Field 13: `session_id` (12 hex chars)
- Field 14: `stun_port` (external port for hole punch)

**Client parsing:**
```cpp
// MetaServerClient.cpp
if (parts.size() >= 14) {
    gameServerInfo.sessionId = parts[12];
    gameServerInfo.stunPort = static_cast<uint16_t>(std::stoi(parts[13]));
    gameServerInfo.holePunchAvailable = true;
} else {
    gameServerInfo.holePunchAvailable = false;
}
```

### New Punch Endpoints (All GET + Line-Based)

#### `command=punch_request`

Client requests hole punch coordination.

**Request:**
```
GET /metaserver.php?command=punch_request&session_id=abc123&stun_port=28747
```

Server derives `client_ip` from `getRealClientIP()`.

**Response (success):**
```
OK
<client_id>
```

**Response (error):**
```
ERROR
<message>
```

**Server-side:**
```php
function handlePunchRequest() {
    $sessionId = sanitize($_GET['session_id'] ?? '');
    $stunPort = intval($_GET['stun_port'] ?? 0);
    
    if (!validateSessionId($sessionId) || $stunPort < 1 || $stunPort > 65535) {
        echo "ERROR\nInvalid parameters\n";
        return;
    }
    
    $game = getGameBySessionId($sessionId);
    if (!$game) {
        echo "ERROR\nGame not found\n";
        return;
    }
    
    // Rate limit: 10/min per IP
    if (isRateLimited('punch_request', getRealClientIP(), 10, 60)) {
        echo "ERROR\nRate limited\n";
        return;
    }
    
    $clientId = bin2hex(random_bytes(8));  // 16 hex chars
    $clientIP = getRealClientIP();  // DERIVED, not from request
    
    storePunchRequest($sessionId, [
        'client_id' => $clientId,
        'client_ip' => $clientIP,
        'client_port' => $stunPort,
        'timestamp' => time()
    ]);
    
    echo "OK\n";
    echo $clientId . "\n";
}
```

#### `command=punch_poll`

Host polls for pending punch requests.

**Request:**
```
GET /metaserver.php?command=punch_poll&secret=<host_secret>
```

**Response (with pending requests):**
```
OK
<client_id>\t<client_ip>\t<client_port>
<client_id>\t<client_ip>\t<client_port>
...
```

**Response (no pending):**
```
OK
```

**Server-side:**
```php
function handlePunchPoll() {
    $secret = sanitize($_GET['secret'] ?? '');
    $game = getGameBySecret($secret);
    
    if (!$game) {
        echo "ERROR\nInvalid secret\n";
        return;
    }
    
    $requests = getPunchRequests($game['session_id']);
    
    echo "OK\n";
    foreach ($requests as $req) {
        echo $req['client_id'] . "\t" . $req['client_ip'] . "\t" . $req['client_port'] . "\n";
    }
    
    // Mark as delivered (so not returned again)
    markPunchRequestsDelivered($game['session_id']);
}
```

#### `command=punch_ready`

Host signals ready to punch a specific client.

**Request:**
```
GET /metaserver.php?command=punch_ready&secret=<host_secret>&client_id=<client_id>
```

**Response:**
```
OK
```

**Server-side:**
```php
function handlePunchReady() {
    $secret = sanitize($_GET['secret'] ?? '');
    $clientId = sanitize($_GET['client_id'] ?? '');
    
    $game = getGameBySecret($secret);
    if (!$game) {
        echo "ERROR\nInvalid secret\n";
        return;
    }
    
    // Store ready signal with host address
    storePunchReady($game['session_id'], $clientId, [
        'host_ip' => $game['ip'],
        'host_port' => $game['stun_port'] ?? $game['port'],
        'ready_at' => time()
    ]);
    
    echo "OK\n";
}
```

#### `command=punch_status`

Client polls for punch readiness.

**Request:**
```
GET /metaserver.php?command=punch_status&session_id=<session_id>&client_id=<client_id>
```

**Response (waiting):**
```
WAITING
```

**Response (ready):**
```
READY
<host_ip>
<host_port>
<punch_in_seconds>
```

**Server-side:**
```php
function handlePunchStatus() {
    $sessionId = sanitize($_GET['session_id'] ?? '');
    $clientId = sanitize($_GET['client_id'] ?? '');
    
    $readyInfo = getPunchReady($sessionId, $clientId);
    
    if (!$readyInfo) {
        echo "WAITING\n";
        return;
    }
    
    // Calculate punch delay (2 seconds from now, relative)
    $punchInSeconds = 2;
    
    echo "READY\n";
    echo $readyInfo['host_ip'] . "\n";
    echo $readyInfo['host_port'] . "\n";
    echo $punchInSeconds . "\n";
    
    // Clean up - one-time read
    deletePunchReady($sessionId, $clientId);
}
```

## Client-Side HTTP (No Changes Needed)

Since all endpoints are GET with query params and line-based responses, the existing `loadFromHttp()` function in `ENetHttp.cpp` works as-is:

```cpp
// Existing function signature - no changes needed
std::string loadFromHttp(const std::string& url, 
                         const std::map<std::string, std::string>& params);

// Usage for punch_request
std::map<std::string, std::string> params;
params["command"] = "punch_request";
params["session_id"] = gameInfo.sessionId;
params["stun_port"] = std::to_string(stunResult.externalPort);

std::string response = loadFromHttp(metaServerURL, params);

// Parse line-based response
std::vector<std::string> lines = splitString(response, '\n');
if (lines.size() >= 2 && lines[0] == "OK") {
    clientId = lines[1];
}
```

## STUN Safety (unchanged from Rev 4)

STUN queries only run pre-connection when no ENet peers exist:
- Host: After `enet_host_create()`, before `command=add`
- Client: Before `punch_request`, while no `connectPeer`

No concurrent `enet_host_service()` during STUN window.

## STUN Call Sites (Rev 5 - Clarified)

The ENet socket is created in `NetworkManager::NetworkManager()`, so STUN must run on the already-bound socket:

| Role | When | Where in Code |
|------|------|---------------|
| Host | After socket exists, before announcing | `startServer()` before `pMetaServerClient->startAnnounce(...)` |
| Client | Before initiating connect | Just before `NetworkManager::connect(...)` in MultiPlayerMenu |

```cpp
// Host: NetworkManager::startServer()
void NetworkManager::startServer(...) {
    // ... host is already created in constructor with bound socket
    
    // Run STUN before announcing (no peers exist yet)
    uint16_t stunPort = performStunQuery(host->socket);
    
    // Pass stunPort to metaserver
    pMetaServerClient->startAnnounce(..., stunPort);
}

// Client: MultiPlayerMenu before connect
void MultiPlayerMenu::onJoin() {
    // ... before calling connect
    
    if (gameInfo.holePunchAvailable) {
        uint16_t stunPort = pNetworkManager->performStunQuery();
        // Initiate punch flow with stunPort
    }
    
    pNetworkManager->connect(...);
}
```

## Fallback to Direct Connect

If hole punching is unavailable or fails, fall back to direct connection (existing behavior):

| Condition | Fallback Behavior |
|-----------|-------------------|
| `list2` unavailable (old server) | Use `list`, connect directly to `ip:port` |
| `holePunchAvailable == false` | Connect directly (no punch attempt) |
| `punch_request` timeout/error | Log warning, connect directly |
| `punch_status` timeout (10s) | Log warning, connect directly |
| Punch packets fail + no connection | Connection fails (as today) |

```cpp
// MultiPlayerMenu::onJoin() pseudocode
if (!gameInfo.holePunchAvailable) {
    // No hole punch data - direct connect
    pNetworkManager->connect(gameInfo.serverAddress);
    return;
}

// Attempt hole punch
auto punchResult = attemptHolePunch(gameInfo);
if (punchResult.success) {
    // Punched address may be different from original
    pNetworkManager->connect(punchResult.address);
} else {
    // Punch failed - try direct connect as last resort
    SDL_Log("Hole punch failed, attempting direct connect");
    pNetworkManager->connect(gameInfo.serverAddress);
}
```

This ensures users without UPnP still get a connection attempt (which may work if port is forwarded manually or if NAT is permissive).

## Host/Client Punch Roles (unchanged from Rev 4)

| Action | Host | Client |
|--------|------|--------|
| STUN query | Before add | Before punch_request |
| Send punch packets | Yes | Yes |
| `enet_host_connect()` | **NO** | **YES** |

## High-Level Flow (Rev 5 - All GET)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      HOLE PUNCH COORDINATION (Rev 5)                         │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  HOST                         METASERVER                        CLIENT      │
│   │                               │                               │         │
│   │── 1. STUN query (pre-conn) ──>│                               │         │
│   │<─ External stunPort ──────────│                               │         │
│   │                               │                               │         │
│   │── 2. GET add&stun_port=X ────>│                               │         │
│   │<─ OK\nsecret\nsession_id ─────│                               │         │
│   │                               │                               │         │
│   │                               │<── 3. GET list2 ──────────────│         │
│   │                               │─── OK\n<tab-separated rows> ─>│         │
│   │                               │    (includes session_id)      │         │
│   │                               │                               │         │
│   │                               │                ┌──────────────│         │
│   │                               │                │ 4. STUN query│         │
│   │                               │                └──────────────│         │
│   │                               │                               │         │
│   │                               │<── 5. GET punch_request ──────│         │
│   │                               │    &session_id&stun_port      │         │
│   │                               │─── OK\nclient_id ────────────>│         │
│   │                               │                               │         │
│   │<── 6. GET punch_poll ─────────│                               │         │
│   │    &secret                    │                               │         │
│   │    OK\nclient_id\tip\tport ──>│                               │         │
│   │                               │                               │         │
│   │── 7. GET punch_ready ────────>│                               │         │
│   │    &secret&client_id          │                               │         │
│   │<── OK ────────────────────────│                               │         │
│   │                               │                               │         │
│   │                               │<── 8. GET punch_status ───────│         │
│   │                               │    &session_id&client_id      │         │
│   │                               │─── READY\nhost_ip\nport\n2 ──>│         │
│   │                               │                               │         │
│   │         (both wait 2 seconds, then punch simultaneously)      │         │
│   │                               │                               │         │
│   │── 9. Punch packets (no connect) ─────────────────────────────>│         │
│   │<───────────────────────────────── 10. Punch + enet_connect ───│         │
│   │                               │                               │         │
│   │<─────────────── 11. ENet connection established ─────────────>│         │
│   │                               │                               │         │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Ops Safeguards (unchanged)

- TTL: 60s for punch requests, 30s for ready signals
- Max 5 punch requests per session
- Rate limit: 10 punch_request/min per IP

## Acceptance Criteria

- [ ] `session_id` preserved on re-add (same secret)
- [ ] All punch endpoints use GET + line-based responses
- [ ] No JSON library required
- [ ] Existing `loadFromHttp()` function works for all endpoints
- [ ] `list2` returns 14 tab-separated fields
- [ ] Old clients using `list` continue to work

## Test Plan

### Unit Tests
- [ ] `command=add` with existing secret preserves session_id
- [ ] `command=add` with new secret generates session_id
- [ ] `command=list2` returns 14 fields
- [ ] `punch_request` returns client_id on success
- [ ] `punch_status` returns READY with host info after punch_ready

### Integration Tests
- [ ] Full flow using only existing HTTP helper
- [ ] Re-add doesn't break punch flow
- [ ] Old client with new server (uses list, no punch)

## Rollout / Rollback

Same as previous revisions:
- `EnableHolePunch = false` disables feature
- `command=list` unchanged
- New endpoints are additive

## Resolved Issues

| Rev | Issue | Resolution |
|-----|-------|------------|
| 1-3 | (see previous) | (see previous) |
| 4 | session_id regenerated on re-add | Preserve per secret |
| 4 | JSON library required | All GET + line-based (no JSON) |
| 4 | POST not supported | All endpoints use GET |
