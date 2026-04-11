using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;

internal static unsafe class PlatformExports
{
    private const uint CORE_STATUS_OK = 0;
    private const uint CORE_STATUS_IO_ERROR = 5;

    private const uint CORE_MODULE_KIND_PLATFORM = 4;

    [StructLayout(LayoutKind.Sequential)]
    private readonly struct CoreModuleMetadata
    {
        public readonly uint api_major;
        public readonly uint api_minor;
        public readonly uint module_kind;
        public readonly nint module_name;
        public readonly nint module_version;
        public CoreModuleMetadata(uint major, uint minor, uint kind, nint name, nint version)
        {
            api_major = major;
            api_minor = minor;
            module_kind = kind;
            module_name = name;
            module_version = version;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    private readonly struct CoreModuleApi
    {
        public readonly uint api_major;
        public readonly uint api_minor;
        public readonly nint metadata;
        public readonly nint api;
        public CoreModuleApi(uint major, uint minor, nint meta, nint apiPtr)
        {
            api_major = major;
            api_minor = minor;
            metadata = meta;
            api = apiPtr;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    private readonly struct PlatformApi
    {
        public readonly uint api_major;
        public readonly uint api_minor;
        public readonly nint open_file;
        public readonly nint close_file;
        public readonly nint set_nonblock;
        public readonly nint set_cloexec;
        public PlatformApi(nint openFile, nint closeFile, nint setNonblock, nint setCloexec)
        {
            api_major = 1u;
            api_minor = 0u;
            open_file = openFile;
            close_file = closeFile;
            set_nonblock = setNonblock;
            set_cloexec = setCloexec;
        }
    }

    private static readonly nint moduleName = Marshal.StringToHGlobalAnsi("platform");
    private static readonly nint moduleVersion = Marshal.StringToHGlobalAnsi("1.0");
    private static readonly nint metadataPtr = Alloc(new CoreModuleMetadata(1u, 0u, CORE_MODULE_KIND_PLATFORM, moduleName, moduleVersion));

    private static readonly nint openFilePtr = (nint)(delegate* unmanaged[Cdecl]<nint, int, uint>)&OpenFile;
    private static readonly nint closeFilePtr = (nint)(delegate* unmanaged[Cdecl]<int, uint>)&CloseFile;
    private static readonly nint setNonblockPtr = (nint)(delegate* unmanaged[Cdecl]<int, uint>)&SetNonblock;
    private static readonly nint setCloexecPtr = (nint)(delegate* unmanaged[Cdecl]<int, uint>)&SetCloexec;

    private static readonly nint platformApiPtr = Alloc(new PlatformApi(openFilePtr, closeFilePtr, setNonblockPtr, setCloexecPtr));
    private static readonly nint moduleApiPtr = Alloc(new CoreModuleApi(1u, 0u, metadataPtr, platformApiPtr));

    private static nint Alloc<T>(T value) where T : unmanaged
    {
        nint mem = (nint)NativeMemory.AllocZeroed(1, (nuint)sizeof(T));
        Unsafe.Write((void*)mem, value);
        return mem;
    }

    [UnmanagedCallersOnly(EntryPoint = "platform_get_api_v1", CallConvs = new[] { typeof(CallConvCdecl) })]
    public static nint PlatformGetApiV1() => platformApiPtr;

    [UnmanagedCallersOnly(EntryPoint = "module_get_api_v1", CallConvs = new[] { typeof(CallConvCdecl) })]
    public static nint ModuleGetApiV1() => moduleApiPtr;

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static uint OpenFile(nint path, int flags) => path == 0 ? CORE_STATUS_IO_ERROR : CORE_STATUS_OK;

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static uint CloseFile(int fd) => fd < 0 ? CORE_STATUS_IO_ERROR : CORE_STATUS_OK;

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static uint SetNonblock(int fd) => fd < 0 ? CORE_STATUS_IO_ERROR : CORE_STATUS_OK;

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static uint SetCloexec(int fd) => fd < 0 ? CORE_STATUS_IO_ERROR : CORE_STATUS_OK;
}
