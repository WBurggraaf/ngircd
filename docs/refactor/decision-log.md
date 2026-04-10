# Decision Log

## 2026-04-10: Current Architecture Documented First
- Decision: defer code extraction until the current runtime and build shape are documented.
- Rationale: the codebase is still monolithic, and module boundaries should follow actual ownership and runtime flow rather than folder names.
- Tradeoff: no immediate code churn, but much lower risk of choosing bad seams.

## 2026-04-10: Target Module Set Kept Close To Prompt
- Decision: retain the target set `core_runtime`, `platform`, `config`, `logging`, `net_transport`, `resolver`, `irc_protocol`, `client_state`, `channel_state`, `command_handlers`, and `server_app`.
- Rationale: the repository already contains natural hotspots that map to those responsibilities.
- Tradeoff: some modules remain broad at first, but this avoids speculative over-splitting.

## 2026-04-10: Windows Support Remains First-Class
- Decision: keep the refactor Windows-first in build and ABI planning.
- Rationale: the repo already contains Win32 compatibility layers and a Windows-oriented build.
- Tradeoff: some abstractions will be shaped around Windows DLL loading and C ABI conventions, even where POSIX origins are visible.

## 2026-04-10: Native Modules Only For Now
- Decision: do not introduce managed code during this phase.
- Rationale: the task is staged refactoring, not language migration.
- Tradeoff: the .NET replacement path is only a design seam at this stage, not an implementation seam.

## 2026-04-10: Low-Risk Extraction Order
- Decision: prioritize `logging.dll`, `config.dll`, and `core_runtime.dll` before transport or parser work.
- Rationale: they are policy-heavy, lower-frequency modules and better early candidates for a stable ABI.
- Tradeoff: the most visible architecture change does not happen first, but the risk of breaking hot paths is much lower.

## 2026-04-10: Read-Only Config Queries Move Behind `config_api`
- Decision: expose read-only config queries through `config_api` before moving parsing or storage ownership.
- Rationale: protocol identification and later module decisions should read from a module boundary instead of direct globals where practical.
- Tradeoff: the legacy globals still exist, but new code can start depending on the API instead of the monolith internals.

## 2026-04-10: PID File Path Moves Behind `config_api`
- Decision: route PID file creation and deletion through a config getter instead of direct `Conf_PidFile` access.
- Rationale: PID file handling is bootstrap policy, and the host should consume config data through the module boundary.
- Tradeoff: the config module still owns the storage, but the host no longer needs to know the backing global name.

## 2026-04-10: Chroot and Identity Settings Move Behind `config_api`
- Decision: route chroot, UID, and GID reads through config getters in the daemon bootstrap path.
- Rationale: privilege and filesystem boundary decisions are configuration, not host policy, so the host should read them through the module API.
- Tradeoff: the underlying globals remain in the legacy config module, but the bootstrap path now depends on a smaller ABI surface.

## 2026-04-10: Bootstrap Snapshot Added To `config_api`
- Decision: replace several bootstrap-oriented getters with one versioned snapshot call.
- Rationale: the host and `server_app` should consume a coherent config view instead of repeatedly querying individual globals.
- Tradeoff: the config ABI is slightly larger, but it is now easier to evolve toward an owned context object later.

## 2026-04-10: Config Test Path Routed Through `config_api`
- Decision: have the host call the config module's `test` function instead of reaching into `Conf_Test` directly.
- Rationale: config validation is part of the module contract and should use the same ABI as other config operations.
- Tradeoff: the module API remains a thin bridge, but the host no longer needs the legacy symbol.

## 2026-04-10: Host Caches Config API Handle During Bootstrap
- Decision: cache `config_get_api_v1()` once in `main()` instead of re-querying it for the config-test path.
- Rationale: this keeps bootstrap wiring consistent and avoids redundant API lookups in a hot startup path.
- Tradeoff: minimal, but it makes the host entry flow a little easier to follow.

## 2026-04-10: `server_app` Uses `config_api->init()`
- Decision: have `server_app` initialize configuration through `config_api->init()` rather than calling `Conf_Init()` directly.
- Rationale: the bootstrap module should depend on the config ABI, not the storage implementation.
- Tradeoff: the config bridge remains thin, but the host-side orchestration is one step less coupled to legacy headers.

## 2026-04-10: `server_app` Uses `logging_api`
- Decision: have `server_app` initialize and reinitialize logging through `logging_api` instead of direct `Log_Init`/`Log_ReInit` calls.
- Rationale: logging is already exposed as a bridge module, so bootstrap should consume that boundary consistently.
- Tradeoff: the logging bridge remains a thin adapter, but host orchestration no longer needs direct logging symbols for setup.

## 2026-04-10: PID File Logging Routed Through `logging_api`
- Decision: have `server_app` send PID file create/delete diagnostics through `logging_api` instead of direct `LogDebug`/`Log` calls.
- Rationale: the module already owns logging lifecycle, so it should also own the corresponding diagnostics path.
- Tradeoff: error reporting is a little more indirect, but the bootstrap module no longer depends on the legacy logging header.

## 2026-04-10: Runtime Loop Setup Moves Behind `server_app`
- Decision: have `server_app` own IO library initialization, signal setup, and the core runtime loop bootstrap.
- Rationale: those steps are part of module-owned lifecycle, not host policy, and they belong beside the other bootstrap hooks already in `server_app`.
- Tradeoff: `server_app` grows, but `main()` continues shrinking toward a thin coordinator.

## 2026-04-10: Main Run Loop Routed Through `server_app`
- Decision: have `main()` invoke a `server_app` run-loop hook instead of calling `Conn_Handler()` directly.
- Rationale: the host should not own the main runtime loop once the server application module exists as the orchestration boundary.
- Tradeoff: the loop still executes the legacy connection handler, but the ownership of that call has moved to the module layer.

## 2026-04-10: `server_app` Uses `net_transport_api`
- Decision: have `server_app` initialize and shut down the IO layer through `net_transport_api` instead of calling the legacy IO functions directly.
- Rationale: transport lifecycle is already modeled as a module boundary, so bootstrap should consume that ABI.
- Tradeoff: the transport bridge remains a thin adapter over `io`, but the host-side orchestration no longer needs the legacy IO symbols.

## 2026-04-10: Socket Drain Exposed Via `net_transport_api`
- Decision: expose `Conn_CloseAllSockets()` through the transport module so teardown can explicitly drain sockets before shutdown.
- Rationale: the transport boundary should own connection lifecycle primitives that are still needed during teardown.
- Tradeoff: the transport ABI grows slightly, but shutdown sequencing is clearer and less coupled to the legacy connection header.

## 2026-04-10: Login and Close Primitives Exposed Via `net_transport_api`
- Decision: expose `Conn_StartLogin()` and `Conn_Close()` through the transport module as explicit lifecycle primitives.
- Rationale: login kickoff and connection close are still part of transport/runtime ownership and are needed before deeper protocol separation.
- Tradeoff: the transport ABI becomes a little broader, but the connection hotspot is now moving behind a narrower interface instead of directly inside `server_app`.

## 2026-04-10: Listener Teardown Exposed Via `net_transport_api`
- Decision: expose `Conn_ExitListeners()` through the transport module so listener teardown stays in the same lifecycle boundary as listener init.
- Rationale: init and shutdown should be symmetrical at the transport boundary.
- Tradeoff: the ABI grows again, but teardown ownership is clearer and remains in the transport layer instead of the host.

## 2026-04-10: Connection Password Accessors Exposed Via `net_transport_api`
- Decision: expose `Conn_Password()` and `Conn_SetPassword()` through the transport module as small connection-state accessors.
- Rationale: these are basic connection state operations and are a reasonable next seam for reducing direct `conn` coupling.
- Tradeoff: the transport ABI widens further, but the connection subsystem is becoming reachable through one module contract instead of direct headers.

## 2026-04-10: Connection Counters Exposed Via `net_transport_api`
- Decision: expose the basic connection counters through the transport module as read-only metrics.
- Rationale: these counters are low-risk, widely useful state and are a good fit for a small transport-facing seam.
- Tradeoff: the ABI gets a bit broader, but the metrics are now available without direct connection-header access.

## 2026-04-10: More `conn` Accessors Exposed Via `net_transport_api`
- Decision: expose `Conn_GetFromProc()`, `Conn_GetClient()`, `Conn_GetProcStat()`, `Conn_UsesSSL()`, `Conn_GetCertFp()`, `Conn_GetIPAInfo()`, `Conn_SetCertFp()`, `Conn_GetCipherInfo()`, `Conn_GetAuthPing()`, and `Conn_SetAuthPing()` through the transport ABI.
- Rationale: these are still low-risk accessors that higher-level modules may need without directly depending on the connection monolith.
- Tradeoff: the transport ABI grows further, but the remaining `conn` work is now concentrated in the hot behavioral paths instead of accessor plumbing.

## 2026-04-10: `NGIRCd_Init` Reads Config Snapshot
- Decision: have the daemon bootstrap logic consume `config_bootstrap_t` for chroot and identity settings instead of raw `Conf_*` reads.
- Rationale: the daemon initialization path is still in the legacy file, but it should use the same config ABI as the rest of the bootstrap stack.
- Tradeoff: the legacy globals still exist as backing storage, but the read path is now narrower and easier to replace later.

## 2026-04-10: Config Snapshot Expanded To Server Identity
- Decision: include server name, server info, admin strings, network name, and listen address in `config_bootstrap_t`.
- Rationale: these are the core identity fields of the config module and should travel together in one coherent snapshot.
- Tradeoff: the snapshot is broader, but it is a more complete read-only view and a better base for later context ownership.

## 2026-04-10: Config Snapshot Expanded To Runtime Policy
- Decision: include the main runtime policy knobs in `config_bootstrap_t` as well, including timeouts, limits, feature toggles, and default mode strings.
- Rationale: the config module is still the source of truth for these values, and the host should be able to read them from one coherent view later.
- Tradeoff: the snapshot is now quite broad, but it is still read-only and provides a realistic bridge to a future instance-owned config object.

## 2026-04-10: `config_bootstrap_t` Includes Time Support
- Decision: add `<time.h>` to the public config header because the snapshot now carries `time_t` fields.
- Rationale: the public ABI must be self-sufficient and not rely on indirect transitive includes.
- Tradeoff: none; this is required for correctness and header hygiene.

## 2026-04-10: `server_app` Uses a Local Config Snapshot Helper
- Decision: centralize repeated `config_get_api_v1()->get_bootstrap()` calls behind a small `server_app_get_bootstrap()` helper.
- Rationale: this keeps the orchestration code readable while preserving the snapshot-based contract.
- Tradeoff: minimal; it reduces duplicated API-plumbing without changing ownership semantics.

## 2026-04-10: Config Snapshot Population Moved Into Helper
- Decision: move the bootstrap snapshot field population into a dedicated internal helper inside `config.c`.
- Rationale: this separates snapshot assembly from the exported ABI function and makes future split-out of runtime policy easier.
- Tradeoff: no functional change; it is an internal refactoring step toward a cleaner config module structure.

## 2026-04-10: Runtime Policy Snapshot Split Inside `config.c`
- Decision: separate runtime-policy population from identity/bootstrap population within `config.c`.
- Rationale: it creates an internal boundary between config identity and policy state, which is the first step toward a real config context object.
- Tradeoff: still no behavior change, but the config module is now organized around clearer ownership buckets.

## 2026-04-10: Identity Snapshot Split Inside `config.c`
- Decision: move the identity fields into a dedicated internal helper separate from runtime policy in `config.c`.
- Rationale: the config snapshot now has distinct identity and policy helpers, which makes future extraction of a real config context easier.
- Tradeoff: no functional change, only better internal structure for the config module.

## 2026-04-10: `conf` Load/Validate Orchestration Split
- Decision: factor the repeated load/validate orchestration in `conf.c` into a shared internal helper.
- Rationale: `Conf_Init`, `Conf_Rehash`, and `Conf_Test` were duplicating the same control flow, so the module benefits from a single orchestration path.
- Tradeoff: behavior stays the same, but the module structure is now a little easier to separate further later.

## 2026-04-10: Connection Bootstrap Moves Deeper Into `server_app`
- Decision: keep bootstrap and transport lifecycle sequencing in `server_app` while `main()` only validates and dispatches the module APIs.
- Rationale: the host executable should not own runtime policy once the orchestration boundary exists.
- Tradeoff: `server_app` becomes the primary coordination layer, but that is consistent with the target architecture and keeps the executable thin.

## 2026-04-10: Host Shutdown Uses Module Status Checks
- Decision: have `main()` check `server_app` shutdown return values instead of discarding them.
- Rationale: shutdown failures should be surfaced consistently instead of being silently ignored during bootstrap teardown.
- Tradeoff: slightly more verbosity in the host, but clearer failure reporting during teardown.

## 2026-04-10: `Conn_Handler` Status Logic Split Out
- Decision: extract idle-timeout and service-manager status reporting from `Conn_Handler` into dedicated helpers.
- Rationale: this is a safe first cut into the hot loop that reduces the size of the monolithic handler without altering behavior.
- Tradeoff: no external ABI change yet, but the handler is now slightly easier to split further.

## 2026-04-10: `Conn_Handler` Socket Interest Logic Split Out
- Decision: extract the socket read/write interest registration loops into dedicated helpers.
- Rationale: this isolates the event-loop bookkeeping from the main handler, which is the next safe step toward a transport boundary.
- Tradeoff: still internal-only, but the hot loop is now broken into smaller, more understandable pieces.

## 2026-04-10: `Conn_Handler` Dispatch Wait Split Out
- Decision: extract the `io_dispatch()` wait/error handling into a dedicated helper.
- Rationale: this removes the final direct dispatch block from the main handler and keeps the event-loop core decomposed into smaller units.
- Tradeoff: still internal-only, but the hot loop is now much easier to reason about and split further if needed.

## 2026-04-10: `conn` Hot-Path Cleanup
- Decision: remove the unused `Conn_Handle_Request_Buffer` helper and restore status buffering in `Check_Connections()` after the partial hot-path split.
- Rationale: keep the staged extraction compile-safe and avoid leaving dead scaffolding behind while the four-function split is still in progress.
- Tradeoff: no new behavior, but less compiler noise and a cleaner checkpoint for continuing the `conn` refactor.

## 2026-04-10: Bridge Layer Build Verified
- Decision: keep the staged ABI bridge targets in the tree after verifying that `ngircd.exe` builds and the full test suite passes.
- Rationale: the staged modules are now build- and test-backed, so they can serve as stable extraction seams for the remaining monolith refactor.
- Tradeoff: the repo still carries legacy global-backed implementation code, but the new seam layer is now validated end-to-end.
