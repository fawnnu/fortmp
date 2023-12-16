#pragma once

class creative_component
{
public:

    // Init
    creative_component();

    // Tells if we are currently playing creative
    static bool is_creative_enabled();
    
    // Assign a portal to the joining player
    static void assign_portal_to_player(UNetConnection* NetConnection, int PortalIndex);
    
    // Handle everything that has to be done once a player joins the match
    static void handle_player_join(UNetConnection* NetConnection);
};

namespace creative_component_hooks
{
    // Called when a player enters a creative portal
    DEFINE_FUNCTION_HOOK(void, TeleportPlayerToLinkedVolume, (AFortAthenaCreativePortal* Portal, AFortPlayerPawn* PlayerPawn, bool bUseSpawnTags));

    // Called when granting an item via the creative inventory
    DEFINE_FUNCTION_HOOK(void, ServerGiveCreativeItem, (AFortPlayerController* FortPlayerController, FFrame& StackFrame));
    
    DEFINE_FUNCTION_HOOK(void, AddMiniGamePlayer, (AFortMiniGame* MiniGame, FFrame& StackFrame));
    
    DEFINE_FUNCTION_HOOK(void, RemoveMiniGamePlayer, (AFortMiniGame* MiniGame, FFrame& StackFrame));
    
    // Called when trying to go back to hub
    DEFINE_FUNCTION_HOOK(void, ServerTeleportToPlaygroundLobbyIsland, (AFortPlayerControllerAthena* FortPlayerControllerAthena, FFrame& StackFrame));

    DEFINE_FUNCTION_HOOK(void, OnFinishedRecordRead, (UObject*, UObject*, void*));
    DEFINE_FUNCTION_HOOK(void, OnFinishedRecordWrite, (UObject*, UObject*, void*));
    DEFINE_FUNCTION_HOOK(void, OnFinishedLoading, (UObject*, UObject*, void*));
    DEFINE_FUNCTION_HOOK(void, SaveOutLevelRecord, (UFortLevelSaveComponent* Self))
    inline int SaveOutLevelRecord_Index;
}

// Used to load a creative playset
inline UObject* (*LoadPlayset)(UPlaysetLevelStreamComponent* InPlaysetComponent);