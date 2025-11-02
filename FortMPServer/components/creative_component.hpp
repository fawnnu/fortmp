#pragma once

class creative_component
{
public:

    // Init
    creative_component();

    // Tells if we are currently playing creative
    static bool is_creative_enabled();
    static bool is_playground_enabled();
    
    // Assign a portal to the joining player
    static void assign_portal_to_player(UNetConnection* NetConnection, int PortalIndex);
    
    // Handle everything that has to be done once a player joins the match
    static void handle_player_join(UNetConnection* NetConnection);

    static void LoadIsland(AFortVolume* Volume);
    
    static const TCHAR* GetEraSaveFileName(const TCHAR* OldName);

    static AFortPlayerControllerAthena* GetVolumeOwner(AFortVolume* Volume);

    // Respawn fixes (Game bugs)
    static void OnRespawn(AController* PlayerController, AActor*& PickedStart);

    // Custom functions
    static void TeleportPlayerToLobbyIslandWithTimer(AFortPlayerControllerAthena* PlayerController, FFrame& StackFrame);
    static void TeleportPlayerToIsland(AFortPlayerControllerAthena* PlayerController, FFrame& StackFrame);
    static void FinalizeTeleport(AFortPlayerControllerAthena* PlayerController, FFrame& StackFrame);
};

namespace creative_component_hooks
{
    // Called when a player enters a creative portal
    DEFINE_FUNCTION_HOOK(void, TeleportPlayerToLinkedVolume, (AFortAthenaCreativePortal* Portal, AFortPlayerPawn* PlayerPawn, bool bUseSpawnTags));

    // Called when granting an item via the creative inventory
    DEFINE_FUNCTION_HOOK(void, ServerGiveCreativeItem, (AFortPlayerController* FortPlayerController, FFrame& StackFrame));
    DEFINE_FUNCTION_HOOK(void, ResetMyCurrentCreativePlot, (AFortPlayerControllerAthena* PlayerController, FFrame& StackFrame));
    
    DEFINE_FUNCTION_HOOK(void, AddMiniGamePlayer, (AFortMinigame* MiniGame, AFortPlayerState* PlayerState));
    
    DEFINE_FUNCTION_HOOK(void, RemoveMiniGamePlayer, (AFortMinigame* MiniGame, AFortPlayerState* PlayerState));
    
    // Called when trying to go back to hub
    DEFINE_FUNCTION_HOOK(void, ServerTeleportToPlaygroundLobbyIsland, (AFortPlayerControllerAthena* FortPlayerControllerAthena, FFrame& StackFrame));

    DEFINE_FUNCTION_HOOK(void, HandleVolumeStateChanged, (AFortVolumeManager*, FFrame& StackFrame));
    DEFINE_FUNCTION_HOOK(void, OnFinishedRecordRead, (UObject*, UObject*, void*));
    DEFINE_FUNCTION_HOOK(void, OnFinishedRecordWrite, (UObject*, UObject*, void*));
    DEFINE_FUNCTION_HOOK(void, HandleMinigameStarted, (UObject*));
    DEFINE_FUNCTION_HOOK(void, HandleMinigameEnded, (UObject*));
    DEFINE_FUNCTION_HOOK(void, HandleMinigameWarmup, (UObject*, UObject*, void*));
    DEFINE_FUNCTION_HOOK(void, OnFinishedLoading, (UObject*, UObject*, void*));
    DEFINE_FUNCTION_HOOK(void, SaveOutLevelRecord, (UFortLevelSaveComponent* Self))
    DEFINE_FUNCTION_HOOK(bool, SaveArrayToFile, (TArray<uint8>& Self, const TCHAR* FileName, void* FileManager, uint32 WriteFlags))
    DEFINE_FUNCTION_HOOK(bool, LoadFileToArray, (TArray<uint8>& Self, const TCHAR* FileName, uint32 WriteFlags))
    DEFINE_FUNCTION_HOOK(void, SetTeamForPlayer, (AFortMinigame* Self, AFortPlayerController* AFortPlayerController, uint8_t TeamIndex));
    DEFINE_FUNCTION_HOOK(void, Server_SetCanEditCreativeIsland, (AFortPlayerStateAthena* Self, FFrame& StackFrame));
    DEFINE_FUNCTION_HOOK(void, ServerLoadPlotForPortal_Impl, (AFortPlayerControllerAthena* Self, AFortAthenaCreativePortal* Portal, FString& PlotItemId));
    DEFINE_FUNCTION_HOOK(bool, ServerLoadPlotForPortal_Validate, (AFortPlayerControllerAthena* Self, FFrame& StackFrame));
    DEFINE_FUNCTION_HOOK(void, NativeEnteringPlayspaceEvent, (AFortMinigame* Self, class APlayerState* PlayerState, class AFortVolume* EnteredVolume));
    DEFINE_FUNCTION_HOOK(void, NativeExitedPlayspaceEvent, (AFortMinigame* Self, class APlayerState* PlayerState, class AFortVolume* ExitedVolume));
    inline int SaveOutLevelRecord_Index;
    inline int HandleMinigameStarted_Index;
    inline int HandleMinigameEnded_Index;
}

// Used to load a creative playset
inline UObject* (*LoadPlayset)(UPlaysetLevelStreamComponent* InPlaysetComponent);
inline uint64 (*UpdateVolumeState)(AFortVolume*, uint8 NewState);
inline void (*ResetLevelRecord)(UFortLevelSaveComponent*);