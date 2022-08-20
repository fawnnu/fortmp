#include "include.hpp"
#include "loader_component.hpp"

#include "elimination_component.hpp"
#include "looting_component.hpp"
#include "net_component.hpp"
#include "pickup_component.hpp"

loader_component::loader_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    REGISTER_FUNCTION_DETOUR(loader_component_hooks::UpdateLevelStreaming, memory_manager::find_function_by_string(L"STAT_UpdateLevelStreamingTime"), loader_component_hooks::UpdateLevelStreaming_Hook);
    REGISTER_FUNCTION_DETOUR(loader_component_hooks::UpdateLevelStreaming, memory_manager::find_pattern("E8 ? ? ? ? 48 8B CF E8 ? ? ? ? 84 C0 75 0E FF 15 ? ? ? ? 84 C0 0F 84 ? ? ? ?", true, 1), loader_component_hooks::UpdateLevelStreaming_Hook);
    
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
            if (in_playlist)
            {
                spdlog::critical(L"Could not set playlist '{}', as playlist data pointer is invalid",
                    in_playlist->GetName().c_str());
                
                return;
            }
        }
        
        if ((*GWorld)->AuthorityGameMode()->GameSession())
        {
            // Update max player count
            (*GWorld)->AuthorityGameMode()->GameSession()->MaxPlayers() = 101;
        }

        static auto FortWeaponItemDefinition = FindObject(L"/Script/FortniteGame.FortWeaponItemDefinition");
        static auto FortTrapItemDefinition = FindObject(L"/Script/FortniteGame.FortTrapItemDefinition");
        
        // All floor loot actors in the sub-level
        TArray<AActor*> floor_loot_actors = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics")->GetAllActorsOfClass(*GWorld, FindObject(L"Tiered_Athena_FloorLoot_01_C", false, true));
        
        for (AActor* floor_loot_actor : floor_loot_actors)
        {
            std::map<UFortItemDefinition*, int> loot_to_spawn = looting_component::spawn_loot(L"Loot_AthenaFloorLoot");

            for (const auto& [ item_definition, count ] : loot_to_spawn)
            {
                // Spawn pickup
                
                // set loaded ammo to spawn the pickup with
                int loaded_ammo = 0;

                if (UKismetMathLibrary::ClassIsChildOf(item_definition->Class, FortWeaponItemDefinition) &&
                    !UKismetMathLibrary::ClassIsChildOf(item_definition->Class, FortTrapItemDefinition))
                {
                    auto Map = *reinterpret_cast<TMap<FName, FFortBaseWeaponStats*>*>(__int64(static_cast<UFortWeaponItemDefinition*>(item_definition)->WeaponStatHandle().Table) + 0x30);
                    loaded_ammo = Map[static_cast<UFortWeaponItemDefinition*>(item_definition)->WeaponStatHandle().RowName].Value->ClipSize();
                }
                
                pickup_component::spawn_pickup(item_definition, floor_loot_actor->K2_GetActorLocation(), count, loaded_ammo);
            }

            // Set as searched if needed
            UKismetSystemLibrary::SetBoolPropertyByName(floor_loot_actor, L"bAlreadySearched", static_cast<ABuildingContainer*>(floor_loot_actor)->bStartAlreadySearched_Athena());

            // Actor might be dormant
            floor_loot_actor->ForceNetUpdate();
        }

        // All warmup floor loot actors in the sub-level
        TArray<AActor*> warmup_floor_loot_actors = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics")->GetAllActorsOfClass(*GWorld, FindObject(L"Tiered_Athena_FloorLoot_Warmup_C", false, true));
        
        for (AActor* floor_loot_actor : warmup_floor_loot_actors)
        {
            std::map<UFortItemDefinition*, int> loot_to_spawn = looting_component::spawn_loot(L"Loot_AthenaFloorLoot_Warmup");

            for (const auto& [ item_definition, count ] : loot_to_spawn)
            {
                // Spawn pickup
                
                // set loaded ammo to spawn the pickup with
                int loaded_ammo = 0;

                if (UKismetMathLibrary::ClassIsChildOf(item_definition->Class, FortWeaponItemDefinition) &&
                    !UKismetMathLibrary::ClassIsChildOf(item_definition->Class, FortTrapItemDefinition))
                {
                    auto Map = *reinterpret_cast<TMap<FName, FFortBaseWeaponStats*>*>(__int64(static_cast<UFortWeaponItemDefinition*>(item_definition)->WeaponStatHandle().Table) + 0x30);
                    loaded_ammo = Map[static_cast<UFortWeaponItemDefinition*>(item_definition)->WeaponStatHandle().RowName].Value->ClipSize();
                }
                
                pickup_component::spawn_pickup(item_definition, floor_loot_actor->K2_GetActorLocation(), count, loaded_ammo);
            }

            // Set as searched if needed
            UKismetSystemLibrary::SetBoolPropertyByName(floor_loot_actor, L"bAlreadySearched", static_cast<ABuildingContainer*>(floor_loot_actor)->bStartAlreadySearched_Athena());

            // Actor might be dormant
            floor_loot_actor->ForceNetUpdate();
        }

        // Infinite warmup
        (*GWorld)->AuthorityGameMode()->WarmupCountdownDuration() = 600.f;
        (*GWorld)->AuthorityGameMode()->WarmupEarlyCountdownDuration() = 600.f;
        
        // Start match
        (*GWorld)->AuthorityGameMode()->StartMatch();

        // Update default pawn class
        (*GWorld)->AuthorityGameMode()->DefaultPawnClass() = FindObject(L"/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");

        // Server is ready
        is_server_ready = true;

        // Remove the server player from AlivePlayers on older builds
        if (GetEngineVersionEasy() < 421)
        {
            APlayerController* player_controller = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics")->GetPlayerController(*GWorld, 0);

            RemoveFromAlivePlayerList((*GWorld)->AuthorityGameMode(), player_controller, 0);
        }
        
        spdlog::info(L"Match started in world '{}' and listening for connections",
            (*GWorld)->GetName().c_str());
    }
}

void loader_component_hooks::UpdateLevelStreaming_Hook(UWorld* World)
{
    // Call original function first
    UpdateLevelStreaming(World);
    
    if (World->GameState())
    {
        if (World->GameState()->WorldManager())
        {
            if (World->AuthorityGameMode())
            {
                // allows vehicles to spawn
                if (static bool b_has_property = (*GWorld)->AuthorityGameMode()->HasProperty(L"bMapInfoInitialized"))
                {
                    bool& bMapInfoInitialized = (*GWorld)->AuthorityGameMode()->bMapInfoInitialized();
                    
                    if (!bMapInfoInitialized)
                    {
                        bMapInfoInitialized = true;
                    }
                }
            }

            for (auto& streamed_level : TArrayManager<FFortLevelStreamingInfo>(&World->GameState()->WorldManager()->StreamedLevels(), FFortLevelStreamingInfo::GetStructSize()))
            {
                if (streamed_level.LevelState == 0)
                {
                    // Return if any level is unloaded
                    return;
                }
            }

            if (World->GameState()->WorldManager()->StreamedLevels().Num())
            {
                // Start match if we have enough loaded levels
                loader_component::start_match();
            }
        }
    }
}