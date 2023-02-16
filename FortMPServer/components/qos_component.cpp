#include "include.hpp"
#include "qos_component.hpp"

qos_component::qos_component()
{
    INIT_POINTER(ISocketSubsystem__Get, memory_manager::find_function_by_string(L"Failed to send broadcast packet %d", {0xe8, 0x100, 0x100, 0x100, 0x1}, 1));
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
    // Always return NA unless the best region was EU
    if (original_name.find(TEXT("EU")) != std::wstring::npos)
    {
        return TEXT("EU");
    }
    
    return TEXT("NA"); // Default region
}

std::string qos_component::pick_best_region()
{
    // Create a QOS region manager
    UQosRegionManager* QosRegionManager = static_cast<UQosRegionManager*>(UGameplayStatics::SpawnObject(UQosRegionManager::StaticClass(), FindObject(L"/Engine/Transient")));

    // Map of each region with relative average ping
    std::vector<std::pair<std::wstring, float>> DatacentersList;

    // Go through each datacenter
    for (auto& Datacenter : TArrayManager<FQosDatacenterInfo>(&QosRegionManager->DatacenterDefinitions(), FQosDatacenterInfo::GetStructSize()))
    {
        if (Datacenter.bEnabled())
        {
            float TotalPing = 0.f;
            const int NumServers = Datacenter.Servers().Num();

            // Pick a random server for each datacenter
            FQosPingServerInfo& Server = TArrayManager<FQosPingServerInfo>(&Datacenter.Servers(), FQosPingServerInfo::GetStructSize())[UKismetMathLibrary::RandomIntegerInRange(0, NumServers - 1)];
        
            // Ping the server the amount of times required
            for (int i = 0; i < QosRegionManager->NumTestsPerRegion(); i++)
            {
                // Build the address
                FString TargetAddress = FString(std::wstring(Server.Address().c_str()).append(L":").append(std::to_wstring(Server.Port())).c_str());

                // Ping!
                auto Result = ping(TargetAddress, QosRegionManager->PingTimeout());

                // Add the result to the total ping for this datacenter
                TotalPing += Result->Time;
            }

            // Add the result to DatacentersMap
            DatacentersList.push_back({Datacenter.RegionId().c_str(), TotalPing / NumServers});
        }
    }

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

