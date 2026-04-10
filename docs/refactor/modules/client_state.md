# client_state

## Responsibility
Own client identity, registration state, client registry queries, and client-level derived data.

## Non-Responsibility
Does not own socket transport or config file parsing.

## Public ABI Summary
Create/destroy client contexts, query and mutate client state, and enumerate clients through explicit APIs.

## Lifecycle
Initialize client registry, create clients as connections are accepted or learned, update state through login flow, then destroy on disconnect or shutdown.

## Dependencies
Depends primarily on `core_runtime`; logging only if justified.

## Owned State
Client registry, identity objects, and registration-related state.

## Memory / Ownership Rules
Clients should own their internal storage; public getters should return caller-safe views or explicit borrowed pointers with documented lifetime.

## Thread-Safety
Likely single-threaded or host-serialized today; if not, this must be stated clearly.

## Error Model Notes
State transitions should be status-coded and explicit about partial updates.

## .NET Suitability
Native first, maybe managed later.
Reason: state ownership is central to behavior, so it should be stabilized natively first.
