# TASK.md

## Title

Refactor the IRC server into a thin host EXE plus subsystem DLLs with stable C ABI seams and future-selective .NET replaceability.

---

## Objective

Restructure the current native IRC server codebase into:

- one thin executable for bootstrap/lifecycle/module loading
- multiple subsystem DLLs
- stable versioned C ABI boundaries
- reduced global state and reduced compile-time coupling
- an architecture that could later support replacing selected high-level native DLLs with C#/.NET implementations behind compatible contracts

This is a **behavior-preserving staged refactor**, not a greenfield rewrite.

---

## Success criteria

The work is successful only if all of the following are true:

1. The application still performs the baseline IRC flows correctly.
2. The executable becomes thin and orchestration-focused.
3. Major subsystems are separated behind DLL boundaries.
4. Cross-module APIs use stable, versioned C ABI contracts.
5. Memory ownership across module boundaries is explicit and safe.
6. Global state is reduced and cataloged.
7. Module dependencies are cleaner and more directional than before.
8. Documentation explains both the current and target architecture.
9. The design identifies realistic future C# replacement candidates.
10. The repository remains understandable and maintainable after the refactor.

---

## Constraints

- Preserve runtime behavior unless change is necessary and documented.
- Do not convert anything to C# in this task.
- Do not expose C++-specific ABI at module boundaries.
- Do not assume every DLL can later be replaced by a C# DLL.
- Do not introduce speculative abstractions without near-term value.
- Keep Windows support first-class.
- Keep the code building as often as possible during the refactor.

---

## Required initial work

Before major edits, inspect the repository and create:

- `docs/refactor/current-architecture.md`
- `docs/refactor/target-architecture.md`
- `docs/refactor/migration-plan.md`
- `docs/refactor/decision-log.md`
- `docs/refactor/dotnet-replacement-strategy.md`

Also create module documentation stubs for:

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

Do not skip the analysis and documentation stage.

---

## Repository inspection checklist

Inspect and document:

- executable entry points
- startup/bootstrap path
- config parsing/loading path
- logging subsystem
- socket/listener creation
- event loop implementation
- connection management
- resolver/DNS/ident path
- line reading/framing
- IRC parser/validator
- command dispatch
- registration flow
- state ownership for users/servers/channels
- mode handling
- message routing
- reload/rehash behavior
- shutdown sequence
- current build/link organization
- Windows-specific concerns
- major global data structures
- compile-time dependency hotspots
- cyclic dependencies

Also produce a source-to-responsibility map.

---

## Target module set

Use this target module set unless inspection shows a substantially better decomposition:

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

---

## Required architectural rules

### Rule 1: Stable C ABI only
All DLL boundaries must be expressible via:
- opaque handles
- plain structs with version fields
- fixed-width scalar types
- pointer+length buffers
- explicit callbacks/service tables
- explicit ownership rules

### Rule 2: Explicit module lifecycle
Every module must define a lifecycle such as:
- query API
- create/init
- start
- stop
- destroy

### Rule 3: No ambiguous ownership
For each public function, document:
- who owns input memory
- who owns output memory
- who frees returned allocations
- lifetime guarantees
- behavior on failure

### Rule 4: Versioned interfaces
Every module API must be versioned.
The host must validate interface versions when loading modules.

### Rule 5: Directional dependencies
Avoid tangled peer-to-peer coupling.
Prefer host-wired service tables and dependency inversion.

### Rule 6: Incremental extraction
Extract low-risk modules first.
Do not attempt to split transport, parser, state, and handlers all at once.

---

## Suggested directory structure

Public API headers:

- `include/core_runtime/...`
- `include/platform/...`
- `include/config/...`
- `include/logging/...`
- `include/net_transport/...`
- `include/resolver/...`
- `include/irc_protocol/...`
- `include/client_state/...`
- `include/channel_state/...`
- `include/command_handlers/...`
- `include/server_app/...`

Private implementation sources:

- `src/core_runtime/...`
- `src/platform/...`
- `src/config/...`
- `src/logging/...`
- `src/net_transport/...`
- `src/resolver/...`
- `src/irc_protocol/...`
- `src/client_state/...`
- `src/channel_state/...`
- `src/command_handlers/...`
- `src/server_app/...`
- `src/host/...`

Adjust only if repository conventions require a nearby equivalent.

---

## Required common contracts to introduce early

Create shared contracts early, likely under `include/core_runtime/` or equivalent:

- export/import macros
- calling convention macros
- API version macros
- common status code enum
- allocator interface
- buffer/span/string-view structs
- common boolean/result helpers
- logging sink/service contract
- module metadata struct
- module API retrieval convention

These contracts should be available before major DLL extraction starts.

---

## Required migration phases

### Phase 0: Baseline and inventory
Deliverables:
- architecture docs
- migration plan
- source map
- baseline tests/smoke paths
- risk register entries in decision log

Acceptance criteria:
- current architecture is documented
- critical flows are identified
- no large behavioral edits yet

### Phase 1: Shared runtime and ABI scaffolding
Deliverables:
- common ABI headers
- status model
- allocator contract
- export/version macros
- initial service-table pattern

Acceptance criteria:
- monolith still builds
- shared contracts exist
- no major behavior regressions

### Phase 2: Introduce host/server seam
Deliverables:
- internal seam for startup/lifecycle
- thin bootstrap path begins to emerge
- top-level orchestration boundary documented

Acceptance criteria:
- startup can flow through the new seam
- behavior is preserved

### Phase 3: Extract low-risk DLLs
Extract:
- `logging.dll`
- `config.dll`
- `core_runtime.dll`

Acceptance criteria:
- modules build as DLLs
- host or current app consumes their C ABI
- tests still pass

### Phase 4: Isolate platform/transport/resolver
Deliverables:
- socket/event abstractions
- connection lifecycle seam
- resolver abstraction

Acceptance criteria:
- transport no longer owns high-level IRC logic
- resolver interactions are interface-based

### Phase 5: Extract protocol layer
Deliverables:
- parser/validator module
- serializer/numeric helpers
- clearer parsed command objects or equivalent structures

Acceptance criteria:
- protocol logic is more isolated
- state mutation is not hidden inside parsing where avoidable

### Phase 6: Extract state ownership
Extract:
- `client_state.dll`
- `channel_state.dll`

Acceptance criteria:
- client/channel ownership is clearer
- relevant globals are reduced or encapsulated into contexts

### Phase 7: Extract command handlers
Deliverables:
- command dispatch uses service interfaces
- handlers depend on protocol/state/logging/transport contracts, not broad globals

Acceptance criteria:
- main IRC command flow still works
- coupling is reduced

### Phase 8: Minimize executable
Deliverables:
- host owns bootstrap and wiring only
- remaining orchestration lives in `server_app.dll`

Acceptance criteria:
- EXE is thin
- module graph is documented
- remaining technical debt is explicitly listed

---

## Mandatory module documentation content

For each module doc under `docs/refactor/modules/`, include:

- responsibility
- non-responsibility
- public ABI summary
- lifecycle
- dependencies
- owned state
- memory/ownership rules
- thread-safety expectations
- error model notes
- .NET replacement suitability classification

Use one of:
- Native forever
- Native first, maybe managed later
- Good managed candidate

Explain why.

---

## .NET replacement strategy deliverable

`docs/refactor/dotnet-replacement-strategy.md` must include:

- realistic future managed candidates
- likely native-only modules
- why each module falls into that category
- Windows interop options
- unmanaged export strategy considerations
- host-CLR interop considerations
- callback/reverse-PInvoke risks
- exception containment rules
- marshalling guidance for strings and buffers
- threading risks
- recommended first managed replacement candidate

Be realistic and specific.

---

## Testing plan requirements

Maintain or add automated coverage for at least:

- startup/shutdown
- configuration load
- loopback connect
- registration via `NICK` + `USER`
- `CAP LS`
- `CAP REQ`
- `CAP END`
- user mode change
- join channel
- channel mode query
- `PRIVMSG` to joined channel
- disconnect/close handling
- error/numeric generation
- rehash/reload if supported

Add module-level checks for:
- successful DLL load
- API version mismatch handling
- missing symbol handling
- initialization failure handling

Where possible, create narrow smoke tests for extracted modules.

---

## Build system tasks

Update the build so that it can:

- build the host executable
- build each DLL
- export/import symbols cleanly
- package or place artifacts predictably for local runs
- publish public headers
- support incremental migration without needlessly breaking legacy targets

Document any temporary dual-build period if both monolith and modular targets coexist.

---

## Required decision log entries

Record major decisions including:

- why module boundaries were chosen
- why a subsystem stayed native
- why an ABI was shaped a certain way
- why a global remained temporarily
- why an extraction sequence changed
- tradeoffs involving performance, maintainability, or future interop

Keep `docs/refactor/decision-log.md` updated.

---

## Output format during execution

For each significant step, provide:

### Summary
- what changed

### Files changed
- exact files added/modified/moved

### Rationale
- why this change is safe and necessary

### Remaining legacy coupling
- what still depends on monolith internals

### Validation
- what was built/tested
- what passed
- what remains unverified

### Next step
- smallest safe next move

---

## Anti-goals

This task fails if you:

- rewrite the application wholesale
- break baseline behavior without documenting and justifying it
- expose C++ ABI across modules
- introduce unclear ownership
- make hot-path module interactions excessively chatty
- split modules purely by folder/file names rather than responsibility
- hand-wave future C# replacement without designing real seams

---

## Final deliverables

At the end, provide:

1. final module graph
2. summary of extracted DLLs
3. summary of remaining monolith residue
4. list of globals eliminated vs still present
5. status of tests
6. major architectural tradeoffs
7. best first candidate for future C# replacement
8. recommended next steps after this refactor

---

## Execution directive

Proceed cautiously, document before extracting, preserve behavior, and optimize for maintainable long-term modularity rather than the fastest superficial split.