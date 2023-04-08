#pragma once

class hwid_component
{
public:
    hwid_component();

    static void StartMessageService();
    
    inline static std::string access_token;
    inline static int MessageCounter;
    inline static std::string version;
    inline static bool bServiceRunning;
};
