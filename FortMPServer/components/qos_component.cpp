#include "include.hpp"
#include "qos_component.hpp"

#include "config_manager.hpp"

qos_component::qos_component()
{
    // Register regions
    era_regions[TEXT("EU")] = TEXT("142.132.145.234:22222");
    era_regions[TEXT("NA")] = TEXT("69.10.34.38:22222");
    era_regions[TEXT("OCE")] = TEXT("139.99.209.91:22222");
    
    if (memory_manager::does_address_belong_to_func(memory_manager::find_function_by_string(L"Failed to send broadcast packet %d", {0xe8, 0x100, 0x100, 0x100, 0x1}, 1, true, false, true), memory_manager::find_function_by_string(L"Failed to send broadcast packet %d")))
    {
        INIT_POINTER(ISocketSubsystem__Get, memory_manager::find_function_by_string(L"Failed to send broadcast packet %d", {0xe8, 0x100, 0x100, 0x100, 0x1}, 1));   
    }
    else if (memory_manager::does_address_belong_to_func(memory_manager::find_function_by_string(L"Failed to send broadcast packet %d", {0xe8, 0x100, 0x100, 0x100, 0x2}, 1, true, false, true), memory_manager::find_function_by_string(L"Failed to send broadcast packet %d")))
    {
        INIT_POINTER(ISocketSubsystem__Get, memory_manager::find_function_by_string(L"Failed to send broadcast packet %d", {0xe8, 0x100, 0x100, 0x100, 0x2}, 1));  
    }
    else if (memory_manager::does_address_belong_to_func(memory_manager::find_function_by_string(L"Failed to send broadcast packet %d", {0xe8, 0x100, 0x100, 0x100, 0xFE}, 1, true, false, true), memory_manager::find_function_by_string(L"Failed to send broadcast packet %d")))
    {
        INIT_POINTER(ISocketSubsystem__Get, memory_manager::find_function_by_string(L"Failed to send broadcast packet %d", {0xe8, 0x100, 0x100, 0x100, 0xFE}, 1));  
    }
    
    INIT_POINTER(UDPEchoImpl, memory_manager::find_function_by_string(L"UDPPing", {}, 0, true, true, false, true));

    best_region = pick_best_region();
}

bool qos_component::compare_ping_results(const std::pair<std::wstring, float>& a, const std::pair<std::wstring, float>& b)
{
    return a.second < b.second;   
}

FIcmpEchoResult* qos_component::ping(FString TargetAddress, float Timeout)
{
    // Get the socket subsystem
    void* SocketSubsystem = ISocketSubsystem__Get(TEXT("WINDOWS"));

    // Perform ping
    FIcmpEchoResult Result;
    return UDPEchoImpl(&Result, SocketSubsystem, TargetAddress, Timeout);
}

std::wstring qos_component::convert_region_name(std::wstring original_name)
{
    // Always return NA unless the best region was EU, GB
    if (original_name.find(TEXT("EU")) != std::wstring::npos ||
        original_name.find(TEXT("GB")) != std::wstring::npos)
    {
        return TEXT("EU");
    }
    if (original_name.find(TEXT("OCE")) != std::wstring::npos)
    {
        return TEXT("OCE");
    }
    
    return TEXT("NA"); // Default region
}

std::string qos_component::pick_best_region()
{
    auto config = config_manager::get_server_config();
    if (config.contains("override_region") && config["override_region"].is_string())
    {
        std::string override_region = config["override_region"];
        if (override_region.length() > 0)
        {
            TO_WSTRING(override_region, wOverride_region);

            // Make sure it's a valid region
            if (era_regions.contains(wOverride_region))
            {
                spdlog::warn("qos_component::pick_best_region() was overridden. Picked region \"{}\"", override_region.c_str());
                
                return override_region;
            }
        }
    }
    
    // Create a QOS region manager
    UQosRegionManager* QosRegionManager = static_cast<UQosRegionManager*>(UGameplayStatics::SpawnObject(UQosRegionManager::StaticClass(), FindObject(L"/Engine/Transient")));

    // Map of each region with relative average ping
    std::vector<std::pair<std::wstring, float>> DatacentersList;

    // List of "threads" pinging
    std::vector<async_wrapper*> datacenter_async_wrappers;
    
    // Go through each datacenter
    for (const auto& [region, ip] : era_regions)
    {
        datacenter_async_wrappers.push_back(new async_wrapper([&]
        {
            std::vector<async_wrapper*> ping_async_wrappers;
            float TotalPing = 0.f;

            // Ping 3 times
            for (int i = 0; i < 3; i++)
            {
                ping_async_wrappers.push_back(new async_wrapper([&]
                {
                    auto Result = ping(ip.c_str(), 3.f);

                    TotalPing += Result->Time;
                }));
            }

            async_wrapper::wait_for_wrappers(ping_async_wrappers);

            DatacentersList.push_back({region, TotalPing / 3});
        }));
    }

    // Wait for tasks to be done
    async_wrapper::wait_for_wrappers(datacenter_async_wrappers);

    // Sort the results
    std::sort(DatacentersList.begin(), DatacentersList.end(), compare_ping_results);

    // Get the first element of the recently sorted vector (Lowest ping)
    std::wstring RegionID = DatacentersList[0].first;

    // Convert the region
    auto ConvertedRegionID = convert_region_name(RegionID);
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    
    std::string sRegionID = converter.to_bytes(ConvertedRegionID);

    spdlog::warn("qos_component::pick_best_region() finished. Picked region \"{}\"", sRegionID.c_str());

    return sRegionID;
}

