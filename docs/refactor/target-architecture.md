# Target Architecture

## Overview
The target shape is a thin host executable that loads subsystem DLLs with versioned C ABIs. The host owns process entry, module loading, lifecycle orchestration, and fatal failure handling. Functional code moves into responsibility-focused modules.

## Target Module Set
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

## Module Responsibilities
- `host.exe`: process entry, argument parsing, bootstrap, module loading, contract validation, service-table wiring, lifecycle control, shutdown.
- `core_runtime.dll`: shared ABI types, status codes, versioning helpers, allocator contract, buffer/span helpers, ownership conventions.
- `platform.dll`: OS-facing helpers, file/process primitives, socket and platform integration utilities.
- `config.dll`: config loading, validation, config object creation, config reload support.
- `logging.dll`: logging sinks, formatting, routing, and diagnostics.
- `net_transport.dll`: listener creation, connection lifecycle, event-loop integration, socket I/O.
- `resolver.dll`: DNS, reverse DNS, ident, and lookup orchestration.
- `irc_protocol.dll`: IRC parsing, framing, validation, serialization, numerics, and command object construction.
- `client_state.dll`: client identity, registration state, and client registry ownership.
- `channel_state.dll`: channel ownership, membership, and channel registry state.
- `command_handlers.dll`: command dispatch and high-level IRC command behavior.
- `server_app.dll`: application-level orchestration, startup sequencing, policy wiring, and module cooperation.

## Dependency Direction
- `host.exe` depends on all modules.
- `server_app.dll` depends on high-level service contracts and orchestrates the rest.
- `command_handlers.dll` depends on protocol, state, logging, and service interfaces.
- `client_state.dll` and `channel_state.dll` depend primarily on `core_runtime.dll`, with logging only if justified.
- `irc_protocol.dll` depends on `core_runtime.dll`.
- `resolver.dll` depends on `platform.dll`, `core_runtime.dll`, and logging.
- `net_transport.dll` depends on `platform.dll`, `core_runtime.dll`, and logging.
- `config.dll` depends on `core_runtime.dll`.
- `logging.dll` depends on `core_runtime.dll`.
- `platform.dll` should stay minimal and avoid broad reverse dependencies.

## Host Responsibilities
- Resolve module paths and load DLLs.
- Query `module_get_api_v1` or equivalent exports.
- Validate API and metadata versions.
- Construct host services and shared allocator/log sinks.
- Wire modules together in a controlled order.
- Handle startup failures and clean shutdown.

## ABI Shape
- Each module exports a versioned entrypoint.
- Public ABI uses `extern "C"`, fixed-width integer types, opaque handles, explicit lifecycles, and caller-owned buffers where possible.
- No C++ classes, STL types, exceptions, or hidden allocation contracts cross DLL boundaries.
- API structs contain version fields and function tables.

## Ownership Model
- Shared ownership is explicit and documented per API.
- Host-provided allocators are preferred for cross-module allocations.
- Caller-owned buffers are the default for returned strings and serialized data.
- If a module allocates, it also exposes the matching free/release function or uses a shared allocator service.

## Runtime Flow
1. `host.exe` loads `core_runtime.dll`, `platform.dll`, and `logging.dll` first.
2. The host constructs service tables and allocator hooks.
3. The host loads `config.dll`, `client_state.dll`, `channel_state.dll`, `irc_protocol.dll`, `resolver.dll`, `net_transport.dll`, and `command_handlers.dll`.
4. `server_app.dll` receives the loaded service contracts and coordinates startup.
5. The host starts the server application and then only supervises lifecycle and shutdown.

## Why This Split
- `config.dll` and `logging.dll` are relatively low-risk, policy-heavy, and good early seams.
- `platform.dll` and `net_transport.dll` remain native-heavy because of OS coupling, performance, and callback frequency.
- `client_state.dll` and `channel_state.dll` deserve separate ownership because they currently serve as the core mutable state model.
- `irc_protocol.dll` should isolate parser and serialization behavior before state mutation is further disentangled.
- `server_app.dll` gives the host a place to stop carrying application policy.

## Future .NET Replaceability
- Good managed candidates: `config.dll`, `logging.dll`, and parts of `server_app.dll`.
- Native first, maybe managed later: some orchestration inside `command_handlers.dll`.
- Native forever for now: `platform.dll`, `net_transport.dll`, and low-level hot-path protocol helpers.
- Managed replacement only becomes realistic where callback frequency is low, marshalling is simple, and fault isolation is acceptable.
