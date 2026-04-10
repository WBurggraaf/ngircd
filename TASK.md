# TASK.md

## Title

Get this repository to **real DLL usage quickly** while shrinking `ngircd.exe` and preserving a future C#-linkable path through stable C ABI contracts.

---

## Immediate objective

Turn the current partial refactor into a practical modular runtime.

The current tree already contains module-shaped headers, wrappers, and one actual DLL output (`core_runtime.dll`), but the main executable still compiles the legacy monolith directly and most subsystem targets are still static libraries.

The task is to close that gap with the least disruptive sequence.

---

## Current state summary

### Already present

- split include/source layout for modules
- `module_get_api_v1` contract direction
- `core_runtime.dll` output in the build tree
- static-library stubs for most intended modules
- host wiring stub in `src/host/host.c`
- architecture docs under `docs/refactor/`

### Still blocking a true modular runtime

- `ngircd.exe` still compiles directly from `original/src/ngircd/...`
- most modules are linked as static libraries, not loaded as DLLs
- the host is not yet the sole orchestrator
- high-level runtime flow still depends on legacy compile-time coupling

---

## Success criteria

The work is successful only if these are true:

1. `ngircd.exe` becomes small and orchestration-focused.
2. At least the first wave of modules are real DLLs, not only `.lib` outputs.
3. Public module boundaries are stable C ABI with versioned exports.
4. The host can wire modules through API tables instead of direct internal coupling.
5. Ownership and allocator expectations are explicit across module boundaries.
6. The path remains open for later C# replacement of selected high-level DLLs.
7. Baseline IRC behavior remains intact.

---

## Fastest recommended milestone order

### Milestone 1 - Make the host truly thin

Move responsibility out of `ngircd.exe` until the EXE handles only:

- process entry
- CLI pass-through
- bootstrap
- module load/resolve
- lifecycle start/stop
- fatal startup failure handling

### Milestone 2 - Convert the easiest existing modules to DLLs

Do first:

- `core_runtime`
- `logging`
- `config`
- `platform`
- `resolver`
- `net_transport`
- `server_app`

Reason: these already have scaffold targets and can become real DLLs faster than a deep protocol/state split.

### Milestone 3 - Route behavior through module APIs instead of direct monolith linkage

Priority order:

1. startup/config
2. logging
3. server lifecycle
4. platform/runtime helpers
5. resolver/network helpers
6. protocol and command surfaces

### Milestone 4 - Cleanly separate high-level modules for future .NET compatibility

Only after the first DLL wave is stable:

- `irc_protocol`
- `command_handlers`
- selected orchestration parts of `server_app`
- possibly parts of state management once ownership is explicit

---

## Recommended module waves

### Wave A - do now

- `core_runtime.dll`
- `logging.dll`
- `config.dll`
- `server_app.dll`

These give the fastest path to a smaller EXE and a stable host/module model.

### Wave B - do next

- `platform.dll`
- `resolver.dll`
- `net_transport.dll`

These are important, but should stay native-first and performance-conscious.

### Wave C - do after seams are cleaner

- `irc_protocol.dll`
- `client_state.dll`
- `channel_state.dll`
- `command_handlers.dll`

These are likely to require more untangling and are a worse first target if the goal is speed.

---

## Constraints

- Do not rewrite the IRC server from scratch.
- Do not introduce C++ ABI at module boundaries.
- Do not promise .NET replacement for low-level hot-path modules.
- Do not keep business logic in the EXE for convenience.
- Do not create DLL seams that require unsafe cross-module memory ownership.
- Keep Windows support first-class.

---

## ABI requirements

Every public module must:

- expose `module_get_api_v1`
- return a versioned API table
- use explicit export/import and calling convention macros
- use fixed-width integer types
- document ownership and lifetime
- avoid module-private struct layout leakage

Preferred surface:

- opaque handles
- API tables
- caller-owned buffers
- host service tables
- status-code returns

---

## C# compatibility rule

Design for **C# callable/replaceable in the future**, not C# implemented now.

Good future managed candidates:

- config processing
- logging front-end contract
- protocol validation helpers
- command dispatch/orchestration
- selected server_app behaviors

Poor early managed candidates:

- socket/event loop code
- resolver internals in timing-sensitive paths
- low-level platform shims

---

## Concrete next actions

1. Change the first-wave module targets from `STATIC` to real shared-library outputs where dependency hygiene already permits it.
2. Make host bootstrap consume module API tables.
3. Remove direct EXE ownership of config/logging/server orchestration behavior.
4. Keep protocol/state extraction incremental, not all-at-once.
5. Update docs whenever an intended DLL is still only a static placeholder.

---

## Non-goal

The non-goal is a theoretically perfect subsystem graph on day one.

The goal is the fastest safe route to:

- real DLL use
- small EXE
- stable native ABI
- future managed-compatible high-level seams

