#include "include.hpp"
#include "creative_component.hpp"

#include "inventory_component.hpp"

creative_component::creative_component()
{
    
    REGISTER_CALL_DETOUR(L"/Script/FortniteGame.FortAthenaCreativePortal.TeleportPlayerToLinkedVolume", creative_component_hooks::TeleportPlayerToLinkedVolume_Hook);
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    REGISTER_UFUNCTION_DETOUR(creative_component_hooks::ServerGiveCreativeItem, L"/Script/FortniteGame.FortPlayerControllerAthena.ServerGiveCreativeItem", creative_component_hooks::ServerGiveCreativeItem_Hook);
    REGISTER_UFUNCTION_DETOUR(creative_component_hooks::ServerTeleportToPlaygroundLobbyIsland, L"/Script/FortniteGame.FortPlayerControllerAthena.ServerTeleportToPlaygroundLobbyIsland", creative_component_hooks::ServerTeleportToPlaygroundLobbyIsland_Hook);
    
    DetourTransactionCommit();
}

void creative_component::handle_player_join(UNetConnection* NetConnection)
{
    UFortPlaylistAthena* current_playlist = (*GWorld)->GameState()->HasProperty("CurrentPlaylistData") ?
            (*GWorld)->GameState()->CurrentPlaylistData() :
            (*GWorld)->GameState()->HasProperty("CurrentPlaylistInfo") ?
            (*GWorld)->GameState()->CurrentPlaylistInfo().BasePlaylist() :
            nullptr;

    if (current_playlist == nullptr)
    {
        spdlog::error(L"creative_component::handle_player_join: nullptr playlist");

        return;
    }
    
    if (current_playlist->HasProperty(L"bEnableCreativeMode") &&
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
        player_controller_athena->OwnedPortal() = creative_portal;
        creative_portal->CachedOwningPlayerState() = NetConnection->PlayerController()->PlayerState();
        
        creative_portal->SetOwner(player_controller_athena);
        creative_portal->OwningPlayerChanged();
        creative_portal->OnRep_OwningPlayer();
        
        // Mark player as ready to use the portal
        creative_portal->PlayersReady().Add(NetConnection->PlayerID());
        creative_portal->OnRep_PlayersReady();
        
        // Open the portal
        creative_portal->bPortalOpen() = true;
        creative_portal->bDisallowPortalInteract() = false;
        creative_portal->PortalOpenChanged();

        // Mark portal as used so it doesn't get assigned to another player
        creative_portal_manager->MarkPortalUsed(creative_portal);

        spdlog::info(L"Assigned and initialized a creative portal for '{}'", player_controller_athena->MyPlayerInfo()->PlayerName().c_str());

        // Get the Playset needed for a certain real estate
        if (UFortCreativeRealEstatePlotItemDefinition* Plot = FindObject<UFortCreativeRealEstatePlotItemDefinition>(L"/Game/Playgrounds/Items/Plots/Temperate_Medium.Temperate_Medium", true))
        {
            UFortPlaysetItemDefinition* Playset = Plot->BasePlayset().Get();
        
            creative_portal->LinkedVolume()->SetCurrentPlayset(Playset);

            creative_portal->LinkedVolume()->OnRep_CurrentPlayset();

            // Now get this volume's playset component and initialize the island
            for (UActorComponent* Component : creative_portal->LinkedVolume()->BlueprintCreatedComponents())
            {
                // Prevent a crash on later builds and when more people are joining
                if(Component != nullptr)
                {
                 if (UKismetMathLibrary::ClassIsChildOf(Component->Class, UPlaysetLevelStreamComponent::StaticClass()))
                {
                    UPlaysetLevelStreamComponent* PlaysetComponent = static_cast<UPlaysetLevelStreamComponent*>(Component);

                    PlaysetComponent->SetPlayset(creative_portal->LinkedVolume()->CurrentPlayset());

                    // Create new streaming data
                    FFortPlaysetStreamingData* NewPlaysetStreamingData = static_cast<FFortPlaysetStreamingData*>(malloc(FFortPlaysetStreamingData::GetStructSize()));

                    ZeroMemory(NewPlaysetStreamingData, FFortPlaysetStreamingData::GetStructSize());

                    // Create the island
                    creative_portal->LinkedVolume()->CurrentPlayset()->SpawnPlayset(*GWorld, creative_portal->LinkedVolume()->CurrentPlayset(), creative_portal->LinkedVolume()->K2_GetActorLocation(), FRotator(), NewPlaysetStreamingData);

                    // Initialize the component
                    PlaysetComponent->ClientLoadedPackageName() = PlaysetComponent->ClientPlaysetData().PackageName();
                
                    memcpy(&PlaysetComponent->ClientPlaysetData(), NewPlaysetStreamingData, FFortPlaysetStreamingData::GetStructSize());

                    // OnRep
                    PlaysetComponent->OnRep_ClientPlaysetData();

                    // Log
                    spdlog::info(L"Created and loaded Island ('{}') for portal '{}'", Plot->GetName().c_str(), creative_portal->GetName().c_str());

                    return;
                }

                else if (UKismetMathLibrary::ClassIsChildOf(Component->Class, UFortLevelSaveComponent::StaticClass()))
                {
                    UFortLevelSaveComponent* LevelSaveComponent = static_cast<UFortLevelSaveComponent*>(Component);

                    LevelSaveComponent->AccountIdOfOwner() = creative_portal->OwningPlayer();
                    
                    if(LevelSaveComponent->HasProperty(L"PlotPermissions"))
                    {
                        /* @ Private = 0
                         * @ Public = 1 */
                        LevelSaveComponent->PlotPermissions().Permission() = 1;
                        LevelSaveComponent->OnRep_PlotPermissionsChanged();
                    }
                }   
                }
            }
        }
    }
}

void creative_component_hooks::TeleportPlayerToLinkedVolume_Hook(AFortAthenaCreativePortal* Portal, AFortPlayerPawn* PlayerPawn, bool bUseSpawnTags)
{
    if (Portal->LinkedVolume() == nullptr || PlayerPawn == nullptr)
    {
        spdlog::error(L"creative_component_hooks::TeleportPlayerToLinkedVolume_Hook: Invalid LinkedVolume or PlayerPawn");
        
        return;
    }
    
    PlayerPawn->K2_TeleportTo(Portal->LinkedVolume()->K2_GetActorLocation(), FRotator());
    
    static_cast<AFortPlayerPawnAthena*>(PlayerPawn)->TeleportToSkyDive(15000);

    reinterpret_cast<AFortPlayerControllerAthena*>(PlayerPawn->Controller())->CreativePlotLinkedVolume() = Portal->LinkedVolume();
    reinterpret_cast<AFortPlayerControllerAthena*>(PlayerPawn->Controller())->OnRep_CreativePlotLinkedVolume();

    // Add to the array
    for (UActorComponent* Component : Portal->LinkedVolume()->BlueprintCreatedComponents())
    {
        if (UKismetMathLibrary::ClassIsChildOf(Component->Class, UPlaysetLevelStreamComponent::StaticClass()))
        {
            static_cast<UPlaysetLevelStreamComponent*>(Component)->PlayersWithValidPlaysetData().Add(PlayerPawn->Controller()->NetConnection()->PlayerID());
            
            // Finish loading
            static_cast<UPlaysetLevelStreamComponent*>(Component)->OnFinishedLoading();
        }
    }
    
    Portal->NotifyTeleportedPlayerPawn(PlayerPawn);
}

void creative_component_hooks::ServerGiveCreativeItem_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(FFortItemEntry, CreativeItem);

    if (CreativeItem.Count() > 0)
    {
        inventory_component::add_item(FortPlayerController, CreativeItem.ItemDefinition(), CreativeItem.Count(), CreativeItem.LoadedAmmo());   
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

        if(FortPlayerControllerAthena->CreativePlotLinkedVolume())
        {
            // Remove from players with valid playset data
            for (UActorComponent* Component : FortPlayerControllerAthena->CreativePlotLinkedVolume()->BlueprintCreatedComponents())
            {
                if(Component != nullptr)
                {
                    if (UKismetMathLibrary::ClassIsChildOf(Component->Class, UPlaysetLevelStreamComponent::StaticClass()))
                    {
                        TArrayManager<FUniqueNetIdRepl> PlayersWithValidPlaysetData = TArrayManager<FUniqueNetIdRepl>(&static_cast<UPlaysetLevelStreamComponent*>(Component)->PlayersWithValidPlaysetData(), sizeof FUniqueNetIdRepl);

                        for (int i = 0; i < PlayersWithValidPlaysetData.Num(); i++)
                        {
                            if (UFortKismetLibrary::EqualEqual_UniqueNetIdReplUniqueNetIdRepl(PlayersWithValidPlaysetData[i], FortPlayerControllerAthena->NetConnection()->PlayerID()))
                            {
                                PlayersWithValidPlaysetData.Remove(i, FindPropertyByName(Component->Class, L"PlayersWithValidPlaysetData"));
                            }
                        }
                    }
                }
            }   
        }
        
        // Unset linked volume
        FortPlayerControllerAthena->CreativePlotLinkedVolume() = nullptr;

        FortPlayerControllerAthena->OnRep_CreativePlotLinkedVolume();
    }
    else
    {
        spdlog::error(L"creative_component_hooks::ServerTeleportToPlaygroundLobbyIsland_Hook: Invalid PlayerStart");
    }

    return ServerTeleportToPlaygroundLobbyIsland(FortPlayerControllerAthena, StackFrame);
}