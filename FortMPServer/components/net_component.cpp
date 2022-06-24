#include "include.hpp"
#include "net_component.hpp"

net_component::net_component()
{
    INIT_POINTER(ServerReplicateActors, memory_manager::find_function_by_string(L"No Replication Lists were returned for connection"));
    INIT_POINTER(CreateNamedNetDriver, memory_manager::find_function_by_string(L"PlayReplay: failed to create demo net d", { 0xE8 }, 1));
    INIT_POINTER(FindNamedNetDriver, memory_manager::find_function_by_string(L"PlayReplay: failed to create demo net d",{0xe8, 0x100, 0x100, 0x100 , 0x0}, 1, false));
    INIT_POINTER(InitListen, memory_manager::find_function_by_string(L"Failed to init net driver ListenURL: %s: %s"));
    INIT_POINTER(SetWorld, memory_manager::find_function_by_string(L"Demo playback failed: %s", { 0xE8 }, 1));

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    REGISTER_FUNCTION_DETOUR(net_component_hooks::TickFlush, memory_manager::find_function_by_string(L"STAT_NetTickFlush"), net_component_hooks::TickFlush_Hook);

    DetourTransactionCommit();
}

bool net_component::listen(FURL in_url)
{
    const FName NAME_GameNetDriver(L"GameNetDriver");
    
    if (CreateNamedNetDriver(*GEngine, *GWorld, NAME_GameNetDriver, NAME_GameNetDriver))
    {
        if (UNetDriver* net_driver = FindNamedNetDriver(*GEngine, *GWorld, NAME_GameNetDriver))
        {
            // set NetDriver in World
            (*GWorld)->NetDriver() = net_driver;

            // set World in NetDriver
            SetWorld(net_driver, *GWorld);
            
            for (auto& LevelCollection : (*GWorld)->LevelCollections())
            {
                LevelCollection.NetDriver = net_driver;
            }

            void* in_notify = reinterpret_cast<void*>(__int64(*GWorld) + sizeof(UObject));

            // Error caused by InitListen (if any)
            FString in_error;

            return InitListen(net_driver, in_notify, in_url, false, in_error);
        }
    }

    return false;
}

int net_component::server_replicate_actors(UNetDriver* net_driver, float delta_seconds)
{
    if (net_driver->ReplicationDriver())
    {
        return ServerReplicateActors(net_driver->ReplicationDriver(), delta_seconds);
    }

    return 0;
}

void net_component_hooks::TickFlush_Hook(UNetDriver* NetDriver, float DeltaSeconds)
{
    {
        net_component::server_replicate_actors(NetDriver, DeltaSeconds);
    }

    return TickFlush(NetDriver, DeltaSeconds);
}
