# resolver

## Responsibility
DNS, reverse DNS, ident lookup, and associated subprocess orchestration.

## Non-Responsibility
Does not own protocol parsing or channel/client state.

## Public ABI Summary
Submit lookup requests, receive results through callbacks or completed result objects, and destroy resolver contexts.

## Lifecycle
Initialize with transport/platform services, issue lookups, dispatch results, then destroy.

## Dependencies
Depends on `platform`, `core_runtime`, and logging; may also interact with transport callbacks.

## Owned State
Outstanding lookup requests and lookup completion state.

## Memory / Ownership Rules
Returned host strings or ident data must have explicit ownership and lifetime rules.

## Thread-Safety
Likely serialized by the transport/event loop; concurrency assumptions should be explicit.

## Error Model Notes
Lookup failures should be status-coded and safe to ignore when non-fatal.

## .NET Suitability
Native first, maybe managed later.
Reason: OS/network coupling and callback timing make it a cautious candidate.
