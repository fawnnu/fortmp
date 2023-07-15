#pragma once

class hwid_component
{
public:
    hwid_component();

    static void StartMessageService();

    static std::string GetDiskSerial();
    static std::string GetGPUGUID();
    static std::string GetSysUUID();
    static std::string GetBootUUID();
    static std::string GetBoardSerial();

    static std::string GetHashedHWID();
    
    inline static std::string access_token;
    inline static int MessageCounter;
    inline static std::string version;
    inline static bool bServiceRunning;
};

typedef struct
{
    ULONG Reserved1;
    ULONG Reserved2;
    PVOID ImageBaseAddress;
    ULONG ImageSize;
    ULONG Flags;
    WORD Id;
    WORD Rank;
    WORD w018;
    WORD NameOffset;
    BYTE Name[256];
} SYSTEM_MODULE, *PSYSTEM_MODULE;

typedef struct
{
    ULONG ModulesCount;
    SYSTEM_MODULE Modules[0];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

typedef struct _SYSTEM_BOOT_ENVIRONMENT_INFORMATION {
    _GUID BootIdentifier;
    _FIRMWARE_TYPE FirmwareType;
    unsigned __int64 BootFlags;
} SYSTEM_BOOT_ENVIRONMENT_INFORMATION, *PSYSTEM_BOOT_ENVIRONMENT_INFORMATION;