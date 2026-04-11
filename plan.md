# Goal
Make `ngircd_impl.dll` stop being the biggest DLL. Move as much code as possible out of it into the existing DLLs. Prefer moving whole subsystems, not wrappers.

# Current priority order
Largest code still inside `ngircd_impl.dll`:
1. `net/conn.c`          ← **Phase 2 IN PROGRESS**
2. `core/conf.c`
3. `proto/irc-info.c`
4. `core/client.c`
5. `channel.c`

Rule: attack in that order unless blocked.

# Constraints
- Do **not** add new DLLs.
- Do **not** leave large implementations behind wrappers in `ngircd_impl`.
- `ngircd_impl` should shrink toward bootstrap/compat only.
- Preserve behavior and build success after each phase.

---

# Execution plan

## Phase 1: Freeze `ngircd_impl` ✅ DONE
Treat `ngircd_impl` as deprecated implementation space.
- No new business logic in `ngircd_impl`.
- New or moved code must live in target DLLs.
- Only temporary forwarding glue is allowed.

## Phase 2: Move networking into `net_transport.dll` ← IN PROGRESS
Move ownership of connection lifecycle, socket I/O, accept/read/write flow, send queues, connection timers, and low-level connection helpers out of `ngircd_impl`.

### Current state
- `conn.c`, `conn-func.c`, `conn-encoding.c`, `io.c` have been moved from `ngircd_impl` sources into `net_transport` in `CMakeLists.txt`.
- A **circular dependency** blocks the straightforward approach: `ngircd_impl` proto files (`irc.c`, `irc-write.c`, etc.) call `Conn_*` directly, while `conn.c` calls `Client_*`, `Conf_*`, `Log_*` still in `ngircd_impl`.
- CMake forbids SHARED→SHARED circular `target_link_libraries`.

### Chosen solution: runtime shim in `ngircd_impl`
- `src/ngircd_impl/conn_shim.c` compiled into `ngircd_impl.dll`
- Provides all `Conn_*` and `io_*` symbol definitions inside `ngircd_impl` via `GetProcAddress`-based lazy forwarding to `net_transport.dll` at runtime
- `ConnShim_Init()` must be called from `NGIRCd_Init()` before any connection activity
- `Conn_WriteStrBuf(CONN_ID, const char*)` added to `conn.c` as a non-variadic helper that the shim calls (shim cannot forward `va_list`)
- No CMake circular dependency — `ngircd_impl` does not link `net_transport.lib`; it loads the DLL at runtime

### Next steps for Phase 2
1. Call `ConnShim_Init()` from `ngircd_daemon.c` → `NGIRCd_Init()`
2. Build and verify all 27 tests pass
3. Confirm `ngircd_impl.dll` size drops (conn.c ~80 KB, io.c ~20 KB, conn-func.c ~6 KB, conn-encoding.c ~5 KB moved out)

Target result:
- `net_transport.dll` owns connection machinery.
- `ngircd_impl` only calls public transport APIs.

## Phase 3: Move config into `config.dll`
Move config parsing, config storage, reload logic, defaults, validation, and config lookup helpers.

Target result:
- `config.dll` owns all configuration state and behavior.
- Other DLLs consume config APIs only.

## Phase 4: Move protocol core into `irc_protocol.dll`
Move IRC parsing, message construction, protocol formatting, numeric/reply generation, and protocol info helpers.

Target result:
- `irc_protocol.dll` owns protocol rules and wire-format behavior.
- `ngircd_impl` stops formatting protocol details directly.

## Phase 5: Move state ownership
Move client state into `client_state.dll` and channel state into `channel_state.dll`.

Move:
- structs
- lifecycle
- lookup/indexing
- membership bookkeeping
- mutation helpers

Target result:
- state DLLs own their data and invariants.
- other DLLs mutate state only through public APIs.

## Phase 6: Move command behavior into `command_handlers.dll`
Move command dispatch-side behavior that still depends on monolith internals.

Target result:
- command handlers orchestrate via APIs from protocol, state, config, and transport DLLs.
- handlers do not reach into `ngircd_impl` internals.

## Phase 7: Remove residual monolith dependencies
Eliminate globals and cross-module reach-through still forcing code to stay in `ngircd_impl`.

Do this by:
- relocating ownership of globals to the correct DLL
- replacing direct struct access with APIs
- inverting dependencies so leaf DLLs do not depend on `ngircd_impl`

---

# Implementation rules
For every phase:
1. Move complete ownership, not helper fragments.
2. Export a minimal API from the target DLL.
3. Update callers to use that API.
4. Leave a tiny compatibility shim only if needed.
5. Then delete old implementation from `ngircd_impl`.

---

# Success criteria
- `ngircd_impl.dll` is no longer the biggest DLL.
- Most code from the five largest remaining source areas is no longer linked into `ngircd_impl`.
- `ngircd_impl` is mostly bootstrap/compat glue.
- Build passes and runtime behavior is unchanged.

---

# Architecture notes

## DLL dependency graph (current)
```
core_runtime  ← nothing
platform      ← core_runtime  (managed NativeAOT C#)
logging       ← core_runtime  (managed NativeAOT C#)
config        ← core_runtime  (managed NativeAOT C#)
irc_protocol  ← core_runtime
client_state  ← core_runtime
channel_state ← core_runtime
command_handlers ← core_runtime
nghost        ← core_runtime
net_transport ← core_runtime, ngircd_impl  +  conn.c/io.c sources (Phase 2 in progress)
resolver      ← core_runtime, ngircd_impl
server_app    ← core_runtime, ngircd_impl, logging, config, net_transport
ngircd_impl   ← ngportab, ngtool, ngipaddr, ws2_32, core_runtime, platform
               [runtime-loads net_transport.dll via conn_shim.c]
ngircd.exe    ← ngircd_impl, server_app, net_transport, resolver, host
```

## Shim pattern (for future phases)
When moving code X from `ngircd_impl` → target.dll where callers in `ngircd_impl` still depend on X:
1. Add all functions from X's public header to a `x_shim.c` compiled into `ngircd_impl`
2. Each shim function: lazy `GetProcAddress` → call through
3. For variadic functions: add a `_Buf` variant that takes a pre-formatted string
4. No `target_link_libraries` change needed — runtime-only loading

## Test coverage
27 tests, all passing. Key module-API tests added:
- `core_runtime_test` (22 checks)
- `host_wiring_test`  (34 checks)
- `logging_module_test` (21 checks) — uses managed Logging.dll (NativeAOT C#), verifies file write
- `config_module_test` (21 checks)
- `platform_module_test` (19 checks)
- `server_app_unit_test` (30 checks)
