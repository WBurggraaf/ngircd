# .NET Replacement Strategy

## Purpose
Define which future modules could realistically be replaced by C#/.NET and which should stay native.

## Good Managed Candidates
- `config.dll`
- `logging.dll`
- Parts of `server_app.dll`

### Why
- These modules are policy-heavy and usually not on the hottest path.
- Their work is mostly about orchestration, formatting, validation, and service coordination.
- They tend to have simpler marshalling needs than transport or parser code.

## Native First, Maybe Managed Later
- Some high-level pieces of `command_handlers.dll`
- Some orchestration pieces of `server_app.dll`

### Why
- They may be suitable once the ABI and service tables are stable.
- They still interact with state and protocol behavior often enough that replacement should be cautious.

## Native Forever
- `platform.dll`
- `net_transport.dll`
- Low-level `irc_protocol.dll` hot-path helpers
- `resolver.dll` if the implementation remains tightly coupled to OS/network behavior

### Why
- These areas are callback-heavy, latency-sensitive, or deeply tied to socket and process APIs.
- Cross-language reverse callbacks and exception boundaries increase risk in these paths.
- Marshalling cost matters more here than in configuration or logging.

## Interop Options
- Use unmanaged exports with `extern "C"` entrypoints.
- Keep host-visible APIs table-driven instead of object-model-based.
- Prefer explicit buffers and lengths for strings and serialized data.
- Use opaque handles for stateful objects.

## Host/CLR Interop Considerations
- The host can load managed implementations behind the same exported ABI if the managed runtime is hosted carefully.
- The first managed candidate should be a low-callback, low-frequency module such as `logging.dll` or `config.dll`.
- Managed modules must not throw exceptions across the ABI boundary.

## Reverse Callback Risks
- Reverse callbacks from managed code into the host increase complexity and failure modes.
- Callbacks must stay explicit, versioned, and minimal.
- Avoid callback-heavy modules for initial replacement attempts.

## Exception Containment
- Exceptions must be caught inside managed boundaries and converted to status codes.
- Native callers must never need to reason about managed exception semantics.

## Marshalling Guidance
- Use caller-provided UTF-8 or byte buffers where possible.
- Avoid shared mutable string ownership across the boundary.
- Prefer fixed-width integers and explicit lengths.
- Do not pass STL or CLR object graphs through the ABI.

## Threading Risks
- Managed runtime initialization and shutdown must be ordered carefully.
- Long-lived worker threads owned by the transport layer are poor managed replacement targets.
- Modules with frequent cross-thread callbacks should remain native unless a strong case exists.

## Recommended First Managed Candidate
- `logging.dll`

### Why
- It is policy-heavy, relatively low-risk, and naturally service-table driven.
- Logging can often be replaced with a stable buffer-and-sink contract without affecting transport timing.
