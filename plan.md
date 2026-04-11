# Goal
Make `ngircd_impl.dll` stop being the biggest DLL. Move as much code as possible out of it into the existing DLLs. Prefer moving whole subsystems, not wrappers.

# Current priority order
Largest code still inside `ngircd_impl.dll`:
1. `net/conn.c`
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

# Execution plan
## Phase 1: Freeze `ngircd_impl`
Treat `ngircd_impl` as deprecated implementation space.
- No new business logic in `ngircd_impl`.
- New or moved code must live in target DLLs.
- Only temporary forwarding glue is allowed.

## Phase 2: Move networking into `net_transport.dll`
Move ownership of connection lifecycle, socket I/O, accept/read/write flow, send queues, connection timers, and low-level connection helpers out of `ngircd_impl`.

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

# Implementation rules
For every phase:
1. Move complete ownership, not helper fragments.
2. Export a minimal API from the target DLL.
3. Update callers to use that API.
4. Leave a tiny compatibility shim only if needed.
5. Then delete old implementation from `ngircd_impl`.

# Success criteria
- `ngircd_impl.dll` is no longer the biggest DLL.
- Most code from the five largest remaining source areas is no longer linked into `ngircd_impl`.
- `ngircd_impl` is mostly bootstrap/compat glue.
- Build passes and runtime behavior is unchanged.

# What to do first
Start with `net/conn.c`. It is the highest-leverage move and should produce the biggest immediate size drop in `ngircd_impl.dll`.
