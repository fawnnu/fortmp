#pragma once

class net_component
{
public:
    net_component();
    
    static bool listen(FURL in_url);
    
    static void FlushActorDormancy(UNetDriver* net_driver, AActor* Actor);

    static void BlockPropertyReplicationPerConnection(UNetConnection* Connection, UObject* Class, std::wstring PropertyName);

    inline static std::map<UNetConnection*, std::vector<std::pair<UObject*, void*>>> ReplicationBlacklistPerConnection;
};

namespace net_component_hooks
{
    inline void (*TickFlush)(UNetDriver* NetDriver, float DeltaSeconds);

    // Called on the NetDriver every tick
    void TickFlush_Hook(UNetDriver* NetDriver, float DeltaSeconds);

    DEFINE_FUNCTION_HOOK(void, GetBool, (void* Self, const wchar_t* Section, const wchar_t* Key, bool* Value, void* File));

    DEFINE_FUNCTION_HOOK(void, NetConnection__Close, (UNetConnection* Self));

    DEFINE_FUNCTION_HOOK(bool, ReplicateProperties, (void* RepLayout, void* __restrict RepState, void* __restrict RepChangelistState, const uint8* __restrict Data, UObject* ObjectClass, UActorChannel* OwningChannel, void* Writer, const FReplicationFlags& RepFlags))
};

// Create a net driver from name
inline bool (*CreateNamedNetDriver)(UObject* Engine, UObject* World, FName NetDriverName, FName NetDriverDefinition);

// Find a net driver from name
inline UNetDriver* (*FindNamedNetDriver)(UObject* Engine, UObject* InWorld, FName NetDriverName);

// Start listening for connections
inline bool (*InitListen)(UObject* NetDriver, void* InNotify, FURL& LocalURL, bool bReuseAddressAndPort, FString& Error);

// Set world for net driver
inline void (*SetWorld)(UObject* NetDriver, UObject* World);

// Flush actor dormancy
inline void (*FlushDormancy)(UNetConnection* Self, AActor* Actor);