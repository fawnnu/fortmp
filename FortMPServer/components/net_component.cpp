#include "include.hpp"
#include "net_component.hpp"

#include "replication_component.hpp"

net_component::net_component()
{
    INIT_POINTER(CreateNamedNetDriver, memory_manager::find_function_by_string(L"PlayReplay: failed to create demo net d", { 0xE8 }, 1));
    INIT_POINTER(FindNamedNetDriver, memory_manager::find_function_by_string(L"PlayReplay: failed to create demo net d",{0xe8, 0x100, 0x100, 0x100 , 0x0}, 1, false));
    INIT_POINTER(InitListen, memory_manager::find_function_by_string(L"Failed to init net driver ListenURL: %s: %s"));
    INIT_POINTER(SetWorld, memory_manager::find_function_by_string(L"Demo playback failed: %s", { 0xE8 }, 1));

    if (uintptr_t set_world_address = memory_manager::find_pattern("FF 90 ? ? ? ? ? 89 ? 9F 49 8D ? 28 ? 89 ? A7 4C 8D 4D 9F 48 8B 03 4C 8D 45 C7 48 8B CB FF 90 ? ? ? ?"))
    {
        INIT_POINTER(SetWorld, FindObject(L"/Script/OnlineSubsystemUtils.Default__IpNetDriver")->VTable[*reinterpret_cast<int*>(set_world_address + 2) / 8]);
    }

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    REGISTER_FUNCTION_DETOUR(net_component_hooks::TickFlush, memory_manager::find_function_by_string(L"STAT_NetTickFlush"), net_component_hooks::TickFlush_Hook);
    REGISTER_FUNCTION_DETOUR(net_component_hooks::TickFlush, memory_manager::find_function_by_string(L"UDemoNetDriver::TickFlush: ReplayStreamer ERROR: %s", {0xe8, 0x100, 0x100, 0x100, 0x0}, 1), net_component_hooks::TickFlush_Hook);

    DetourTransactionCommit();
}

bool net_component::listen(FURL in_url)
{
    if ((*GWorld)->NetDriver())
    {
        //A netdriver has been set already, therefore we shouldn't attempt listening again!
        return false;
    }
    
    const FName NAME_GameNetDriver(L"GameNetDriver");
    
    if (CreateNamedNetDriver(*GEngine, *GWorld, NAME_GameNetDriver, NAME_GameNetDriver))
    {
        if (UNetDriver* net_driver = FindNamedNetDriver(*GEngine, *GWorld, NAME_GameNetDriver))
        {
            // set NetDriver in World
            (*GWorld)->NetDriver() = net_driver;

            // set World in NetDriver
            SetWorld(net_driver, *GWorld);
            
            auto LevelCollections = TArrayManager<FLevelCollection>(&(*GWorld)->LevelCollections(), FLevelCollection::GetStructSize());

            // set netdriver in levelcollections, RPCs will now work
            LevelCollections[0].NetDriver = net_driver;
            LevelCollections[1].NetDriver = net_driver;

            void* in_notify = reinterpret_cast<void*>(__int64(*GWorld) + sizeof(UObject));

            // Error caused by InitListen (if any)
            FString in_error;

            return InitListen(net_driver, in_notify, in_url, false, in_error);
        }
    }

    return false;
}

void net_component_hooks::TickFlush_Hook(UNetDriver* NetDriver, float DeltaSeconds)
{
    if (NetDriver->ClientConnections().Num() > 0 && !NetDriver->ClientConnections()[0]->InternalAck())
    {
        // Update all clients
        replication_component::ServerReplicateActors(NetDriver, DeltaSeconds);
    }

    return TickFlush(NetDriver, DeltaSeconds);
}
