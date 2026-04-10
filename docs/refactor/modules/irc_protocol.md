# irc_protocol

## Responsibility
IRC parsing, tokenization, validation, line framing, serialization, numerics, and protocol helpers.

## Non-Responsibility
Does not own transport state, config loading, or full client/channel registries.

## Public ABI Summary
Parse request lines into stable command objects and serialize replies/numerics into caller-owned buffers.

## Lifecycle
Initialize protocol tables and helpers if needed, then provide pure or mostly pure parsing/formatting services.

## Dependencies
Depends on `core_runtime`; should avoid broad dependency reach.

## Owned State
Prefer little or no mutable state beyond protocol tables and constants.

## Memory / Ownership Rules
Prefer caller-owned buffers and explicit parse result ownership rules.

## Thread-Safety
Should be reentrant or at least safe for single-threaded event-loop use without hidden globals.

## Error Model Notes
Parse failures should be reported via status/result enums with enough detail to drive numerics.

## .NET Suitability
Native first, maybe managed later.
Reason: hot-path performance and parser correctness favor native implementation, but some higher-level formatting helpers could eventually move.
