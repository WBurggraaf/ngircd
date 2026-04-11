using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

internal static unsafe class LoggingExports
{
    private const uint CoreStatusOk = 0;
    private const uint CoreModuleKindLogging = 2;

    [StructLayout(LayoutKind.Sequential)]
    private struct CoreModuleMetadata
    {
        public uint api_major;
        public uint api_minor;
        public uint module_kind;
        public nint module_name;
        public nint module_version;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct CoreModuleApi
    {
        public uint api_major;
        public uint api_minor;
        public nint metadata;
        public nint api;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct LoggingApi
    {
        public uint api_major;
        public uint api_minor;
        public nint init;
        public nint shutdown;
        public nint reinit;
        public nint log_message;
        public nint debug_message;
    }

    private static readonly nint moduleName = Marshal.StringToHGlobalAnsi("logging");
    private static readonly nint moduleVersion = Marshal.StringToHGlobalAnsi("1.0");
    private static readonly CoreModuleMetadata metadataValue = new CoreModuleMetadata
    {
        api_major = 1u,
        api_minor = 0u,
        module_kind = CoreModuleKindLogging,
        module_name = moduleName,
        module_version = moduleVersion
    };

    private static readonly nint metadataPtr = Alloc(metadataValue);
    private static readonly nint initPtr = (nint)(delegate* unmanaged[Cdecl]<int, uint>)&Init;
    private static readonly nint shutdownPtr = (nint)(delegate* unmanaged[Cdecl]<void>)&Shutdown;
    private static readonly nint reinitPtr = (nint)(delegate* unmanaged[Cdecl]<void>)&Reinit;
    private static readonly nint logMessagePtr = (nint)(delegate* unmanaged[Cdecl]<int, nint, void>)&LogMessage;
    private static readonly nint debugMessagePtr = (nint)(delegate* unmanaged[Cdecl]<nint, void>)&DebugMessage;
    private static readonly LoggingApi apiValue = new LoggingApi
    {
        api_major = 1u,
        api_minor = 0u,
        init = initPtr,
        shutdown = shutdownPtr,
        reinit = reinitPtr,
        log_message = logMessagePtr,
        debug_message = debugMessagePtr
    };
    private static readonly nint apiPtr = Alloc(apiValue);
    private static readonly CoreModuleApi moduleApiValue = new CoreModuleApi
    {
        api_major = 1u,
        api_minor = 0u,
        metadata = metadataPtr,
        api = apiPtr
    };
    private static readonly nint moduleApiPtr = Alloc(moduleApiValue);

    private static nint Alloc<T>(T value) where T : unmanaged
    {
        nint mem = (nint)NativeMemory.AllocZeroed(1, (nuint)sizeof(T));
        Unsafe.Write((void*)mem, value);
        return mem;
    }

    [UnmanagedCallersOnly(EntryPoint = "logging_get_api_v1", CallConvs = new[] { typeof(CallConvCdecl) })]
    public static nint LoggingGetApiV1() => apiPtr;

    [UnmanagedCallersOnly(EntryPoint = "module_get_api_v1", CallConvs = new[] { typeof(CallConvCdecl) })]
    public static nint ModuleGetApiV1() => moduleApiPtr;

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static uint Init(int syslogMode) => CoreStatusOk;

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static void Shutdown() { }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static void Reinit() { }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static void LogMessage(int level, nint message)
    {
        Console.WriteLine($"[{level}] {Marshal.PtrToStringAnsi(message) ?? string.Empty}");
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static void DebugMessage(nint message)
    {
        Console.WriteLine(Marshal.PtrToStringAnsi(message) ?? string.Empty);
    }
}
