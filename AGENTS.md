# AGENTS.md

## Mission

Refactor this existing IRC server codebase from a largely monolithic native application into:

- one thin host executable
- multiple DLLs organized by subsystem
- stable C ABI boundaries between host and modules
- an architecture that can later allow selected native DLLs to be replaced by C#/.NET implementations behind compatible host-facing contracts

This is a **staged refactor of an existing codebase**, not a rewrite.

Behavior preservation is a first-class requirement.

---

## Operating principles

1. **Do not rewrite from scratch.**
   - Preserve existing behavior wherever possible.
   - Prefer extraction, seam creation, and encapsulation over redesigning everything at once.

2. **Do analysis first.**
   - Before major code edits, inspect the current code thoroughly.
   - Document the current architecture, coupling, globals, and runtime flow.
   - Base all module boundaries on real code structure and runtime behavior.

3. **Keep the code buildable frequently.**
   - Refactor in phases.
   - Minimize long-lived broken intermediate states.
   - At the end of each phase, summarize what compiles, what remains legacy, and what risks remain.

4. **All cross-module boundaries must use a C ABI.**
   - No C++ classes across DLL boundaries.
   - No STL types across DLL boundaries.
   - No compiler-specific mangled entrypoints as public contracts.
   - Use opaque handles, fixed-width integer types, pointer+length buffers, explicit versioning, and explicit ownership.

5. **Design for future managed substitution, but do not implement managed code now.**
   - Some high-level DLLs may later be implemented in C#/.NET.
   - Do not assume all modules are good candidates.
   - Be explicit about which modules should remain native and why.

6. **Prefer explicit ownership and service tables.**
   - Avoid hidden globals.
   - Avoid ambiguous memory ownership.
   - Prefer host-provided service tables and versioned function tables.

7. **Be conservative in hot paths.**
   - Avoid adding overly chatty ABI boundaries in transport, socket loops, or parser hot paths.
   - Do not sacrifice core performance or reliability for abstraction purity.

---

## Primary target architecture

The target shape is:

- `host.exe`
- `core_runtime.dll`
- `platform.dll`
- `config.dll`
- `logging.dll`
- `net_transport.dll`
- `resolver.dll`
- `irc_protocol.dll`
- `client_state.dll`
- `channel_state.dll`
- `command_handlers.dll`
- `server_app.dll`

This is the starting target. Adjust only when code inspection proves a better split.

---

## Host executable responsibilities

The executable must become as small as practical and should primarily handle:

- process entry
- argument parsing
- top-level bootstrap
- dynamic module loading
- interface resolution
- service table construction
- module wiring
- lifecycle control
- fatal error reporting
- shutdown orchestration

The host must not permanently retain business logic that belongs in modules.

---

## Mandatory ABI rules

Every DLL must expose a clear versioned entrypoint, for example:

- `module_get_api_v1`

Every exported public ABI must:

- use explicit export/import macros
- use explicit calling convention macros
- use `extern "C"` where applicable
- use fixed-width integer types
- avoid ambiguous ownership
- avoid CRT allocation mismatch across DLL boundaries
- define version fields in public structs
- document lifecycle and threading assumptions

Preferred patterns:

- opaque handles
- caller-provided buffers
- explicit create/init/start/stop/destroy functions
- host-provided allocator/logging/service tables
- status-code-based error returns

---

## Memory ownership rules

These rules are mandatory:

- Never assume memory can be allocated in one module and freed in another unless a shared allocator contract explicitly permits it.
- Prefer caller-owned buffers.
- If a callee allocates memory, it must either:
  - also expose the matching free function, or
  - use an explicitly shared allocator/service contract.
- Every public function must clearly document:
  - input ownership
  - output ownership
  - ownership on failure
  - lifetime of returned pointers

---

## Error handling rules

Use a common error/status model across modules.

Required:

- shared status code enum
- consistent success/failure conventions
- optional extended diagnostics path
- no ad hoc mixed conventions across different DLLs

Public ABI docs must state:

- success value
- recoverable failure behavior
- fatal failure behavior
- whether partial side effects can occur

---

## Dependency direction rules

Target dependency directions:

- host -> all modules
- `server_app.dll` -> high-level modules
- `command_handlers.dll` -> protocol + state + logging + service interfaces
- `client_state.dll` -> core/runtime only, plus logging only if justified
- `channel_state.dll` -> core/runtime only, plus logging only if justified
- `irc_protocol.dll` -> core/runtime only
- `resolver.dll` -> platform + core/runtime + logging
- `net_transport.dll` -> platform + core/runtime + logging
- `config.dll` -> core/runtime only
- `logging.dll` -> core/runtime only
- `platform.dll` -> minimal dependencies

Avoid peer-to-peer tangles. Prefer dependency inversion through service tables.

---

## Required first artifacts

Before major code restructuring, create these files:

- `docs/refactor/current-architecture.md`
- `docs/refactor/target-architecture.md`
- `docs/refactor/migration-plan.md`
- `docs/refactor/decision-log.md`
- `docs/refactor/dotnet-replacement-strategy.md`

Also create module docs under:

- `docs/refactor/modules/<module>.md`

Do not skip this documentation phase.

---

## Current architecture analysis requirements

The current architecture document must include:

- executable entry path
- startup/bootstrap flow
- config loading flow
- listener creation and socket path
- event loop
- connection lifecycle
- parse/dispatch path
- registration/login flow
- channel management flow
- message routing flow
- logging path
- resolver/DNS/ident flow
- reload/rehash path if supported
- shutdown path
- global state inventory
- subsystem ownership/coupling inventory
- build/link structure
- Windows-specific behavior relevant to modularization

Also identify:

- global mutable state
- singleton assumptions
- cyclic dependencies
- compile-time coupling hotspots
- cross-cutting helpers
- direct calls that should become interface-based

---

## Global state reduction policy

Reducing uncontrolled global state is a major goal.

For every global encountered, classify it as:

- true process-global
- host-owned singleton
- module-owned singleton
- instance-owned context
- removable legacy global

Prefer converting globals into:

- module context structs
- host-owned registries
- explicit instance state
- immutable config objects

Do not merely wrap globals in getters and call the problem solved.

---

## Recommended migration sequence

Use this sequence unless repository inspection reveals a better order:

### Phase 0: Inventory and guardrails
- map current files to responsibilities
- improve or add behavior-preserving tests
- capture baseline runtime flows
- prepare build support for modular outputs

### Phase 1: Shared runtime/contracts
- create shared ABI headers
- define status codes
- define export/calling convention macros
- define allocator/logging/service contracts
- define versioning helpers

### Phase 2: Host/server seam
- introduce a thin orchestration seam
- route startup through a stable internal interface
- begin isolating bootstrap responsibilities

### Phase 3: Extract low-risk modules
Extract first:
- `logging.dll`
- `config.dll`
- `core_runtime.dll`

### Phase 4: Extract platform/transport/resolver seams
- separate raw transport from IRC semantics
- define connection/event callbacks
- isolate resolver behavior

### Phase 5: Extract protocol layer
- parsing
- validation
- serialization helpers
- numeric construction helpers

### Phase 6: Extract state modules
- `client_state.dll`
- `channel_state.dll`

### Phase 7: Extract command handling
- route behavior through explicit service interfaces
- reduce direct coupling to globals

### Phase 8: Minimize the EXE
- move remaining orchestration into `server_app.dll`
- keep EXE limited to bootstrap/module wiring/lifecycle

---

## Testing requirements

As refactors proceed, maintain or add tests for:

- startup/shutdown
- config load
- client connect/register
- `CAP LS`
- `CAP REQ`
- `CAP END`
- `NICK`
- `USER`
- user registration completion
- user `MODE`
- `JOIN`
- channel `MODE` query
- `PRIVMSG` to joined channel
- connection close/disconnect
- numerics/error replies
- rehash/reload if supported

Also add:

- smoke tests per extracted DLL
- ABI/version mismatch tests
- missing-export/module-load failure tests
- ownership/lifetime tests where possible

Prefer extending existing tests before inventing a new framework.

---

## .NET replacement design policy

Design seams so later replacement by managed code is feasible for selected modules, but do not pretend all modules are suitable.

For each module, classify as:

- Native forever
- Native first, maybe managed later
- Good managed candidate

Include reasons such as:

- performance sensitivity
- marshalling complexity
- callback frequency
- OS coupling
- threading sensitivity
- debugability
- fault isolation

Likely better managed candidates:
- `config.dll`
- `logging.dll`
- parts of `server_app.dll`
- some higher-level command orchestration

Likely native-heavy:
- `platform.dll`
- `net_transport.dll`
- low-level parser/framing hot paths
- resolver if tightly coupled to OS/network behavior

---

## Build system expectations

Update the build system incrementally so it can:

- build the host EXE
- build DLLs
- publish public headers
- use clean export/import macros
- support Windows as a first-class target
- preserve developer workflow where possible

Do not break the existing build without a recovery path.

---

## Required deliverable style

When making changes:

- show concrete file changes
- explain why a boundary is being introduced
- explain why a subsystem belongs in a given module
- document known compromises
- summarize remaining legacy coupling after each phase
- record major decisions in `docs/refactor/decision-log.md`

Do not jump into blind edits without documenting reasoning.

---

## Output expectations for each major step

At the end of each major step or phase, provide:

1. what changed
2. what still depends on legacy monolith internals
3. what compiles now
4. what tests were run
5. what risks remain
6. what the next smallest safe step is

---

## Anti-patterns to avoid

Do not:

- rewrite the server from scratch
- split DLLs purely by source folder
- create trivial micro-DLLs
- expose C++ object models as ABI
- introduce hidden ownership rules
- make every tiny parser step a cross-DLL call
- over-engineer plugin discovery before the basics are stable
- claim future C# replacement is easy without showing the actual seam constraints

---

## Preferred implementation style

- precise
- conservative
- staged
- documented
- test-backed
- reversible where practical

When uncertain, choose the least risky path and explain the tradeoff.