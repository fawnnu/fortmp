#include "include.hpp"
#include "loader_component.hpp"

#include "looting_component.hpp"
#include "net_component.hpp"
#include "pickup_component.hpp"

loader_component::loader_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    REGISTER_FUNCTION_DETOUR(loader_component_hooks::ReadyToStartMatch, memory_manager::find_function_by_string(L"GameMode returned ReadyToStartMatch", {0xe8}, 1), loader_component_hooks::ReadyToStartMatch_Hook);
    REGISTER_FUNCTION_DETOUR(loader_component_hooks::UpdateLevelStreaming, memory_manager::find_function_by_string(L"STAT_UpdateLevelStreamingTime"), loader_component_hooks::UpdateLevelStreaming_Hook);
    
    DetourTransactionCommit();
}

void loader_component::start_match(UFortPlaylistAthena* in_playlist)
{
    constexpr int listen_port = 7777;
    
    // Listen for connections
    const FURL in_url((*GWorld)->GetName().c_str(), listen_port);

    //make sure we weren't listening already
    if (net_component::listen(in_url))
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

        // Update default pawn class
        (*GWorld)->AuthorityGameMode()->DefaultPawnClass() = FindObject(L"/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");

        //log
        spdlog::info(L"Match started in world '{}' and listening for connections",
            (*GWorld)->GetName().c_str());
    }
}

bool loader_component_hooks::ReadyToStartMatch_Hook(AFortGameModeAthena* GameMode)
{
    //Make sure warmup is infinite before startmatch is called!
    GameMode->WarmupCountdownDuration() = 99999999.f;
    GameMode->WarmupEarlyCountdownDuration() = 99999999.f;
    
    return true; //Automatically starts match
}

void loader_component_hooks::UpdateLevelStreaming_Hook(UWorld* World)
{
    //Call the c++ function before executing our code
    UpdateLevelStreaming(World);

    //make sure we have a valid gamestate
    if (World->GameState())
    {
        //make sure we have a valid worldmanager
        if (World->GameState()->WorldManager())
        {
            //loop through all streamed levels to make sure none is unloaded
            for (auto& streamed_level : TArrayManager(World->GameState()->WorldManager()->StreamedLevels(), FFortLevelStreamingInfo::GetStructSize()))
            {
                //check if the level is unloaded
                if (streamed_level.LevelState == 0)
                {
                    //if it is, return
                    return;
                }
            }

            //make sure we have enough loaded levels and start match (listen for connections, spawn floor loot, set playlist)
            if (World->GameState()->WorldManager()->StreamedLevels().Num())
            {
                loader_component::start_match();
            }
        }
    }
}

