#pragma once

class loader_component
{
public:
    loader_component();
    static void pre_init();

    static void load_asset(FName Path, FName Asset);
    static void start_match(UFortPlaylistAthena* in_playlist = nullptr);
    static void handle_unloaded_foundations();
    static bool initialize_playlist(UFortPlaylistAthena* in_playlist);
    static void update_max_players(int MaxPlayers);
    static int get_max_players();
    static void spawn_floor_loot(UObject* Class, FName LootTier);
    static void initialize_vending_machines();
    static void initialize_llamas();
    static void initialize_bga();
    static void initialize_reboot_vans();
    static void handle_duplicated_foundations();
    static void handle_cl_specific_additions();

    static void parse_game_data_hotfixes(UFortPlaylistAthena* in_playlist);
    
    static void OnMapInfoLoaded();
    
    static void OnGamePhaseUpdated(AFortGameStateAthena* Self, FFrame& StackFrame);
    static void WaitForMapInfo(AFortGameStateAthena* Self, FFrame& StackFrame);
    static void SwapButterflyEventPOIs(AFortGameModeAthena* Self, FFrame& StackFrame);
    
    inline static bool bPendingShutdown;

    // Log
    inline static bool bLogged85Percent;
    inline static bool bLogged95Percent;
};

namespace loader_component_hooks
{
    // Called when a streaming level is loaded
    DEFINE_FUNCTION_HOOK(void, UpdateLevelStreaming, (UWorld* World));

    DEFINE_FUNCTION_HOOK(void, EQSQueryDoneSignature__DelegateSignature, (UObject* Object, UObject* Function, void* Params));
    DEFINE_FUNCTION_HOOK(void, SwapPOI, (UObject* Object, UObject* Function, void* Params));
    
    DEFINE_FUNCTION_HOOK(void, ABuildingContainer_BeginPlay, (ABuildingContainer* Target));
    DEFINE_FUNCTION_HOOK(void, AFortGameModeAthena_BeginPlay, (AFortGameModeAthena* Target));
    
    DEFINE_FUNCTION_HOOK(bool, SetGameMode, ());
    DEFINE_FUNCTION_HOOK(void, NotifyCustomStatEvent, (UFortQuestManager* QuestManager, UObject* ContextObject, FGameplayTagContainer SourceTags, FGameplayTagContainer ContextTags));
    
    DEFINE_FUNCTION_HOOK(UObject*, GetGameSessionClass, (AGameModeBase* GameMode, UObject* GameSessionClass));

    DEFINE_FUNCTION_HOOK(bool, ShouldSpawnVehicleInternal, (AFortGameModeAthena* GameMode, UObject* VehicleClass));
    
    DEFINE_FUNCTION_HOOK(void, BuildingFoundation_PostUpdate, (ABuildingFoundation* BuildingFoundation, uint8_t State, void* ReservedValues));
    
    // Steal the function
    DEFINE_FUNCTION_HOOK(void, SetCurrentPlaylistId, (AFortGameStateAthena* Self, int PlaylistId));

    DEFINE_FUNCTION_HOOK(void, OnPlaylistDataLoaded, (AFortGameStateAthena* Self));
    DEFINE_FUNCTION_HOOK(void, GetCalendarEventInformation, (class UObject* WorldContextObject, FString* EventName, bool* bIsEventActive, FTimespan* TimeSinceBegin, FTimespan* TimeUntilEnd, float* TimespanRatio))
}