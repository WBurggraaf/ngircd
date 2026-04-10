# command_handlers

## Responsibility
Dispatch IRC commands and implement high-level command behavior using protocol, state, logging, and service contracts.

## Non-Responsibility
Does not own raw socket I/O, config parsing, or low-level parser mechanics.

## Public ABI Summary
Register handlers, process parsed commands, and drive state transitions through narrow interfaces.

## Lifecycle
Initialize with service tables, process requests during runtime, and destroy cleanly during shutdown.

## Dependencies
Depends on `irc_protocol`, `client_state`, `channel_state`, `logging`, and relevant services.

## Owned State
Handler registry and any lightweight orchestration state.

## Memory / Ownership Rules
Commands should carry explicit ownership and buffer lifetime rules from protocol inputs through handler outputs.

## Thread-Safety
Probably serialized by the event loop; if not, concurrency must be made explicit.

## Error Model Notes
Handler failures should map to explicit status codes and numerics, not hidden side effects.

## .NET Suitability
Native first, maybe managed later.
Reason: it is a plausible orchestration candidate but still tied to protocol and state semantics.
