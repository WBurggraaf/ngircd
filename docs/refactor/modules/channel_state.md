# channel_state

## Responsibility
Own channel registry, membership, channel modes, and channel-level derived data.

## Non-Responsibility
Does not own transport, resolver, or config file parsing.

## Public ABI Summary
Create/destroy channels, manage membership, query modes, and emit channel state through explicit APIs.

## Lifecycle
Initialize registry, create channels on demand or from config, mutate membership during IRC events, then destroy on shutdown.

## Dependencies
Depends primarily on `core_runtime`; logging only if justified.

## Owned State
Channel registry, membership lists, and channel mode structures.

## Memory / Ownership Rules
Membership and list ownership must be explicit; borrowed pointers should have documented lifetimes.

## Thread-Safety
Likely single-threaded today; avoid implying concurrency that the current server does not have.

## Error Model Notes
Join/part/mode operations should report clear status codes and possible denial reasons.

## .NET Suitability
Native first, maybe managed later.
Reason: state consistency matters more than language portability at this stage.
