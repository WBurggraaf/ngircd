# AGENTS.md

## Mission

Convert this repository to a **real thin-host + DLL** architecture as fast as possible without breaking baseline IRC behavior.

The near-term goal is not a perfect final decomposition. The near-term goal is to reach a build where:

- `ngircd.exe` is mostly bootstrap and lifecycle code
- real subsystem DLLs are produced, not just static-library placeholders
- every cross-module surface is C ABI and versioned
- future C# substitution is practical for selected high-level modules

This is a staged refactor of an existing native codebase.
It is not a rewrite.

---

## What the repository already shows

From the current tree and build:

- The repo already has split headers and wrapper sources under `include/` and `src/`.
- The build already emits `build/core_runtime.dll`.
- Most other new subsystem targets are still static libraries (`*.lib`) and are not yet real DLLs.
- `ngircd.exe` still directly compiles the original monolithic sources from `original/src/ngircd/...`.
- The current `host_api` layer is still a wiring stub, not a dynamic-loader-based host.
- `module_get_api_v1` already exists as the intended ABI pattern and must remain the standard entrypoint shape.

Treat this as a partially-completed extraction, not a clean modular system.

---

## Fastest path principle

Choose the path that gets to **actual DLL usage** fastest while keeping the EXE small:

1. **Keep hot-path code native.**
   - transport, polling, socket loops, resolver shims, and low-level runtime stay native-first.

2. **Promote existing wrapper targets to real shared libraries before inventing new abstraction layers.**
   - The repo already has module-shaped source directories. Use them.

3. **Move orchestration out of the EXE before moving deep behavior out of the core.**
   - Thin host first.
   - Perfect subsystem purity later.

4. **Prefer coarse DLL boundaries first.**
   - Too many small DLL seams early will slow the refactor and increase ABI churn.

5. **Do not move allocation ownership across DLLs casually.**
   - C ABI + explicit allocators only.

---

## Recommended execution order

### Phase 0 - Stabilize the ABI contract

Do first:

- Keep `core_runtime` as the contract anchor.
- Standardize export/import macros, calling convention macros, status codes, version structs, and service tables there.
- Require every module to expose `module_get_api_v1`.
- Add explicit ownership notes for every public struct and function.

This is the foundation for both native DLL loading and future C# compatibility.

### Phase 1 - Make the host executable genuinely thin

Fastest win:

- Move bootstrap/orchestration into `host` and `server_app` layers.
- Keep `ngircd.exe` responsible only for:
  - process entry
  - argument forwarding
  - loading core modules
  - resolving exported APIs
  - top-level lifecycle
  - fatal startup failure reporting

Do not leave config parsing, protocol dispatch, or server logic in the EXE.

### Phase 2 - Promote existing static module targets to shared libraries

Do this before a large source reorganization.

Immediate target set:

- `core_runtime.dll` (already present)
- `logging.dll`
- `config.dll`
- `platform.dll`
- `net_transport.dll`
- `resolver.dll`
- `server_app.dll`

These are the fastest modules to make real because the repository already has target stubs for them.

Keep these provisional or internal until dependencies are cleaner:

- `irc_protocol`
- `client_state`
- `channel_state`
- `command_handlers`

### Phase 3 - Replace direct EXE-to-legacy calls with module calls

Refactor in this order:

- config/bootstrap path
- logging path
- runtime/platform shims
- server lifecycle
- resolver/network support hooks
- protocol/state/command surfaces

The key milestone is when `ngircd.exe` no longer directly owns the original subsystem behavior.

### Phase 4 - Split high-level logic for future managed substitution

Best future .NET candidates:

- `config`
- `logging` interface layer
- selected `irc_protocol` validation/dispatch helpers
- parts of `command_handlers`
- portions of `server_app` orchestration

Poor .NET candidates for early replacement:

- `platform`
- `net_transport`
- resolver internals tied to socket/event timing
- tightly coupled low-level state mutation in hot loops

---

## Current practical module strategy

### Keep native and low-level

These should remain native-first and performance-conservative:

- `core_runtime`
- `platform`
- `net_transport`
- most of `resolver`

### Make DLL-backed now

These should become real DLLs as soon as possible:

- `logging`
- `config`
- `server_app`
- `core_runtime`

### Delay full externalization until seams are cleaner

These likely need another cleanup pass before they become externally stable DLLs:

- `irc_protocol`
- `client_state`
- `channel_state`
- `command_handlers`

---

## Mandatory ABI rules

Every public module boundary must:

- use a C ABI only
- use fixed-width integer types
- expose version fields
- avoid cross-module CRT ownership assumptions
- avoid exposing internal structs directly unless they are explicitly stable
- prefer opaque handles + API tables + caller-owned buffers
- define thread/lifetime expectations

Preferred public pattern:

- one public module header
- one metadata struct
- one API function table
- one `module_get_api_v1` export
- host-provided service table input

---

## EXE size rule

Always prefer this shape:

- thin `ngircd.exe`
- more code in DLLs
- no business logic retained in the executable just because it is convenient

A small EXE is a success metric, not a side effect.

---

## Build-system rule

The build must stop pretending modules are DLLs when they are still static-only.

Required near-term cleanups:

- rename `*_api` targets to real module names when they become shared libraries
- use `SHARED` where the module is intended to be loaded as a DLL
- centralize export macro definitions per module
- make it obvious which modules are runtime-loaded versus link-time coupled

---

## Definition of done for the next serious milestone

The next milestone is done when all of the following are true:

- `ngircd.exe` is orchestration-only
- at least `core_runtime`, `logging`, `config`, and `server_app` are real DLLs
- the host resolves module APIs through versioned C ABI entrypoints
- public ownership rules are explicit
- no new ABI blocks future C# replacement of the selected high-level modules

