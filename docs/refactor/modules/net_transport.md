# net_transport

## Responsibility
Listeners, connection lifecycle, event loop integration, socket I/O, and transport-level state.

## Non-Responsibility
Does not own high-level IRC parsing or command semantics.

## Public ABI Summary
Create/destroy transport contexts, attach listeners, drive dispatch, and surface connection events through callbacks.

## Lifecycle
Initialize with host services, create listeners, enter dispatch loop, stop on host request, then destroy.

## Dependencies
Depends on `platform`, `core_runtime`, and logging.

## Owned State
Listener sets, connection registry, and transport-level per-connection data.

## Memory / Ownership Rules
Transport should own its internal connection structures; host-visible data should be copied or referenced with explicit lifetime rules.

## Thread-Safety
Likely single-threaded event-loop oriented today; that assumption should be explicit.

## Error Model Notes
Should use status codes and callback error reports rather than cross-module exceptions.

## .NET Suitability
Native forever.
Reason: callback frequency, socket timing, and OS integration make this a poor managed replacement candidate.
