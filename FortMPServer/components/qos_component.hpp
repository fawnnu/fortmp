#pragma once

class qos_component
{
public:
    qos_component();

    static bool compare_ping_results(const std::pair<std::wstring, float>& a, const std::pair<std::wstring, float>& b);
    static FIcmpEchoResult* ping(FString TargetAddress, float Timeout);
    static std::wstring convert_region_name(std::wstring original_name);
    static std::string pick_best_region();

    static inline std::string best_region;
};

inline FIcmpEchoResult* (*UDPEchoImpl)(FIcmpEchoResult* OutResult, void* ISocketSubsystem, const FString& TargetAddress, float Timeout);
inline void* (*ISocketSubsystem__Get)(const FName& SubsystemName);

class async_wrapper
{
public:
    async_wrapper(std::function<void()> in_function)
    {
        future = std::async(std::launch::async, in_function);
    }

    bool is_running()
    {
        return future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready;
    }

    static void wait_for_wrappers(std::vector<async_wrapper*> wrappers)
    {
        while (true)
        {
            bool running = false;
            for (auto async_wrapper : wrappers)
            {
                if (async_wrapper->is_running())
                {
                    running = true;
                }
            }

            if (running)
            {
                continue;
            }
        
            break;
        }
    }
    
    std::future<void> future;
};