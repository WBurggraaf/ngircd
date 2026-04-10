# CODEX_PROMPT.md

You are working in an existing native IRC server repository that is already partway through a modularization effort.

Your job is to push it to the **fastest credible thin-host + DLL architecture** without rewriting the server.

The most important practical outcomes are:

- real DLL usage, not placeholder modularity
- a smaller `ngircd.exe`
- stable C ABI contracts
- future compatibility with selected C#/.NET replacements behind those contracts

---

## Repository reality you must respect

Assume these facts unless the code proves otherwise:

- `core_runtime.dll` already exists in the build output
- most other intended modules are still static libraries
- the executable still compiles much of the original monolithic source directly
- wrapper modules under `include/` and `src/` are real scaffolding, but not yet the final runtime architecture
- the correct public export pattern is `module_get_api_v1`

Do not describe the repo as fully modular if the build still links it monolithically.

---

## Prime directive

Do the minimum structural work necessary to reach a **real module-loaded runtime** quickly.

Prefer:

- promoting existing module scaffolding into actual shared libraries
- moving orchestration out of the EXE first
- keeping low-level hot paths native
- delaying deep subsystem purity until after the first DLL wave works

Avoid:

- speculative abstraction layers
- unnecessary source rewrites
- early over-splitting into too many DLLs
- pretending low-level transport code is a good first .NET target

---

## Priority order

### Priority 1

Make `ngircd.exe` thin.

The executable should own only:

- process entry
- argument forwarding
- bootstrap
- module loading and API resolution
- lifecycle control
- fatal startup failure handling

### Priority 2

Turn the first module wave into real DLLs.

Fastest likely first wave:

- `core_runtime`
- `logging`
- `config`
- `server_app`
- then `platform`, `resolver`, `net_transport`

### Priority 3

Route behavior through versioned C ABI tables.

Use:

- export/import macros
- calling convention macros
- fixed-width integers
- opaque handles
- explicit ownership rules
- host-provided service tables

### Priority 4

Only then push deeper into protocol/state extraction.

Do not start with the hardest split if the simpler DLL wins are available.

---

## Required design stance

### Keep hot paths conservative

These should remain native-first:

- socket loops
- low-level I/O
- event/poll/select logic
- platform shims
- most resolver internals

### Be honest about .NET replaceability

Likely future C# candidates:

- config
- logging contract/front-end
- command dispatch/orchestration
- protocol validation helpers
- selected server_app orchestration

Likely poor early C# candidates:

- net transport internals
- platform shims
- timing-sensitive resolver pieces
- tightly-coupled low-level state mutation loops

---

## Mandatory ABI rules

Every public DLL boundary must:

- be C ABI only
- avoid compiler-specific language objects
- use versioned exports
- avoid implicit memory ownership
- avoid CRT allocator mismatch across modules
- document lifetime, ownership, and threading assumptions

Preferred shape:

- one module metadata struct
- one API function table
- one `module_get_api_v1` export
- caller-owned buffers whenever possible
- explicit create/init/start/stop/destroy lifecycle

---

## What to optimize for

Optimize for this order:

1. preserve behavior
2. get to actual DLL loading fast
3. make the EXE small
4. reduce compile-time coupling
5. improve future managed replaceability
6. improve architectural elegance after the runtime shape is real

---

## If you need to choose between two refactor options

Choose the option that:

- removes logic from the EXE sooner
- produces a real DLL sooner
- keeps the ABI simpler
- preserves native hot-path behavior
- reduces future rewrite risk

---

## Definition of a good next patch

A good next patch does at least one of the following:

- converts an existing module target into a real shared library
- replaces EXE-owned subsystem behavior with host/module orchestration
- standardizes ABI/service-table contracts
- makes a future C# substitution boundary more realistic

A bad next patch adds abstractions without increasing actual DLL usage.

