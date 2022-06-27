#include "include.hpp"
#include "loader_component.hpp"

#include "looting_component.hpp"
#include "net_component.hpp"
#include "pickup_component.hpp"

loader_component::loader_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    REGISTER_FUNCTION_DETOUR(loader_component_hooks::SetLoadedLevel, memory_manager::find_function_by_string(L"STAT_OnLevelLoaded_Broadcast"), loader_component_hooks::SetLoadedLevel_Hook);
    
    DetourTransactionCommit();
}

void loader_component::start_match(UFortPlaylistAthena* in_playlist)
{
    if (in_playlist == nullptr)
    {
        // Use default playlist if none specified
        in_playlist = FindObject<UFortPlaylistAthena>(L"/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo");
    }

    if ((*GWorld)->GameState()->HasProperty("CurrentPlaylistData"))
    {
        (*GWorld)->GameState()->CurrentPlaylistData() = in_playlist;

        // Load playlist on the server
        (*GWorld)->GameState()->OnRep_CurrentPlaylistData();
    }
    else if ((*GWorld)->GameState()->HasProperty("CurrentPlaylistInfo"))
    {
        (*GWorld)->GameState()->CurrentPlaylistInfo().BasePlaylist() = in_playlist;
        (*GWorld)->GameState()->CurrentPlaylistInfo().OverridePlaylist() = in_playlist;
        (*GWorld)->GameState()->CurrentPlaylistInfo().PlaylistReplicationKey()++;

        // Load playlist on the server
        (*GWorld)->GameState()->OnRep_CurrentPlaylistInfo();
    }
    else
    {
        spdlog::critical(L"Could not set playlist '{}', as playlist data pointer is invalid",
            in_playlist->GetName().c_str());
        
        return;
    }

    // All floor loot actors in the sub-level
    TArray<AActor*> floor_loot_actors = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics")->GetAllActorsOfClass(*GWorld, FindObject(L"Tiered_Athena_FloorLoot_01_C", false, true));
        
    for (AActor* floor_loot_actor : floor_loot_actors)
    {
        // TODO: Roll floor loot
        std::map<UFortItemDefinition*, int> loot_to_spawn = looting_component::spawn_loot(L"Loot_AthenaFloorLoot");

        for (const auto& [ item_definition, count ] : loot_to_spawn)
        {
            // Spawn pickup
            pickup_component::spawn_pickup(item_definition, floor_loot_actor->K2_GetActorLocation(), count);
        }
    }
    
    // Infinite warmup
    (*GWorld)->AuthorityGameMode()->WarmupCountdownDuration() = 99999999.f;
    (*GWorld)->AuthorityGameMode()->WarmupEarlyCountdownDuration() = 99999999.f;

    constexpr int listen_port = 7777;
    
    // Listen for connections
    const FURL in_url((*GWorld)->GetName().c_str(), listen_port);
                    
    if (net_component::listen(in_url))
    {
        spdlog::info(L"Match started in world '{}' and listening for connections",
            (*GWorld)->GetName().c_str());
    }
    else
    {
        spdlog::critical(L"Failed to start listening in world '{}' on port '{}'",
            (*GWorld)->GetName().c_str(),
            listen_port);
    }

    // Update default pawn class
    (*GWorld)->AuthorityGameMode()->DefaultPawnClass() = FindObject(L"/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");
}

// Little hack to prevent start match from being run more than once
bool bReadyToStartMatch = false;

void loader_component_hooks::SetLoadedLevel_Hook(ULevelStreaming* LevelStreaming, UObject* Level)
{
    // Add to loaded levels
    loader_component::loaded_levels.push_back(LevelStreaming->PackageNameToLoad());
    
    if (loader_component::loaded_levels.size() == loader_component::loaded_levels_threshold)
    {
        if (!bReadyToStartMatch)
        {
            bReadyToStartMatch = true;

            // Start match
            loader_component::start_match();
        }
    }

    return SetLoadedLevel(LevelStreaming, Level);
}