# logging

## Responsibility
Logging sinks, formatting, severity routing, diagnostics, and optional backend fan-out.

## Non-Responsibility
Does not own config parsing, transport, or protocol state.

## Public ABI Summary
Logging initialization, sink registration, message emission, flush, and shutdown APIs.

## Lifecycle
Initialize early, accept sinks/service tables, log throughout runtime, then flush and shut down late.

## Dependencies
Depends on `core_runtime` and potentially minimal platform services.

## Owned State
Logger configuration, sink registrations, and backend state.

## Memory / Ownership Rules
Prefer caller-owned formatted buffers and explicit sink lifetime management.

## Thread-Safety
Should be safe for concurrent emission if the implementation supports it; otherwise this must be documented and serialized by the host.

## Error Model Notes
Logging should degrade gracefully and avoid aborting the process unless initialization is fatally broken.

## .NET Suitability
Good managed candidate.
Reason: policy-heavy, low callback frequency, and a natural fit for table-driven service contracts.
