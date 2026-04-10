# Migration Plan

## Goal
Refactor the current monolith into a thin host plus responsibility-focused DLLs without breaking existing behavior.

## Phase 0: Inventory And Guardrails
- Document current architecture and target architecture.
- Map source files to runtime responsibilities.
- Inventory globals, coupling hotspots, and runtime flows.
- Maintain existing tests and add narrow coverage only where it improves confidence.

## Phase 1: Shared Runtime And ABI Scaffolding
- Introduce shared ABI headers for versioning, status codes, allocator hooks, buffer types, and module metadata.
- Add export/import and calling convention macros.
- Define service-table patterns for host-to-module communication.
- Keep the monolith build working while the new contracts are introduced.

## Phase 2: Host/Server Seam
- Create an internal orchestration seam between bootstrap and runtime logic.
- Move top-level lifecycle decisions into a dedicated server application layer.
- Keep the executable thin but still behaviorally equivalent.

## Phase 3: Low-Risk Extraction
- Extract `logging.dll`, `config.dll`, and `core_runtime.dll` first.
- Use the new ABI scaffolding so the host or current app can consume them without C++ ABI leakage.
- Preserve current logging and configuration semantics.

## Phase 4: Platform, Transport, Resolver
- Isolate OS/platform code.
- Separate raw transport from higher-level IRC behavior.
- Introduce explicit connection and resolver callbacks.
- Minimize event-loop hot-path chatter across module boundaries.

## Phase 5: IRC Protocol Layer
- Isolate parsing, validation, framing, numerics, and serialization helpers.
- Keep state mutation separate where practical.
- Stabilize protocol-facing data shapes.

## Phase 6: State Modules
- Extract `client_state.dll` and `channel_state.dll`.
- Move ownership of client and channel state behind explicit module contexts or registries.
- Reduce direct dependence on globals and hidden singleton assumptions.

## Phase 7: Command Handling
- Route command dispatch through explicit service interfaces.
- Reduce direct reach into configuration, transport, and state internals.
- Preserve core command behavior and numerics.

## Phase 8: Minimize Executable
- Move remaining orchestration into `server_app.dll`.
- Keep `host.exe` focused on bootstrap, loading, wiring, and lifecycle control.

## Validation Strategy
- Re-run existing unit and white-box tests after each significant seam.
- Add module-load/version mismatch tests once the first DLL exports exist.
- Keep the baseline config and protocol flows covered throughout the refactor.

## Risks
- Listener and event-loop changes can break timing-sensitive behavior.
- Resolver and ident flow may be tightly coupled to connection lifetime.
- Configuration reload needs careful ownership handling to avoid use-after-free.
- Channel and client state extraction can create subtle aliasing bugs if context ownership is not explicit.

## Next Safe Step
- Add shared runtime contracts first, because every later DLL depends on them.
