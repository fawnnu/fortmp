#pragma once

class loader_component
{
public:
    loader_component();

    static void start_match(UFortPlaylistAthena* in_playlist = nullptr);

    static bool initialize_playlist(UFortPlaylistAthena* in_playlist);
    static void update_max_players(int MaxPlayers);
    static int get_max_players();
    static void spawn_floor_loot(UObject* Class, FName LootTier);
    static void initialize_vending_machines();
    static void initialize_llamas();
    static void initialize_bga();

    static void OnGamePhaseUpdated(AFortGameStateAthena* Self, FFrame& StackFrame);

    inline static bool bPendingShutdown;
};

namespace loader_component_hooks
{
    // Called when a streaming level is loaded
    DEFINE_FUNCTION_HOOK(void, UpdateLevelStreaming, (UWorld* World));

    DEFINE_FUNCTION_HOOK(void, EQSQueryDoneSignature__DelegateSignature, (UObject* Object, UObject* Function, void* Params));
    
    DEFINE_FUNCTION_HOOK(void, ABuildingContainer_BeginPlay, (ABuildingContainer* Target));

    DEFINE_FUNCTION_HOOK(bool, SetGameMode, ());
}