#include "include.hpp"
#include "creative_component.hpp"

#include "inventory_component.hpp"
#include "looting_component.hpp"

creative_component::creative_component()
{
    
    REGISTER_CALL_DETOUR(L"/Script/FortniteGame.FortAthenaCreativePortal.TeleportPlayerToLinkedVolume", creative_component_hooks::TeleportPlayerToLinkedVolume_Hook);
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    REGISTER_UFUNCTION_DETOUR(creative_component_hooks::ServerGiveCreativeItem, L"/Script/FortniteGame.FortPlayerControllerAthena.ServerGiveCreativeItem", creative_component_hooks::ServerGiveCreativeItem_Hook);
    REGISTER_FUNCTION_DETOUR(creative_component_hooks::AddMiniGamePlayer, memory_manager::find_function_by_string(L".AddMinigamePlayer - %s added"), creative_component_hooks::AddMiniGamePlayer_Hook);
    REGISTER_FUNCTION_DETOUR(creative_component_hooks::RemoveMiniGamePlayer, memory_manager::find_function_by_string(L".RemoveMinigamePlayer - %s removed"), creative_component_hooks::RemoveMiniGamePlayer_Hook);
    REGISTER_UFUNCTION_DETOUR(creative_component_hooks::ServerTeleportToPlaygroundLobbyIsland, L"/Script/FortniteGame.FortPlayerControllerAthena.ServerTeleportToPlaygroundLobbyIsland", creative_component_hooks::ServerTeleportToPlaygroundLobbyIsland_Hook);
    INIT_POINTER(LoadPlayset, memory_manager::find_function_by_string(L"UPlaysetLevelStreamComponent::LoadPlayset Error: no owner for %s"));
    
    DetourTransactionCommit();
}

void creative_component_hooks::AddMiniGamePlayer_Hook(AFortMiniGame* MiniGame, FFrame& StackFrame)
{
    if(MiniGame->Volume())
    {
        if(MiniGame->Volume()->LinkedPortals()[0])
        {
            MiniGame->Volume()->LinkedPortals()[0]->CurrentPopulation()++;
            spdlog::info(L"creative_component:AddMiniGamePlayer_Hook: Updating Island population {}", MiniGame->Volume()->LinkedPortals()[0]->CurrentPopulation());
        }
    }
}

void creative_component_hooks::RemoveMiniGamePlayer_Hook(AFortMiniGame* MiniGame, FFrame& StackFrame)
{
    if(MiniGame->Volume())
    {
        if(MiniGame->Volume()->LinkedPortals()[0])
        {
            MiniGame->Volume()->LinkedPortals()[0]->CurrentPopulation()--;
            spdlog::info(L"creative_component:RemoveMiniGamePlayer_Hook: Updating Island population {}", MiniGame->Volume()->LinkedPortals()[0]->CurrentPopulation());
        }
    }
}

bool creative_component::is_creative_enabled()
{
    UFortPlaylistAthena* current_playlist = GetPlaylistEasy();

    if (current_playlist == nullptr)
    {
        return false;
    }

    STATIC_HASPROPERTY(current_playlist, bEnableCreativeMode);
    if (bExists_bEnableCreativeMode &&
        current_playlist->bEnableCreativeMode() == true)
    {
        // Assign portal to player
        return true;
    }

    return false;
}

void creative_component::handle_player_join(UNetConnection* NetConnection)
{
    UFortPlaylistAthena* current_playlist = GetPlaylistEasy();

    if (current_playlist == nullptr)
    {
        return;
    }

    STATIC_HASPROPERTY(current_playlist, bEnableCreativeMode);
    if (bExists_bEnableCreativeMode &&
        current_playlist->bEnableCreativeMode() == true)
    {
        // Assign portal to player
        assign_portal_to_player(NetConnection, 0);
    }
}

void creative_component::assign_portal_to_player(UNetConnection* NetConnection, int PortalIndex)
{
    AFortCreativePortalManager* creative_portal_manager = (*GWorld)->GameState()->CreativePortalManager();

    if (creative_portal_manager != nullptr)
    {
        AFortAthenaCreativePortal* creative_portal = creative_portal_manager->AvailablePortals()[PortalIndex];

        AFortPlayerControllerAthena* player_controller_athena = reinterpret_cast<AFortPlayerControllerAthena*>(NetConnection->PlayerController());
        
        /* @ Setting the portal owner ship.
         * @ Allows the player to interact in the portal and teleport through it once the stripped functions are re-implemented.*/
        creative_portal->OwningPlayer() = NetConnection->PlayerID();

        // Mark player as ready to use the portal
        creative_portal->PlayersReady().Add(NetConnection->PlayerID());

        // Open the portal
        creative_portal->bPortalOpen() = true;
        creative_portal->bDisallowPortalInteract() = false;
        
        player_controller_athena->OwnedPortal() = creative_portal;
        
        creative_portal->CachedOwningPlayerState() = NetConnection->PlayerController()->PlayerState();
        creative_portal->OwningPlayerChanged();
        creative_portal->OnRep_OwningPlayer();
        creative_portal->OnRep_PlayersReady();
        creative_portal->PortalOpenChanged();
        
        // Mark portal as used so it doesn't get assigned to another player
        creative_portal_manager->MarkPortalUsed(creative_portal);
        
        creative_portal->SetOwner(player_controller_athena);

      //  creative_portal->ImageUrl() = L"";
        
        creative_portal->ClientSaveStateChanged(false);
        creative_portal->ClientSaveDisplayChanged(false);
        
        spdlog::info(L"Assigned and initialized a creative portal for '{}'", player_controller_athena->MyPlayerInfo()->PlayerName().c_str());

        // Get the Playset needed for a certain real estate
        if (UFortCreativeRealEstatePlotItemDefinition* Plot = FindObject<UFortCreativeRealEstatePlotItemDefinition>(L"/Game/Playgrounds/Items/Plots/Temperate_Medium.Temperate_Medium", true))
        {
            UFortItem* PlotInstance = Plot->CreateTemporaryItemInstanceBP(1, 1);
            
            UFortPlaysetItemDefinition* Playset = Plot->BasePlayset().Get();
            
            // Now get this volume's playset component and initialize the island
            for (UActorComponent* Component : creative_portal->LinkedVolume()->BlueprintCreatedComponents())
            {
                // Prevent a crash on later builds and when more people are joining
                if (Component)
                {
                    if (Component->IsA(UPlaysetLevelStreamComponent::StaticClass()))
                    {
                        UPlaysetLevelStreamComponent* PlaysetComponent = static_cast<UPlaysetLevelStreamComponent*>(Component);
                        
                        PlaysetComponent->SetPlayset(Playset);
                        PlaysetComponent->CurrentPlayset() = Playset;

                        LoadPlayset(PlaysetComponent);

                        creative_portal->LinkedVolume()->SetCurrentPlayset(Playset);
                        creative_portal->LinkedVolume()->CurrentPlayset() = Playset;
                        creative_portal->LinkedVolume()->OverridePlayset() = Playset;
                        creative_portal->LinkedVolume()->OnRep_CurrentPlayset();
                        creative_portal->LinkedVolume()->VolumeState() = 3;
                        creative_portal->LinkedVolume()->OnRep_VolumeState();
                        creative_portal->LinkedVolume()->LinkedPortals().Add(creative_portal);
                        
                        (*GWorld)->GameState()->VolumeManager()->UpdateVolumeLevelStreams();

                        // TODO: Proper
                        auto NewVolume = (*GWorld)->GameState()->VolumeManager()->SpawnVolume(FindObject(L"/Game/Athena/BuildingActors/FortVolumeActor_PrefabGrenades.FortVolumeActor_PrefabGrenades_C", true), Playset, creative_portal->LinkedVolume()->K2_GetActorLocation(), creative_portal->LinkedVolume()->K2_GetActorRotation());
                        if (NewVolume) NewVolume->K2_DestroyActor();
                        
                        // Log
                        spdlog::info(L"Created and loaded Island ('{}') for portal '{}'", Plot->GetName().c_str(), creative_portal->GetName().c_str());

                        return;
                    }
                    if (Component->IsA(UFortLevelSaveComponent::StaticClass()))
                    {
                        UFortLevelSaveComponent* LevelSaveComponent = static_cast<UFortLevelSaveComponent*>(Component);

                        LevelSaveComponent->AccountIdOfOwner() = creative_portal->OwningPlayer();
                    }
                }
            }
        }
    }
}

void creative_component_hooks::TeleportPlayerToLinkedVolume_Hook(AFortAthenaCreativePortal* Portal, AFortPlayerPawn* PlayerPawn, bool bUseSpawnTags)
{
    if (Portal)
    {
        if (STATIC_HASPROPERTY(Portal, LinkedVolume))
        {
            if (Portal == nullptr || memory_manager::is_bad_read_pointer(Portal) || Portal->LinkedVolume() == nullptr || memory_manager::is_bad_read_pointer(Portal->LinkedVolume()) || PlayerPawn == nullptr)
            {
                spdlog::error(L"creative_component_hooks::TeleportPlayerToLinkedVolume_Hook: Invalid LinkedVolume or PlayerPawn");
        
                return;
            }
    
            PlayerPawn->K2_TeleportTo(Portal->LinkedVolume()->K2_GetActorLocation(), FRotator());
            
            static_cast<AFortPlayerPawnAthena*>(PlayerPawn)->TeleportToSkyDive(18000);

            static_cast<AFortPlayerControllerAthena*>(PlayerPawn->Controller())->CreativePlotLinkedVolume() = Portal->LinkedVolume();
            static_cast<AFortPlayerControllerAthena*>(PlayerPawn->Controller())->OnRep_CreativePlotLinkedVolume();
    
            Portal->NotifyTeleportedPlayerPawn(PlayerPawn);
            Portal->OnPlayerPawnTeleported(PlayerPawn);
        }
    }
}

void creative_component_hooks::ServerGiveCreativeItem_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(FFortItemEntry, CreativeItem);

    if (CreativeItem.Count() && CreativeItem.ItemDefinition())
    {
        inventory_component::add_item(FortPlayerController, CreativeItem.ItemDefinition(), CreativeItem.Count(), looting_component::get_default_loaded_ammo(CreativeItem.ItemDefinition()));   
    }
    
    return ServerGiveCreativeItem(FortPlayerController, StackFrame);
}

void creative_component_hooks::ServerTeleportToPlaygroundLobbyIsland_Hook(AFortPlayerControllerAthena* FortPlayerControllerAthena, FFrame& StackFrame)
{
    if (FortPlayerControllerAthena == nullptr || FortPlayerControllerAthena->Pawn() == nullptr)
    {
        spdlog::error(L"creative_component_hooks::ServerTeleportToPlaygroundLobbyIsland_Hook: Invalid PlayerController or Pawn.");
        
        return ServerTeleportToPlaygroundLobbyIsland(FortPlayerControllerAthena, StackFrame);
    }
    
    FortPlayerControllerAthena->PreLobbyIslandTeleport();

    AActor* PlayerStart = (*GWorld)->AuthorityGameMode()->ChoosePlayerStart(FortPlayerControllerAthena);

    if (PlayerStart)
    {
        FortPlayerControllerAthena->Pawn()->K2_TeleportTo(PlayerStart->K2_GetActorLocation(), PlayerStart->K2_GetActorRotation());
        
        FortPlayerControllerAthena->PostLobbyIslandTeleport(FortPlayerControllerAthena->Pawn());

        // Clear inventory when going to hub to make sure player doesnt have creative phone and any sort of items
        inventory_component::clear_primary_inventory(FortPlayerControllerAthena, false);
        
        // Unset linked volume
        FortPlayerControllerAthena->CreativePlotLinkedVolume() = nullptr;

        FortPlayerControllerAthena->OnRep_CreativePlotLinkedVolume();
        
        // Equip pickaxe
        FortPlayerControllerAthena->ServerExecuteInventoryItem(inventory_component::get_pickaxe(FortPlayerControllerAthena)->ItemEntry().ItemGuid());
        
        FortPlayerControllerAthena->ServerCreativeStopFly();
        FortPlayerControllerAthena->ClientCreativeStopFly();
    }
    else
    {
        spdlog::error(L"creative_component_hooks::ServerTeleportToPlaygroundLobbyIsland_Hook: Invalid PlayerStart");
    }

    return ServerTeleportToPlaygroundLobbyIsland(FortPlayerControllerAthena, StackFrame);
}