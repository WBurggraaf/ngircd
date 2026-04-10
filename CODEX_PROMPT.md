# CODEX_PROMPT.md

You are working in an existing native IRC server repository.

Your task is to perform a **staged architecture refactor** that transforms the codebase from a largely monolithic application into:

- one thin host executable
- multiple subsystem DLLs
- stable versioned C ABI boundaries
- an architecture that can later allow **selected** high-level native DLLs to be replaced by C#/.NET implementations behind compatible host-facing contracts

This is **not** a rewrite.
This is **not** a greenfield redesign.
This is a behavior-preserving, implementation-driven refactor of a real existing codebase.

You must be conservative, explicit, test-aware, and highly detailed.

---

## Prime directive

Do not jump straight into code edits.

You must first inspect the codebase and produce architecture and migration documentation.
Only then should you begin phased extraction work.

If you find that the repository structure differs from assumptions, adapt the plan based on the real code, and document why.

---

## What success looks like

At the end of this work, the repo should have:

1. a thin bootstrap-oriented executable
2. DLLs organized by stable subsystem responsibilities
3. public, versioned, C ABI-based module interfaces
4. reduced cross-cutting global state
5. documented ownership rules across module boundaries
6. documented target architecture and migration rationale
7. a realistic strategy for future selective .NET replacement of some modules
8. preserved or intentionally documented behavior changes

---

## Absolute rules

### Rule 1: Do not rewrite the server
Do not replace working logic with a fresh implementation unless extraction is impossible otherwise.
Prefer:
- seam creation
- interface extraction
- file movement
- context introduction
- wrapper layers
- incremental dependency inversion

### Rule 2: All module boundaries must be C ABI
Across DLL boundaries, do not expose:
- C++ classes
- STL containers
- compiler-specific name mangling
- implicit ownership
- exceptions
- RTTI-based designs

Use instead:
- opaque handles
- function tables
- fixed-width integer types
- plain structs with version fields
- pointer + length buffers
- explicit ownership contracts
- explicit lifecycle functions

### Rule 3: Preserve behavior first
This refactor should preserve runtime behavior unless a change is necessary and documented.

### Rule 4: Keep the code buildable often
Avoid long periods of broken intermediate state.
Prefer smaller steps with checkpoints.

### Rule 5: Do docs first
Before major changes, create and populate the required docs.

### Rule 6: Be honest about .NET replaceability
Do not pretend every module can later be trivially replaced with C#.
Identify real candidates and real non-candidates.

---

## Required first actions

Before major edits, inspect the repository and create these files if they do not already exist:

- `docs/refactor/current-architecture.md`
- `docs/refactor/target-architecture.md`
- `docs/refactor/migration-plan.md`
- `docs/refactor/decision-log.md`
- `docs/refactor/dotnet-replacement-strategy.md`

Also create module docs under:
- `docs/refactor/modules/core_runtime.md`
- `docs/refactor/modules/platform.md`
- `docs/refactor/modules/config.md`
- `docs/refactor/modules/logging.md`
- `docs/refactor/modules/net_transport.md`
- `docs/refactor/modules/resolver.md`
- `docs/refactor/modules/irc_protocol.md`
- `docs/refactor/modules/client_state.md`
- `docs/refactor/modules/channel_state.md`
- `docs/refactor/modules/command_handlers.md`
- `docs/refactor/modules/server_app.md`

Do not skip this phase.

---

## Repository inspection checklist

You must inspect and document:

- program entry points
- startup/bootstrap path
- configuration load/parse path
- logging path
- socket listener setup
- event loop / polling / select model
- connection lifecycle
- resolver / reverse DNS / ident flow
- inbound line reading/framing
- IRC parsing and validation
- command dispatch
- registration/login flow
- client/user/server state
- channel state
- mode handling
- message routing
- rehash/reload flow if supported
- shutdown flow
- current build/link organization
- platform-specific branches, especially Windows
- global mutable state
- singletons
- cyclic dependencies
- shared structs crossing subsystem boundaries
- compile-time coupling hotspots
- source files that act as accidental god modules

Also produce a source-to-responsibility map.

---

## Initial target architecture

Use this as the initial target, but refine it when repository inspection proves a better split:

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

### Host EXE responsibilities
The host executable should eventually own:
- process entry
- argument parsing
- top-level bootstrap
- dynamic module loading
- module API resolution
- construction of host service tables
- module wiring
- lifecycle control
- fatal error reporting
- shutdown orchestration

The host must become thin.

---

## Dependency direction target

Try to enforce these dependency directions:

- host -> all modules
- `server_app.dll` -> high-level modules
- `command_handlers.dll` -> protocol + state + logging + service interfaces
- `client_state.dll` -> core/runtime only, plus logging only if justified
- `channel_state.dll` -> core/runtime only, plus logging only if justified
- `irc_protocol.dll` -> core/runtime only
- `resolver.dll` -> platform + core/runtime + logging
- `net_transport.dll` -> platform + core_runtime + logging
- `config.dll` -> core_runtime only
- `logging.dll` -> core_runtime only
- `platform.dll` -> minimal dependencies

If the current code violates this, document the violation before correcting it.

---

## ABI design requirements

Every DLL must expose a versioned API entrypoint such as:

- `module_get_api_v1`

or an equivalent clearly versioned export.

Every public ABI should:
- use export/import macros
- use calling convention macros
- use fixed-width integer types
- have explicit ownership rules
- avoid passing complex language-specific objects
- include version fields where appropriate
- use explicit init/start/stop/destroy lifecycle

Preferred module API shape:
- one public header per module
- one API struct containing function pointers
- module metadata struct
- opaque context handle types
- host-provided services as input

---

## Memory ownership requirements

This is mandatory.

Across DLL boundaries:
- do not assume memory allocated in one module may be freed in another unless there is an explicit shared allocator contract
- prefer caller-allocated buffers
- where callee allocation is necessary, expose a matching release path or shared allocator
- document ownership and lifetime for every public parameter and return value

You must introduce a shared allocator/service contract early in the refactor.

---

## Error handling requirements

Create or standardize a shared status/error model.

Requirements:
- shared status code enum
- documented success/failure conventions
- optional extended diagnostic path if useful
- no inconsistent public ABI return semantics across modules

For each public function, document:
- success condition
- recoverable failure behavior
- fatal failure behavior
- whether partial side effects can occur

---

## Global state reduction requirements

A major goal is to reduce uncontrolled global state.

For each major global encountered, classify it as:
- truly process-global
- host-owned singleton
- module-owned singleton
- instance-owned context
- removable legacy global

Prefer converting globals into:
- module context structs
- host-owned registries
- instance-owned state
- immutable config structures

Do not merely hide globals behind helper accessors.

---

## .NET replaceability requirements

Design seams so some high-level modules could later be implemented in C#/.NET.

However, do not make false promises.

For each module, classify it as:
- Native forever
- Native first, maybe managed later
- Good managed candidate

Use real reasons:
- performance sensitivity
- callback frequency
- marshalling complexity
- OS coupling
- threading sensitivity
- exception boundary risk
- debugging complexity

Likely better managed candidates:
- `config.dll`
- `logging.dll`
- some high-level orchestration in `server_app.dll`
- possibly some command orchestration

Likely native-first or native-forever:
- `platform.dll`
- `net_transport.dll`
- low-level parser/framing hot paths
- resolver if tightly OS-integrated

Document this realistically in `docs/refactor/dotnet-replacement-strategy.md`.

---

## Required migration phases

Unless the real code strongly suggests a safer order, follow this sequence.

### Phase 0: Inventory and guardrails
Tasks:
- inspect the codebase
- document current architecture
- create migration plan
- identify coupling hotspots
- identify major globals
- identify critical runtime paths
- add or improve baseline tests if practical
- capture baseline behavior expectations

Checkpoint output required:
- docs created
- source map created
- risk areas identified
- no reckless code churn yet

### Phase 1: Shared runtime and ABI scaffolding
Tasks:
- introduce common export/import macros
- introduce calling convention macros
- define module API versioning helpers
- define common status codes
- define allocator interface
- define common buffer/span/string view helpers if needed
- define common module metadata conventions
- define host-service-table conventions

Checkpoint output required:
- what common contracts now exist
- how they will be used by future modules
- what old coupling remains

### Phase 2: Introduce host/server seam
Tasks:
- separate top-level orchestration from monolithic entry path
- create internal seam through which startup/lifecycle can flow
- make future host/minimal-EXE direction visible without breaking behavior

Checkpoint output required:
- entry path before vs after
- remaining direct monolith startup coupling
- validation performed

### Phase 3: Extract low-risk modules first
Start with:
- `logging.dll`
- `config.dll`
- `core_runtime.dll`

Tasks:
- create public headers
- move implementation behind module boundaries
- expose versioned APIs
- keep behavior stable
- update build system

Checkpoint output required:
- exact files moved/added
- exported APIs introduced
- tests/build validation
- remaining legacy dependencies

### Phase 4: Extract platform/transport/resolver seams
Tasks:
- separate socket/event-loop behavior from IRC semantics
- create connection/event abstractions
- move resolver behind an interface
- document callback/event model

Checkpoint output required:
- transport boundary design
- risks to performance or event flow
- state/ownership changes
- validation performed

### Phase 5: Extract IRC protocol layer
Tasks:
- isolate line framing, parse, validation, serialization, numeric helpers
- separate parsing from high-level state mutation where practical
- create protocol-facing types suitable for stable ABI use

Checkpoint output required:
- parser boundary
- remaining direct dependencies on old globals/state
- tests run on registration and command flow

### Phase 6: Extract state modules
Extract:
- `client_state.dll`
- `channel_state.dll`

Tasks:
- define ownership of client/channel structures
- reduce shared mutable globals
- introduce explicit contexts/indexes/registries

Checkpoint output required:
- global state removed vs still present
- ownership model
- risks remaining

### Phase 7: Extract command handling
Tasks:
- move command dispatch and handlers behind interfaces
- make handlers depend on state/protocol/logging/service contracts, not broad monolith globals

Checkpoint output required:
- command dispatch flow before vs after
- what handlers remain coupled
- validation on core IRC flows

### Phase 8: Minimize the EXE
Tasks:
- move remaining orchestration into `server_app.dll`
- keep EXE focused on bootstrap, loading, wiring, lifecycle

Checkpoint output required:
- final host responsibilities
- final DLL graph
- remaining technical debt
- recommended next steps

---

## Build system requirements

Update the build incrementally to support:
- host EXE target
- DLL targets
- public include paths
- export/import macro usage
- predictable output layout
- Windows-first support
- temporary coexistence of legacy and modular builds if needed

Document temporary dual-build arrangements if introduced.

---

## Testing requirements

Maintain or add tests/smoke coverage for at least:
- startup/shutdown
- config load
- client connect
- registration via NICK/USER
- `CAP LS`
- `CAP REQ`
- `CAP END`
- user `MODE`
- `JOIN`
- channel `MODE` query
- `PRIVMSG` to joined channel
- disconnect/close handling
- basic numeric/error replies
- rehash/reload if supported

Also add module-level checks for:
- DLL load success
- missing export handling
- interface version mismatch handling
- module init failure handling

Prefer extending existing harnesses before creating a new test framework.

---

## Required documentation content per module

For each file under `docs/refactor/modules/`, include:
- responsibility
- non-responsibility
- public ABI summary
- lifecycle
- dependencies
- state ownership
- thread-safety assumptions
- memory/ownership rules
- error model notes
- .NET suitability classification

---

## Decision log policy

Keep `docs/refactor/decision-log.md` updated.

Record major decisions such as:
- why module boundaries were chosen
- why a subsystem remains native
- why a global could not yet be removed
- why a particular ABI shape was chosen
- why the phase sequence changed
- tradeoffs involving performance, maintainability, or future interop

---

## How you must report progress

After each significant phase or checkpoint, produce a structured update with these sections:

### Summary
What changed.

### Files changed
Exact files added, modified, moved, or deleted.

### Architecture impact
What seam, boundary, or ownership model improved.

### Remaining legacy coupling
What is still monolithic or globally coupled.

### Validation
What was built, run, or tested.
State clearly what is validated and what is still unverified.

### Risks
What may still break or needs follow-up.

### Next smallest safe step
What should happen next.

Do not hide uncertainty.
Be explicit.

---

## Anti-patterns to avoid

Do not:
- rewrite from scratch
- split DLLs purely by directory or file count
- create trivial micro-DLLs with no stable responsibility
- expose C++ object models across ABI boundaries
- create chatty per-token or per-small-parser-step DLL APIs
- bury ownership in comments without enforcing interface shape
- over-engineer plugin discovery before basic modularization works
- claim C# replacement is easy without designing the actual seam

---

## Preferred implementation style

You should behave like a careful systems engineer performing a risky live refactor:

- inspect first
- document first
- extract lowest-risk seams first
- keep behavior stable
- reduce globals intentionally
- keep interfaces explicit
- make small safe moves
- validate continuously
- explain tradeoffs

When uncertain, choose the least risky path and explain why.

---

## Final output requirements

At the end of the engagement, provide:

1. final module graph
2. list of DLLs and their responsibilities
3. list of host EXE responsibilities
4. list of public module APIs introduced
5. globals removed vs globals remaining
6. summary of behavior validated
7. summary of technical debt intentionally left
8. realistic best first candidate for future C# replacement
9. recommended next steps after this refactor

---

## Execution order reminder

Your order of work should be:

1. inspect repository
2. produce/refine architecture docs
3. define migration plan based on real code
4. add shared ABI/runtime contracts
5. introduce host/server seam
6. extract low-risk DLLs
7. continue phased extraction
8. minimize executable
9. summarize architecture and remaining risks

Do not skip to step 6.