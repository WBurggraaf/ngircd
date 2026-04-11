using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

internal static unsafe class ConfigExports
{
    private const uint CORE_STATUS_OK = 0;
    private const uint CORE_STATUS_INVALID_ARGUMENT = 1;

    private const uint CORE_MODULE_KIND_CONFIG = 3;

    [StructLayout(LayoutKind.Sequential)]
    private struct ConfigBootstrap
    {
        public uint api_version;
        public nint server_name;
        public nint server_info;
        public nint server_admin1;
        public nint server_admin2;
        public nint server_admin_mail;
        public nint network_name;
        public nint listen_address;
        public int oper_can_mode;
        public nint pid_file;
        public nint chroot_dir;
        public uint uid;
        public uint gid;
        public int ping_timeout;
        public int pong_timeout;
        public int connect_retry;
        public int idle_timeout;
        public int max_connections;
        public int max_joins;
        public int max_connections_ip;
        public uint max_nick_length;
        public int max_list_size;
        public long max_penalty_time;
        public int connect_ipv4;
        public int connect_ipv6;
        public nint default_channel_modes;
        public nint default_user_modes;
        public int dns_enabled;
        public int ident_enabled;
        public int more_privacy;
        public int notice_before_registration;
        public int pam_enabled;
        public int pam_is_optional;
        public nint pam_service_name;
        public int scrub_ctcp;
    }

    [StructLayout(LayoutKind.Sequential)]
    private readonly struct ConfigApi
    {
        public readonly uint api_major;
        public readonly uint api_minor;
        public readonly nint init;
        public readonly nint rehash;
        public readonly nint test;
        public readonly nint get_bootstrap;
        public ConfigApi(nint initPtr, nint rehashPtr, nint testPtr, nint getBootstrapPtr)
        {
            api_major = 1u;
            api_minor = 0u;
            init = initPtr;
            rehash = rehashPtr;
            test = testPtr;
            get_bootstrap = getBootstrapPtr;
        }
    }

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
        public CoreModuleApi(uint major, uint minor, nint metadataPtr, nint apiPtr)
        {
            api_major = major;
            api_minor = minor;
            metadata = metadataPtr;
            api = apiPtr;
        }
    }

    private static readonly nint moduleName = Marshal.StringToHGlobalAnsi("config");
    private static readonly nint moduleVersion = Marshal.StringToHGlobalAnsi("1.0");
    private static readonly nint serverName = Marshal.StringToHGlobalAnsi("ngircd");
    private static readonly nint serverInfo = Marshal.StringToHGlobalAnsi("ngIRCd managed config");
    private static readonly nint admin1 = Marshal.StringToHGlobalAnsi("ngIRCd");
    private static readonly nint admin2 = Marshal.StringToHGlobalAnsi("Managed");
    private static readonly nint adminMail = Marshal.StringToHGlobalAnsi("ngircd@lists.barton.de");
    private static readonly nint networkName = Marshal.StringToHGlobalAnsi("ngircd");
    private static readonly nint listenAddress = Marshal.StringToHGlobalAnsi("0.0.0.0");
    private static readonly nint pidFile = Marshal.StringToHGlobalAnsi("C:/develop/ngircd/runtime/ngircd.pid");
    private static readonly nint chrootDir = 0;
    private static readonly nint defaultModes = Marshal.StringToHGlobalAnsi("+nt");
    private static readonly nint pamService = Marshal.StringToHGlobalAnsi("ngircd");

    private static readonly ConfigBootstrap bootstrapValue = CreateBootstrap();
    private static readonly nint bootstrapPtr = Alloc(bootstrapValue);

    private static ConfigBootstrap CreateBootstrap()
    {
        ConfigBootstrap bs = default;
        bs.api_version = 1u;
        bs.server_name = serverName;
        bs.server_info = serverInfo;
        bs.server_admin1 = admin1;
        bs.server_admin2 = admin2;
        bs.server_admin_mail = adminMail;
        bs.network_name = networkName;
        bs.listen_address = listenAddress;
        bs.oper_can_mode = 0;
        bs.pid_file = pidFile;
        bs.chroot_dir = chrootDir;
        bs.uid = 0;
        bs.gid = 0;
        bs.ping_timeout = 60;
        bs.pong_timeout = 60;
        bs.connect_retry = 30;
        bs.idle_timeout = 300;
        bs.max_connections = 1024;
        bs.max_joins = 0;
        bs.max_connections_ip = 0;
        bs.max_nick_length = 9;
        bs.max_list_size = 0;
        bs.max_penalty_time = 0L;
        bs.connect_ipv4 = 1;
        bs.connect_ipv6 = 1;
        bs.default_channel_modes = defaultModes;
        bs.default_user_modes = defaultModes;
        bs.dns_enabled = 1;
        bs.ident_enabled = 1;
        bs.more_privacy = 0;
        bs.notice_before_registration = 0;
        bs.pam_enabled = 0;
        bs.pam_is_optional = 0;
        bs.pam_service_name = pamService;
        bs.scrub_ctcp = 0;
        return bs;
    }

    private static readonly nint initPtr = (nint)(delegate* unmanaged[Cdecl]<uint>)&Init;
    private static readonly nint rehashPtr = (nint)(delegate* unmanaged[Cdecl]<uint>)&Rehash;
    private static readonly nint testPtr = (nint)(delegate* unmanaged[Cdecl]<int>)&Test;
    private static readonly nint getBootstrapPtr = (nint)(delegate* unmanaged[Cdecl]<nint, uint>)&GetBootstrap;
    private static readonly nint apiPtr = Alloc(new ConfigApi(initPtr, rehashPtr, testPtr, getBootstrapPtr));
    private static readonly nint metadataPtr = Alloc(new CoreModuleMetadata(1u, 0u, CORE_MODULE_KIND_CONFIG, moduleName, moduleVersion));
    private static readonly nint moduleApiPtr = Alloc(new CoreModuleApi(1u, 0u, metadataPtr, apiPtr));

    private static nint Alloc<T>(T value) where T : unmanaged
    {
        nint mem = (nint)NativeMemory.AllocZeroed(1, (nuint)sizeof(T));
        Unsafe.Write((void*)mem, value);
        return mem;
    }

    [UnmanagedCallersOnly(EntryPoint = "config_get_api_v1", CallConvs = new[] { typeof(CallConvCdecl) })]
    public static nint ConfigGetApiV1() => apiPtr;

    [UnmanagedCallersOnly(EntryPoint = "module_get_api_v1", CallConvs = new[] { typeof(CallConvCdecl) })]
    public static nint ModuleGetApiV1() => moduleApiPtr;

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static uint Init() => CORE_STATUS_OK;

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static uint Rehash() => CORE_STATUS_OK;

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int Test() => 0;

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static uint GetBootstrap(nint bootstrap)
    {
        if (bootstrap == 0)
            return CORE_STATUS_INVALID_ARGUMENT;

        Unsafe.Write((void*)bootstrap, bootstrapValue);
        return CORE_STATUS_OK;
    }
}
