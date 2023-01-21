#pragma once

class qos_component
{
public:
    qos_component();

    static bool compare_ping_results(const std::pair<std::wstring, float>& a, const std::pair<std::wstring, float>& b);
    static FIcmpEchoResult* ping(FString TargetAddress, float Timeout);
    static std::wstring convert_region_name(std::wstring original_name);
    static std::string pick_best_region();
};

inline FIcmpEchoResult* (*UDPEchoImpl)(FIcmpEchoResult* OutResult, void* ISocketSubsystem, const FString& TargetAddress, float Timeout);
inline void* (*ISocketSubsystem__Get)(const FName& SubsystemName);