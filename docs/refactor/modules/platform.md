# platform

## Responsibility
OS-facing primitives such as files, process helpers, socket-adjacent helpers, and platform-specific glue.

## Non-Responsibility
Does not own IRC semantics, protocol parsing, or state machines.

## Public ABI Summary
Low-level platform service table for host and native modules.

## Lifecycle
Initialize early, remain available throughout runtime, and shut down last or near last.

## Dependencies
Should remain minimal and avoid depending on higher-level server modules.

## Owned State
Platform service hooks and process/OS-specific helper state where needed.

## Memory / Ownership Rules
Prefer caller-owned buffers and explicit close/destroy functions.

## Thread-Safety
Must be safe for use from event-loop and bootstrap paths; avoid hidden mutable global assumptions.

## Error Model Notes
Return status codes and explicit OS error information where useful.

## .NET Suitability
Native forever.
Reason: heavy OS coupling, low-level performance, and callback sensitivity.
