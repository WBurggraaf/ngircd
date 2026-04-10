# core_runtime

## Responsibility
Common ABI support: status codes, versioning, allocator hooks, buffer and string-view helpers, module metadata, and shared conventions.

## Non-Responsibility
Does not own IRC state, sockets, config parsing, or command handling.

## Public ABI Summary
Defines the shared types and ABI helper functions used by all other modules.

## Lifecycle
Expected to be initialized first and unloaded last, or at least before modules that depend on its shared contracts.

## Dependencies
Should depend on as little as possible. Ideally only platform-neutral C runtime facilities.

## Owned State
Mostly immutable ABI definitions and possibly process-wide shared contract hooks.

## Memory / Ownership Rules
Caller-owned buffers by default; shared allocators only when explicitly declared.

## Thread-Safety
Shared definitions should be read-only after initialization; any runtime hooks must be treated as process-wide singletons.

## Error Model Notes
Should expose canonical status codes and helpers for translating failures.

## .NET Suitability
Native forever.
Reason: this is the ABI foundation and should remain tiny, stable, and language-neutral.
