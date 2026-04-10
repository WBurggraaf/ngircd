# config

## Responsibility
Load, parse, validate, dump, and reload server configuration.

## Non-Responsibility
Does not own socket I/O, event-loop mechanics, or protocol parsing.

## Public ABI Summary
Configuration context creation, load/reload, validation, query, and destroy APIs.

## Lifecycle
Create config context, load from file, validate, expose read-only views, reload if supported, then destroy.

## Dependencies
Depends on `core_runtime` and possibly logging for diagnostics.

## Owned State
Configuration objects and parsed config collections.

## Memory / Ownership Rules
Prefer caller-owned output buffers or immutable config snapshots with explicit destruction.

## Thread-Safety
Best treated as a mostly read-only snapshot after initialization; rehash should be serialized.

## Error Model Notes
Validation failures should return status codes with optional diagnostics.

## .NET Suitability
Good managed candidate.
Reason: policy-heavy, low-level OS coupling is limited, and marshalling needs are manageable.
