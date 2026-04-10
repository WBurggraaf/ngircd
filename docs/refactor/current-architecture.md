# Current Architecture

## Overview
The codebase is still a monolithic native IRC server with a few in-process seams for tests and backend substitution. The executable in `original/src/ngircd/platform/ngircd.c` owns process entry, command-line parsing, daemon bootstrap, and the main runtime loop. Most server behavior still lives in the same build target and shares global mutable state.

## Entry Path
- Program entry starts in `original/src/ngircd/platform/ngircd.c:74`.
- `main()` parses CLI flags, handles `--configtest`, `--help`, `--version`, `--debug`, `--nodaemon`, and related switches.
- After CLI processing, it initializes logging, random seeding, configuration, daemon state, I/O, signals, channels, connections, classes, and clients.
- `Conn_Handler()` in `original/src/ngircd/net/conn.c` runs the event loop until shutdown or restart is signaled.

## Startup And Bootstrap Flow
- `main()` sets process flags, fills version strings, and decides whether to run config test mode or the server loop.
- `NGIRCd_Init()` performs daemon-specific startup work: SSL library init, chroot, UID/GID changes, fork/setsid, stdio redirection, and PID file creation.
- The bootstrap path mixes orchestration with platform work and server policy, so it is not yet separable by DLL boundary.

## Configuration Flow
- Configuration loading starts in `Conf_Init()` in `original/src/ngircd/core/conf.c`.
- Parsing is file-driven and section-based, with handlers such as `Handle_GLOBAL()`, `Handle_OPTIONS()`, `Handle_SERVER()`, and `Handle_CHANNEL()`.
- Validation is centralized in `Validate_Config()`.
- `Conf_Rehash()` reloads configuration at runtime and updates the local server client info.
- `Conf_Test()` formats a config dump for `--configtest`.
- Configuration state is stored in many exported globals declared in `original/src/ngircd/conf.h`.

## Logging Flow
- Logging is initialized by `Log_Init()` and later reinitialized by `Log_ReInit()`.
- The current logging backend lives in `original/src/ngircd/core/log.c`.
- Logging remains an in-process service with pluggable ops through `module_iface.h`, but it is not yet a public DLL contract.

## Socket Listener Setup
- Listener creation happens in `Conn_InitListeners()` in `original/src/ngircd/net/conn.c`.
- The code supports normal socket creation and systemd-style socket activation via `my_sd_listen_fds()`.
- `NewListener()` binds, listens, and stores sockets in the listener array.
- SSL-enabled listeners are handled separately when compiled with SSL support.

## Event Loop
- The I/O backend is abstracted by `original/src/ngircd/net/io.c`.
- The implementation chooses among epoll, kqueue, /dev/poll, poll, or select at build/runtime depending on platform support.
- `Conn_Handler()` drives the main loop by calling `io_dispatch()`, processing connection buffers, throttling, timeouts, and periodic maintenance.

## Connection Lifecycle
- `Conn_Init()` prepares the connection pool.
- `New_Connection()` creates inbound connections, while `New_Server()` establishes outbound server links.
- `Conn_Close()` and `Conn_Exit()` tear down individual connections or the whole pool.
- `CONNECTION` is defined in `original/src/ngircd/conn.h` and currently remains an internal monolith-owned structure.
- Connection state is tightly interwoven with client state, resolver subprocesses, SSL, compression, and protocol handling.

## Resolver, DNS, And Ident Flow
- Reverse DNS and ident work are done through subprocess plumbing in `original/src/ngircd/net/proc.c` and `original/src/ngircd/net/resolve.c`.
- `cb_Read_Resolver_Result()` in `conn.c` consumes lookup results and updates host/user info for unregistered clients.
- `cb_Connect_to_Server()` consumes forward lookup results for outbound server connections.
- Resolver behavior is already callback-based, which is a good future seam, but it still depends on connection internals and configuration globals.

## Parse, Dispatch, And Registration Flow
- IRC parsing starts in `original/src/ngircd/proto/parse.c`.
- The parse layer dispatches to handlers in `original/src/ngircd/proto/*.c`.
- Registration/login behavior is split across `login_state.c`, `login_flow.c`, `login_check.c`, `nick_check.c`, `user_check.c`, `join_check.c`, and `join_error.c`.
- Command handlers still depend directly on global configuration, client tables, channel tables, and connection helpers.

## State Ownership
- Client state is owned by `original/src/ngircd/core/client.c` and exposed through `original/src/ngircd/client.h`.
- Channel state is owned by `original/src/ngircd/channel.c` and exposed through `original/src/ngircd/channel.h`.
- Connection state is owned by `original/src/ngircd/net/conn.c` and exposed through `original/src/ngircd/conn.h`.
- Configuration state is owned by `original/src/ngircd/core/conf.c` and exposed through `original/src/ngircd/conf.h`.
- These modules are not yet cleanly isolated; they form the core of the current monolith.

## Message Routing
- Routing is primarily implemented through protocol handler calls and channel/client helpers.
- `Channel_Write()`, `Client_Announce()`, `Conn_WriteStr()`, and related helpers are used widely across the protocol layer.
- There is no stable public message-routing ABI yet; routing remains implementation-coupled.

## Reload, Rehash, And Shutdown
- `Conf_Rehash()` supports runtime configuration reload.
- Shutdown starts when the signal layer sets quit/restart flags, then the main loop exits and `Conn_Exit()`, `Client_Exit()`, `Channel_Exit()`, `Class_Exit()`, `Log_Exit()`, and `Signals_Exit()` run in order.
- PID file cleanup happens in `Pidfile_Delete()` in the platform entry file.

## Build And Link Structure
- `CMakeLists.txt` builds a single `ngircd` executable plus static support libraries `ngportab`, `ngtool`, and `ngipaddr`.
- White-box test executables are linked directly against selected monolith source files.
- There are no DLL targets, no module loader, and no exported ABI headers yet.

## Windows-Specific Behavior
- The repo contains `win32/src` and `win32/include` shims for Windows portability.
- Build configuration prioritizes the Win32 compatibility headers.
- The current code still assumes a mostly native, single-process model with explicit platform work in the host process.

## Global Mutable State Inventory
- Truly process-global or legacy-global: `NGIRCd_Start`, `NGIRCd_Version`, `NGIRCd_VersionAddition`, `NGIRCd_Debug`, `NGIRCd_Sniffer`, `NGIRCd_Passive`, `NGIRCd_SignalQuit`, `NGIRCd_SignalRestart`, `NGIRCd_DebugLevel`, `NGIRCd_ConfFile`, `NGIRCd_ProtoID`.
- Host-owned singleton candidates: logging backend state, signal backend state, I/O backend selection.
- Module-owned singletons: configuration tables, connection pool, client table, channel table, resolver subprocess state.
- Instance-owned contexts are largely absent; most runtime state still rides on globals.
- Removable legacy globals include many direct `Conf_*` exports and the direct `My_Connections`/`Pool_Size` pattern.

## Coupling Hotspots
- `original/src/ngircd/conf.h` exports a wide configuration surface and causes broad compile-time coupling.
- `original/src/ngircd/client.h`, `channel.h`, and `conn.h` expose internal data models to many subsystems.
- `original/src/ngircd/net/conn.c` is a god module for listener setup, connection lifecycle, resolver callbacks, and socket I/O integration.
- `original/src/ngircd/core/conf.c` is a god module for config parsing, validation, runtime rehash, and config dump output.
- The protocol layer is still heavily dependent on these modules instead of consuming narrow interfaces.

## Source-To-Responsibility Map
- `original/src/ngircd/platform/ngircd.c`: entry, bootstrap, daemon lifecycle.
- `original/src/ngircd/core/conf.c`: config parse, validate, dump, rehash.
- `original/src/ngircd/core/log.c`: logging backend.
- `original/src/ngircd/net/io.c`: event-loop backend abstraction.
- `original/src/ngircd/net/conn.c`: socket lifecycle, connection pool, outbound server login, resolver callbacks.
- `original/src/ngircd/net/proc.c`: subprocess spawning and pipe-based communication.
- `original/src/ngircd/net/resolve.c`: DNS and ident lookup orchestration.
- `original/src/ngircd/core/client.c`: client state and identity.
- `original/src/ngircd/channel.c`: channel state and membership.
- `original/src/ngircd/proto/*.c`: IRC parsing, command dispatch, numerics, registration flows, and command-specific behavior.
- `original/src/ngircd/core/module_iface.c` and `original/src/ngircd/module_iface.h`: in-process backend seam registry.
