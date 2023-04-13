#pragma once

class hwid_component
{
public:
    hwid_component();

    static void StartMessageService();

    static std::string GetDiskSerial();
    static std::string GetGPUGUID();
    static std::string GetSysUUID();
    
    inline static std::string access_token;
    inline static int MessageCounter;
    inline static std::string version;
    inline static bool bServiceRunning;
};
