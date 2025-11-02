#include "include.hpp"
#include "creative_component.hpp"

#include "function_component.hpp"
#include "inventory_component.hpp"
#include "log_component.hpp"
#include "looting_component.hpp"
#include "pickup_component.hpp"
#include "player_component.hpp"

creative_component::creative_component()
{
    INIT_POINTER(UpdateVolumeState, memory_manager::find_pattern("40 53 48 83 EC 30 80 B9 ? ? ? ? ? 48 8B D9 75 31 88 91 ? ? ? ? 80 FA 03 75 0C E8 ? ? ? ? 0F B6 93 ? ? ? ?"))
    INIT_POINTER(ResetLevelRecord, memory_manager::find_function_by_string(L"ResetLevelRecord requested"))
    
    if (UFunction* SaveOutLevelRecord = FindObject<UFunction>(L"/Script/FortniteGame.LevelSaveBaseComponent.SaveOutLevelRecord"))
    {
        if (const uint64 Result = memory_manager::scan_for(reinterpret_cast<uintptr_t>(SaveOutLevelRecord->get_native_function()), {0x48, 0xFF, 0xA0}))
        {
            creative_component_hooks::SaveOutLevelRecord_Index = *reinterpret_cast<int*>(Result + 3) / 8;
            REGISTER_VTABLE_HOOK(CreateDefaultObject<UFortLevelSaveComponent>()->VTable, creative_component_hooks::SaveOutLevelRecord_Index, creative_component_hooks::SaveOutLevelRecord);
        }
    }

    if (UFunction* HandleMinigameStarted = FindObject<UFunction>(L"/Script/FortniteGame.FortMinigame.HandleMinigameStarted"))
    {
        if (const uint64 Result = memory_manager::scan_for(reinterpret_cast<uintptr_t>(HandleMinigameStarted->get_native_function()), {0x48, 0xFF, 0xA0}))
        {
            creative_component_hooks::HandleMinigameStarted_Index = *reinterpret_cast<int*>(Result + 3) / 8;
            REGISTER_VTABLE_HOOK(CreateDefaultObject<AFortMinigame>()->VTable, creative_component_hooks::HandleMinigameStarted_Index, creative_component_hooks::HandleMinigameStarted);
        }
    }

    if (UFunction* HandleMinigameEnded = FindObject<UFunction>(L"/Script/FortniteGame.FortMinigame.HandleMinigameEnded"))
    {
        if (const uint64 Result = memory_manager::scan_for(reinterpret_cast<uintptr_t>(HandleMinigameEnded->get_native_function()), {0x48, 0xFF, 0xA0}))
        {
            creative_component_hooks::HandleMinigameEnded_Index = *reinterpret_cast<int*>(Result + 3) / 8;
            REGISTER_VTABLE_HOOK(CreateDefaultObject<AFortMinigame>()->VTable, creative_component_hooks::HandleMinigameEnded_Index, creative_component_hooks::HandleMinigameEnded);
        }
    }
    
    REGISTER_POST_PROCESS_EVENT_DETOUR(L"/Script/FortniteGame.LevelSaveBaseComponent.OnFinishedRecordRead", creative_component_hooks::OnFinishedRecordRead_Hook);
    REGISTER_POST_PROCESS_EVENT_DETOUR(L"/Script/FortniteGame.LevelSaveBaseComponent.OnFinishedRecordWrite", creative_component_hooks::OnFinishedRecordWrite_Hook);
    REGISTER_POST_PROCESS_EVENT_DETOUR(L"/Script/FortniteGame.PlaysetLevelStreamComponent.OnFinishedLoading", creative_component_hooks::OnFinishedLoading_Hook);
    REGISTER_CALL_DETOUR(L"/Script/FortniteGame.FortAthenaCreativePortal.TeleportPlayerToLinkedVolume", creative_component_hooks::TeleportPlayerToLinkedVolume_Hook);
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    // Hook native entering/exited playspace (island) events to avoid people carrying inventories on the main island accidentally (due to fortnite's own bugs)
    if (UFunction* NativeEnteringPlayspaceEvent = FindObject<UFunction>(L"/Script/FortniteGame.FortMinigame.NativeEnteringPlayspaceEvent"))
    {
        if (const uint64 Result = memory_manager::scan_for(reinterpret_cast<uintptr_t>(NativeEnteringPlayspaceEvent->get_native_function()), {0xE8, 0x100, 0x100, 0x100, 0xFF}))
        {
            uint64 TargetFunction = Result + 5 + *reinterpret_cast<int32*>(Result + 1);
            REGISTER_FUNCTION_DETOUR(creative_component_hooks::NativeEnteringPlayspaceEvent, TargetFunction, creative_component_hooks::NativeEnteringPlayspaceEvent_Hook);
        }
    }
    if (UFunction* NativeExitedPlayspaceEvent = FindObject<UFunction>(L"/Script/FortniteGame.FortMinigame.NativeExitedPlayspaceEvent"))
    {
        if (const uint64 Result = memory_manager::scan_for(reinterpret_cast<uintptr_t>(NativeExitedPlayspaceEvent->get_native_function()), {0xE8, 0x100, 0x100, 0x100, 0xFF}))
        {
            uint64 TargetFunction = Result + 5 + *reinterpret_cast<int32*>(Result + 1);
            REGISTER_FUNCTION_DETOUR(creative_component_hooks::NativeExitedPlayspaceEvent, TargetFunction, creative_component_hooks::NativeExitedPlayspaceEvent_Hook);
        }
    }
    
    REGISTER_UFUNCTION_DETOUR(creative_component_hooks::ServerGiveCreativeItem, L"/Script/FortniteGame.FortPlayerControllerAthena.ServerGiveCreativeItem", creative_component_hooks::ServerGiveCreativeItem_Hook);
    REGISTER_FUNCTION_DETOUR(creative_component_hooks::AddMiniGamePlayer, memory_manager::find_function_by_string(L".AddMinigamePlayer - %s added"), creative_component_hooks::AddMiniGamePlayer_Hook);
    REGISTER_FUNCTION_DETOUR(creative_component_hooks::RemoveMiniGamePlayer, memory_manager::find_function_by_string(L".RemoveMinigamePlayer - %s removed"), creative_component_hooks::RemoveMiniGamePlayer_Hook);
    REGISTER_FUNCTION_DETOUR(creative_component_hooks::SetTeamForPlayer, memory_manager::find_pattern("48 89 6C 24 ? 56 48 83 EC 20 80 B9 ? ? ? ? ? 41 0F B6 F0 48 8B EA 75 6E 48 8B 01 FF 90 ? ? ? ? 48 85 C0 74 60"), creative_component_hooks::SetTeamForPlayer_Hook);
    REGISTER_UFUNCTION_DETOUR(creative_component_hooks::ServerTeleportToPlaygroundLobbyIsland, L"/Script/FortniteGame.FortPlayerControllerAthena.ServerTeleportToPlaygroundLobbyIsland", creative_component_hooks::ServerTeleportToPlaygroundLobbyIsland_Hook);
    REGISTER_UFUNCTION_DETOUR(creative_component_hooks::ResetMyCurrentCreativePlot, L"/Script/FortniteGame.FortPlayerControllerAthena.ResetMyCurrentCreativePlot", creative_component_hooks::ResetMyCurrentCreativePlot_Hook);
    REGISTER_UFUNCTION_DETOUR(creative_component_hooks::Server_SetCanEditCreativeIsland, L"/Script/FortniteGame.FortPlayerStateAthena.Server_SetCanEditCreativeIsland", creative_component_hooks::Server_SetCanEditCreativeIsland_Hook);
    REGISTER_UFUNCTION_DETOUR(creative_component_hooks::HandleVolumeStateChanged, L"/Script/FortniteGame.FortVolumeManager.HandleVolumeStateChanged", creative_component_hooks::HandleVolumeStateChanged_Hook);

    const UObject* DefaultFortPlayerControllerAthenaObject = FindObject(L"/Script/FortniteGame.Default__FortPlayerControllerAthena");
    if (const int validate_target_index = memory_manager::find_function_by_string(L"ServerLoadPlotForPortal_Validate", {0xFF, 0x90}, 2, true, false))
    {
        REGISTER_VTABLE_HOOK(DefaultFortPlayerControllerAthenaObject->VTable, validate_target_index / 8, creative_component_hooks::ServerLoadPlotForPortal_Validate)
    }
    if (const int impl_target_index = memory_manager::find_function_by_string(L"ServerLoadPlotForPortal_Validate", {0xFF, 0x90}, 2, false, false))
    {
        REGISTER_VTABLE_HOOK(DefaultFortPlayerControllerAthenaObject->VTable, impl_target_index / 8, creative_component_hooks::ServerLoadPlotForPortal_Impl)
    }
    
    INIT_POINTER(LoadPlayset, memory_manager::find_function_by_string(L"UPlaysetLevelStreamComponent::LoadPlayset Error: no owner for %s"));

    REGISTER_CALL_DETOUR_WITH_SAVE(memory_manager::find_pattern("E8 ? ? ? ? 0F B6 F8 44 89 75 EF 44 39 75 F3 74 0B 33 D2 48 8D 4D E7 E8 ? ? ? ?"), creative_component_hooks::SaveArrayToFile);
    REGISTER_CALL_DETOUR_WITH_SAVE(memory_manager::find_pattern("E8 ? ? ? ? 84 C0 75 38 80 3D ? ? ? ? ? 72 2A 48 8B 05 ? ? ? ? 4C 8D 45 28 48 89 45 28"), creative_component_hooks::LoadFileToArray);
    
    DetourTransactionCommit();
    
    // Register custom function
    REGISTER_CUSTOM_UFUNCTION(TeleportPlayerToLobbyIslandWithTimer, AFortPlayerControllerAthena::StaticClass())
    REGISTER_CUSTOM_UFUNCTION(TeleportPlayerToIsland, AFortPlayerControllerAthena::StaticClass())
    REGISTER_CUSTOM_UFUNCTION(FinalizeTeleport, AFortPlayerControllerAthena::StaticClass())
}

void creative_component_hooks::SetTeamForPlayer_Hook(AFortMinigame* Self, AFortPlayerController* AFortPlayerController, uint8_t TeamIndex)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"creative_component_hooks::SetTeamForPlayer_Hook called (Presumably on minigame start). Nulling call.");
    }
}

void creative_component_hooks::AddMiniGamePlayer_Hook(AFortMinigame* MiniGame, AFortPlayerState* PlayerState)
{
    if (log_component::enable_debug_logs)
    {
        if (STATIC_HASPROPERTY(MiniGame, bAllowJoinInProgress))
        {
            spdlog::info(L"creative_component:AddMiniGamePlayer_Hook: bAllowJoinInProgress {}", MiniGame->bAllowJoinInProgress());
        }
    }
    
    AddMiniGamePlayer(MiniGame, PlayerState);
    
    if(MiniGame->Volume())
    {
        if(MiniGame->Volume()->LinkedPortals()[0])
        {
            {
                MiniGame->Volume()->LinkedPortals()[0]->CurrentPopulation() = MiniGame->PlayerArray().Players().Num();
            
                spdlog::info(L"[{}] creative_component:AddMiniGamePlayer_Hook: Updating Island population {}", MiniGame->GetFullName().c_str(), MiniGame->Volume()->LinkedPortals()[0]->CurrentPopulation());
            }
        }
    }
}

void creative_component_hooks::RemoveMiniGamePlayer_Hook(AFortMinigame* MiniGame, AFortPlayerState* PlayerState)
{
    RemoveMiniGamePlayer(MiniGame, PlayerState);

    if(MiniGame->Volume())
    {
        if(MiniGame->Volume()->LinkedPortals()[0])
        {
            MiniGame->Volume()->LinkedPortals()[0]->CurrentPopulation() = MiniGame->PlayerArray().Players().Num();
            
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

bool creative_component::is_playground_enabled()
{
    UFortPlaylistAthena* current_playlist = GetPlaylistEasy();
    UFortPlaylistAthena* Playground = FindObject<UFortPlaylistAthena>(L"/Game/Athena/Playlists/Playground/Playlist_Playground.Playlist_Playground");

    if (current_playlist == nullptr)
    {
        return false;
    }

    return current_playlist == Playground;
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

void creative_component::LoadIsland(AFortVolume* Owner)
{
    if (!Owner)
    {
        return;
    }
    
    UFortLevelSaveComponent* LevelSaveComponent = nullptr;
    UPlaysetLevelStreamComponent* LevelStreamComponent = nullptr;
    
    for (auto Component : Owner->BlueprintCreatedComponents())
    {
        if (Component)
        {
            if (Component->IsA(UFortLevelSaveComponent::StaticClass()))
            {
                LevelSaveComponent = reinterpret_cast<UFortLevelSaveComponent*>(Component);
            }
            if (Component->IsA(UPlaysetLevelStreamComponent::StaticClass()))
            {
                LevelStreamComponent = reinterpret_cast<UPlaysetLevelStreamComponent*>(Component);
            }
        }
    }

    if (LevelSaveComponent && LevelStreamComponent)
    {
        // Attempt loading the island record from CDN
        std::string account_id = player_component::get_account_id_from_unique_id(LevelSaveComponent->AccountIdOfOwner());
        if (account_id.size() <= 0)
        {
            return;
        }
                
        // Make save folder for account id if it does not exist yet
        std::string AccountSavePath = UKismetSystemLibrary::GetProjectSavedDirectory().AnsiString().append("SaveGames/").append(account_id);
        std::filesystem::create_directories(AccountSavePath);
                
        // Get playset name
        std::wstring PlaysetName = LevelStreamComponent->CurrentPlayset()->GetName();
        TO_STRING(PlaysetName, sPlaysetName);

        // Perform request
        Aws::S3::Model::GetObjectRequest Request;
        Request.SetBucket("creativeislands");
        Request.SetKey(Owner->EraVolumeInfo().bIsFeaturedCreativeIsland ? "featured.sav" : account_id + "/" + sPlaysetName + ".sav" );
        std::wstring VolumeName = Owner->GetName().c_str();
        TO_STRING(VolumeName, sVolumeName);
        std::string Path = Owner->EraVolumeInfo().bIsFeaturedCreativeIsland ? UKismetSystemLibrary::GetProjectSavedDirectory().AnsiString().append("SaveGames/").append("featured.sav") : AccountSavePath.append("/").append(sVolumeName).append("LevelSave.sav");
        Request.SetResponseStreamFactory(
        [=]() {
               return Aws::New<Aws::FStream>("dl", Path, std::ios_base::out | std::ios_base::binary);
        }
        );
        Aws::S3::Model::GetObjectOutcome outcome = gameserver_client::Client->S3Client->GetObject(Request);
                
        if (log_component::enable_debug_logs)
        {
            spdlog::info(L"creative_component::LoadIsland: finished loading playset, loading record from disk. Download status: [Size: {}, Successful: {}]", outcome.GetResult().GetContentLength(), outcome.IsSuccess());
        }

        // If we couldn't download but we have an island file locally, delete it before loading it.
        if (!outcome.IsSuccess())
        {
            if (std::filesystem::exists(Path))
            {
                std::filesystem::remove(Path);
            }

            // There will be a new record: prepare the island

            // Log
            spdlog::info(L"creative_component::LoadIsland: Using clean island (No record on R2). Processing ResetVolumeAndLevelRecord");
            
            // Generate island
            (*GWorld)->GameState()->VolumeManager()->ResetVolumeAndLevelRecord(Owner, nullptr);
        }
        else
        {
            // Load level record
            LevelSaveComponent->LoadLevelRecord();
        }
    }
}

const TCHAR* creative_component::GetEraSaveFileName(const TCHAR* OldName)
{
    std::wstring LocalFilePath(OldName);
    if (LocalFilePath.find_last_of(L"/\\") != std::wstring::npos &&
        LocalFilePath.find(L"LevelSave.sav") != std::wstring::npos)
    {
        std::wstring LocalFileName = LocalFilePath.substr(LocalFilePath.find_last_of(L"/\\") + 1);
        LocalFileName = LocalFileName.substr(0, LocalFileName.find(L"LevelSave.sav"));
        TO_STRING(LocalFileName, sLocalFileName);
        
        // Get volume actor by name
        TArray<AFortVolume*> VolumeActors = UGameplayStatics::GetAllActorsOfClass<AFortVolume>(*GWorld, AFortVolume::StaticClass());
        for (AFortVolume* Volume : VolumeActors)
        {
            if (!memory_manager::is_bad_read_pointer(Volume) && Volume && Volume->GetName() == LocalFileName)
            {
                for (UActorComponent* Component : Volume->BlueprintCreatedComponents())
                {
                    if (Component)
                    {
                        if (Component->IsA(UFortLevelSaveComponent::StaticClass()))
                        {
                            UFortLevelSaveComponent* FortLevelSaveComponent = static_cast<UFortLevelSaveComponent*>(Component);

                            std::string account_id = player_component::get_account_id_from_unique_id(FortLevelSaveComponent->AccountIdOfOwner());
                            if (account_id.size() <= 0)
                            {
                                return nullptr;
                            }

                            std::string AccountSavePath = UKismetSystemLibrary::GetProjectSavedDirectory().AnsiString().append("SaveGames/").append(account_id);
                            std::filesystem::create_directories(AccountSavePath);
                            std::string Path = Volume->EraVolumeInfo().bIsFeaturedCreativeIsland ? UKismetSystemLibrary::GetProjectSavedDirectory().AnsiString().append("SaveGames/").append("featured.sav") : AccountSavePath.append("/").append(sLocalFileName).append("LevelSave.sav");
                            TO_WSTRING(Path, wPath);

                            if (log_component::enable_debug_logs)
                            {
                                spdlog::info(L"creative_component::GetEraSaveFileName converted OldName ['{}'] to Era SaveGame Path (includes account_id) ['{}'].", OldName, wPath.c_str());
                            }
                            
                            return wPath.c_str();
                        }
                    }
                }
                
                break;
            }
        }
    }

    return nullptr;
}

AFortPlayerControllerAthena* creative_component::GetVolumeOwner(AFortVolume* Volume)
{
    if (Volume)
    {
        for (UActorComponent* Component : Volume->BlueprintCreatedComponents())
        {
            if (Component)
            {
                if (Component->IsA(UFortLevelSaveComponent::StaticClass()))
                {
                    UFortLevelSaveComponent* LevelSaveComponent = reinterpret_cast<UFortLevelSaveComponent*>(Component);

                    if (UFortKismetLibrary::IsValid_UniqueNetIdRepl(LevelSaveComponent->AccountIdOfOwner()))
                    {
                        for (int i = 0; i < (*GWorld)->NetDriver()->ClientConnections().Num(); i++)
                        {
                            if (UNetConnection* ClientConnection = (*GWorld)->NetDriver()->ClientConnections()[i])
                            {
                                if (UFortKismetLibrary::EqualEqual_UniqueNetIdReplUniqueNetIdRepl(ClientConnection->PlayerID(), LevelSaveComponent->AccountIdOfOwner()))
                                {
                                    return reinterpret_cast<AFortPlayerControllerAthena*>(ClientConnection->PlayerController());
                                }
                            }
                        }
                    }

                    break;
                }
            }
        }
    }

    return nullptr;
}

void creative_component::OnRespawn(AController* PlayerController, AActor*& PickedStart)
{
    // Creative?
    if (is_creative_enabled() && PickedStart)
    {
        // Safety checks 
        if (!memory_manager::is_bad_read_pointer(PlayerController) && PlayerController)
        {
            // Class
            if (AFortPlayerControllerAthena* AthenaPlayerController = PlayerController->Cast<AFortPlayerControllerAthena>())
            {
                TArray<AFortPlayerStartCreative*> PlayerStarts = UGameplayStatics::GetAllActorsOfClass<AFortPlayerStartCreative>();
                
                // If we're on an island DO NOT ever use spawn island player starts
                if (AthenaPlayerController->CreativePlotLinkedVolume())
                {
                    // Spawn island?
                    if (AFortPlayerStartCreative* PickedStartCreative = PickedStart->Cast<AFortPlayerStartCreative>())
                    {
                        if (PickedStartCreative->PlayerStartTags().GameplayTags.Find(FGameplayTag(TEXT("Playground.LobbyIsland.Spawn"))) ||
                            PickedStartCreative->PlayerStartTags().GameplayTags.Find(FGameplayTag(TEXT("Playground.IslandDock.Spawn"))))
                        {
                            // Pick a new start
                            for (AFortPlayerStartCreative* PlayerStart : PlayerStarts)
                            {
                                if (PlayerStart->CreativeLinkComponent())
                                {
                                    if (PlayerStart->CreativeLinkComponent()->LinkedVolume() == AthenaPlayerController->CreativePlotLinkedVolume())
                                    {
                                        if (PlayerStart->PlayerStartTags().GameplayTags.Find(FGameplayTag(TEXT("Creative.Island.Spawn"))))
                                        {
                                            // log
                                            if (log_component::enable_debug_logs)
                                            {
                                                spdlog::info(L"creative_component::OnRespawn: Spawn redirected. Picked start {} (before: {}) for player {}.",
                                                    PlayerStart->GetName().c_str(),
                                                    PickedStart->GetName().c_str(),
                                                    AthenaPlayerController->GetName().c_str());
                                            }
                                        
                                            // Found new start
                                            PickedStart = PlayerStart;
                                        
                                            // Return
                                            return;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void creative_component::TeleportPlayerToLobbyIslandWithTimer(AFortPlayerControllerAthena* FortPlayerControllerAthena,
                                                              FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    AActor* PlayerStart = (*GWorld)->AuthorityGameMode()->ChoosePlayerStart(FortPlayerControllerAthena);

    if (PlayerStart)
    {
        FortPlayerControllerAthena->Pawn()->K2_TeleportTo(PlayerStart->K2_GetActorLocation(), PlayerStart->K2_GetActorRotation());

        // Revert stasis
        FortPlayerControllerAthena->Pawn()->SetStasisMode(0);
        
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
}

void creative_component::TeleportPlayerToIsland(AFortPlayerControllerAthena* PlayerController, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    AFortPlayerPawn* PlayerPawn = PlayerController->Pawn();
    AFortAthenaCreativePortal* Portal = PlayerController->CreativePlotLinkedVolume()->LinkedPortals()[PlayerController->CreativePlotLinkedVolume()->LinkedPortals().Num() - 1];
    
    PlayerPawn->K2_TeleportTo(Portal->LinkedVolume()->K2_GetActorLocation(), FRotator());
    
    // Reset stasis
    PlayerPawn->SetStasisMode(0);
            
    static_cast<AFortPlayerPawnAthena*>(PlayerPawn)->TeleportToSkyDive(18000);

    Portal->NotifyTeleportedPlayerPawn(PlayerPawn);
    Portal->OnPlayerPawnTeleported(PlayerPawn);
    
    // Load the record, which will then apply the save to world
    if (Portal->LinkedVolume()->EraVolumeInfo().bPendingReload)
    {
        Portal->LinkedVolume()->EraVolumeInfo().bPendingReload = false;
        
        LoadIsland(Portal->LinkedVolume());
    }
}

void creative_component::FinalizeTeleport(AFortPlayerControllerAthena* PlayerController, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    (*GWorld)->AuthorityGameMode()->RestartPlayer(PlayerController);
}

void creative_component::assign_portal_to_player(UNetConnection* NetConnection, int PortalIndex)
{
    AFortCreativePortalManager* creative_portal_manager = (*GWorld)->GameState()->CreativePortalManager();

    if (creative_portal_manager != nullptr)
    {
        AFortAthenaCreativePortal* creative_portal = creative_portal_manager->AvailablePortals()[PortalIndex];

        AFortPlayerControllerAthena* player_controller_athena = reinterpret_cast<AFortPlayerControllerAthena*>(NetConnection->PlayerController());

        // Give islands
        {
            TArrayManager CreativeIslands = player_controller_athena->CreativeIslands();

            {
                FCreativeIslandData* Island = new FCreativeIslandData();
                UKismetTextLibrary::Conv_StringToText(std::wstring(L"My Era Island"), &Island->IslandName());
                Island->McpId() = std::wstring(L"island");
                CreativeIslands.Add(Island);
            }

            // Common featured island
            {
                FCreativeIslandData* Island = new FCreativeIslandData();
                UKismetTextLibrary::Conv_StringToText(std::wstring(L"Nuketown Sniper Map (FEATURED)"), &Island->IslandName());
                Island->McpId() = std::wstring(L"featured");
                CreativeIslands.Add(Island);
            }
        }
        
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

        creative_portal->ClientSaveStateChanged(false);
        creative_portal->ClientSaveDisplayChanged(false);
        
        spdlog::info(L"Assigned and initialized a creative portal for '{}'", player_controller_athena->MyPlayerInfo()->PlayerName().c_str());

        // Get the Playset needed for a certain real estate
        if (UFortCreativeRealEstatePlotItemDefinition* Plot = FindObject<UFortCreativeRealEstatePlotItemDefinition>(L"/Game/Playgrounds/Items/Plots/Temperate_Medium.Temperate_Medium", true))
        {
            UFortItem* PlotInstance = Plot->CreateTemporaryItemInstanceBP(1, 1);
            
            UFortPlaysetItemDefinition* Playset = Plot->BasePlayset().Get();

            // Check whether or not we should reload the island
            bool bRefreshingOwnership = false;
            for (UActorComponent* Component : creative_portal->LinkedVolume()->BlueprintCreatedComponents())
            {
                // Prevent a crash on later builds and when more people are joining
                if (Component)
                {
                    if (Component->IsA(UFortLevelSaveComponent::StaticClass()))
                    {
                        UFortLevelSaveComponent* LevelSaveComponent = static_cast<UFortLevelSaveComponent*>(Component);

                        // If we already had an owner
                        if (UFortKismetLibrary::IsValid_UniqueNetIdRepl(LevelSaveComponent->AccountIdOfOwner()))
                        {
                            bRefreshingOwnership = true;
                            break;
                        }
                    }
                }
            }
            
            // Now get this volume's playset component and initialize the island
            for (UActorComponent* Component : creative_portal->LinkedVolume()->BlueprintCreatedComponents())
            {
                // Prevent a crash on later builds and when more people are joining
                if (Component)
                {
                    if (Component->IsA(UPlaysetLevelStreamComponent::StaticClass()) && !bRefreshingOwnership)
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
                        
                        // Log
                        spdlog::info(L"Created and loaded Island ('{}') for portal '{}'", Plot->GetName().c_str(), creative_portal->GetName().c_str());
                    }
                    if (Component->IsA(UFortLevelSaveComponent::StaticClass()))
                    {
                        UFortLevelSaveComponent* LevelSaveComponent = static_cast<UFortLevelSaveComponent*>(Component);
                        
                        LevelSaveComponent->AccountIdOfOwner() = creative_portal->OwningPlayer();
                        
                        // If we already had an owner
                        if (bRefreshingOwnership)
                        {
                            if (log_component::enable_debug_logs)
                            {
                                spdlog::info(L"creative_component::assign_portal_to_player is transferring island ownership.");
                            }

                            // Clear the volume
                            creative_portal->LinkedVolume()->ServerClearVolume();

                            // Make a new record
                            LevelSaveComponent->ConstructNewLevelRecord(creative_portal->LinkedVolume());

                            // Load the record, which will then apply the save to world
                            LoadIsland(creative_portal->LinkedVolume());
                        }
                    }
                }
            }
        }
    }
}

void creative_component_hooks::ResetMyCurrentCreativePlot_Hook(AFortPlayerControllerAthena* PlayerController, FFrame& StackFrame)
{
    if (AFortVolume* CreativePlot = PlayerController->CreativePlotLinkedVolume())
    {
        if (log_component::enable_debug_logs)
        {
            spdlog::info(L"creative_component_hooks::ResetMyCurrentCreativePlot_Hook: resetting player's plot.");
        }

        (*GWorld)->GameState()->VolumeManager()->ResetVolumeAndLevelRecord(CreativePlot, PlayerController);
    }
}

void creative_component_hooks::TeleportPlayerToLinkedVolume_Hook(AFortAthenaCreativePortal* Portal, AFortPlayerPawn* PlayerPawn, bool bUseSpawnTags)
{
    if (Portal && !memory_manager::is_bad_read_pointer(PlayerPawn) && PlayerPawn && !memory_manager::is_bad_read_pointer(PlayerPawn->Controller()) && PlayerPawn->Controller())
    {
        if (STATIC_HASPROPERTY(Portal, LinkedVolume))
        {
            if (Portal == nullptr || memory_manager::is_bad_read_pointer(Portal) || Portal->LinkedVolume() == nullptr || memory_manager::is_bad_read_pointer(Portal->LinkedVolume()) || PlayerPawn == nullptr)
            {
                spdlog::error(L"creative_component_hooks::TeleportPlayerToLinkedVolume_Hook: Invalid LinkedVolume or PlayerPawn");
        
                return;
            }

            if (Portal->LinkedVolume())
            {
                if ((*GWorld)->GameState()->VolumeManager()->GetMinigameForVolume(Portal->LinkedVolume()))
                {
                    if (log_component::enable_debug_logs)
                    {
                        spdlog::info("GetMinigameForVolume CurrentState: {}", (*GWorld)->GameState()->VolumeManager()->GetMinigameForVolume(Portal->LinkedVolume())->CurrentState());
                    }
                    
                    // Don't let in if minigame is in progress
                    if ((*GWorld)->GameState()->VolumeManager()->GetMinigameForVolume(Portal->LinkedVolume())->CurrentState() == 1 ||
                        (*GWorld)->GameState()->VolumeManager()->GetMinigameForVolume(Portal->LinkedVolume())->CurrentState() == 2)
                    {
                        return;
                    }
                }
            }
            else
            {
                return;
            }

            // Stasis mode
            PlayerPawn->SetStasisMode(3);
            PlayerPawn->HandleInStasis();
            
            // Save portal in playercontroller
            static_cast<AFortPlayerControllerAthena*>(PlayerPawn->Controller())->CreativePlotLinkedVolume() = Portal->LinkedVolume();
            static_cast<AFortPlayerControllerAthena*>(PlayerPawn->Controller())->OnRep_CreativePlotLinkedVolume();
            
            // Teleport timer
            UKismetSystemLibrary::K2_SetTimer(PlayerPawn->Controller(), TEXT("TeleportPlayerToIsland"), 3.0, false);
        }
    }
}

void creative_component_hooks::ServerGiveCreativeItem_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(FFortItemEntry, CreativeItem);

    if (!inventory_component::is_inventory_full(FortPlayerController, 5))
    {
        if (CreativeItem.Count() && CreativeItem.ItemDefinition())
        {
            inventory_component::add_item(FortPlayerController, CreativeItem.ItemDefinition(), CreativeItem.Count(), looting_component::get_default_loaded_ammo(CreativeItem.ItemDefinition()));
        }
    }
    else
    {
        if (CreativeItem.Count() && CreativeItem.ItemDefinition())
        {
            if (FortPlayerController->Pawn())
            {
                pickup_component::spawn_pickup(CreativeItem.ItemDefinition(), FortPlayerController->Pawn()->K2_GetActorLocation(), CreativeItem.Count(), looting_component::get_default_loaded_ammo(CreativeItem.ItemDefinition()), true, nullptr, {}, FortPlayerController->Pawn());
            }
        }
    }
    
    return ServerGiveCreativeItem(FortPlayerController, StackFrame);
}

void creative_component_hooks::ServerTeleportToPlaygroundLobbyIsland_Hook(AFortPlayerControllerAthena* FortPlayerControllerAthena, FFrame& StackFrame)
{
    if (!creative_component::is_creative_enabled())
    {
        spdlog::error(L"creative_component_hooks::ServerTeleportToPlaygroundLobbyIsland_Hook: Called outside creative. Aborted.");
    }
    
    if (FortPlayerControllerAthena == nullptr || FortPlayerControllerAthena->Pawn() == nullptr)
    {
        spdlog::error(L"creative_component_hooks::ServerTeleportToPlaygroundLobbyIsland_Hook: Invalid PlayerController or Pawn.");
        
        return ServerTeleportToPlaygroundLobbyIsland(FortPlayerControllerAthena, StackFrame);
    }

    // Pre teleport
    FortPlayerControllerAthena->PreLobbyIslandTeleport();

    // Stasis
    FortPlayerControllerAthena->Pawn()->SetStasisMode(3);
    FortPlayerControllerAthena->Pawn()->HandleInStasis();
    
    // Timer
    UKismetSystemLibrary::K2_SetTimer(FortPlayerControllerAthena, TEXT("TeleportPlayerToLobbyIslandWithTimer"), 3.0, false);

    return ServerTeleportToPlaygroundLobbyIsland(FortPlayerControllerAthena, StackFrame);
}

void creative_component_hooks::OnFinishedRecordRead_Hook(UObject* Object, UObject*, void*)
{
    if (Object->IsA(UFortLevelSaveComponent::StaticClass()))
    {
        UFortLevelSaveComponent* LevelSaveComponent = static_cast<UFortLevelSaveComponent*>(Object);

        if (log_component::enable_debug_logs)
        {
            spdlog::info(L"creative_component_hooks::OnFinishedRecordRead_Hook: finished loading record [{}] from disk.", UKismetSystemLibrary::GetPathName(LevelSaveComponent->LevelRecord()).c_str());
        }
        
        // Apply to world
        LevelSaveComponent->ApplySaveToWorld(true);
    }
}

void creative_component_hooks::OnFinishedRecordWrite_Hook(UObject* Object, UObject*, void*)
{
    CreateThread(nullptr, 0, [](LPVOID InParam)->DWORD
    {
        UObject* Object = static_cast<UObject*>(InParam);
        
        if (Object->IsA(UFortLevelSaveComponent::StaticClass()))
        {
            UFortLevelSaveComponent* LevelSaveComponent = static_cast<UFortLevelSaveComponent*>(Object);

            if (log_component::enable_debug_logs)
            {
                spdlog::info(L"creative_component_hooks::OnFinishedRecordWrite_Hook: finished writing record [{}] to disk.", UKismetSystemLibrary::GetPathName(LevelSaveComponent->LevelRecord()).c_str());
            }

            // Get owner
            AFortVolume* Owner = static_cast<AFortVolume*>(static_cast<UPlaysetLevelStreamComponent*>(Object)->GetOwner());
            UPlaysetLevelStreamComponent* PlaysetComponent = nullptr;

            if (!Owner)
            {
                return 0;
            }
        
            for (auto Component : Owner->BlueprintCreatedComponents())
            {
                if (Component)
                {
                    if (Component->IsA(UPlaysetLevelStreamComponent::StaticClass()))
                    {
                        PlaysetComponent = static_cast<UPlaysetLevelStreamComponent*>(Component);
                    }
                }
            }

            // Get file buffer & upload on CDN bucket 'creativeislands'
            if (PlaysetComponent)
            {
                // Get owner's account ID
                std::string account_id = player_component::get_account_id_from_unique_id(LevelSaveComponent->AccountIdOfOwner());
                if (account_id.size() <= 0)
                {
                    return 0;
                }

                // Make save folder for account id if it does not exist yet
                std::string AccountSavePath = UKismetSystemLibrary::GetProjectSavedDirectory().AnsiString().append("SaveGames/").append(account_id);
                std::filesystem::create_directories(AccountSavePath);
            
                // The file name will be the playset's name
                std::wstring wPlaysetName = PlaysetComponent->CurrentPlayset()->GetName();
                TO_STRING(wPlaysetName, sPlaysetName);

                // Get the actual file name locally, in appdata
                std::wstring VolumeName = Owner->GetName();
                TO_STRING(VolumeName, sVolumeName);
                std::string Path = AccountSavePath.append("/").append(sVolumeName).append("LevelSave.sav");

                if (std::filesystem::exists(Path))
                {
                    Aws::S3::Model::PutObjectRequest Request;
                    Request.SetBucket("creativeislands");
                    Request.SetKey(account_id + "/" + sPlaysetName + ".sav");
                
                    if (log_component::enable_debug_logs)
                    {
                        spdlog::info("creative_component_hooks::OnFinishedRecordWrite_Hook: uploading '{}' to ERACDN:creativeislands. Key: '{}'", Path.c_str(), account_id + "/" + sPlaysetName + ".sav");
                    }

                    std::shared_ptr<Aws::IOStream> inputData = Aws::MakeShared<Aws::FStream>("SampleAllocationTag",Path, std::ios_base::in | std::ios_base::binary);

                    Request.SetBody(inputData);

                    Aws::S3::Model::PutObjectOutcome outcome = gameserver_client::Client->S3Client->PutObject(Request);

                    if (log_component::enable_debug_logs)
                    {
                        spdlog::info("creative_component_hooks::OnFinishedRecordWrite_Hook: upload of key '{}' finished. Result: {}.", account_id + "/" + sPlaysetName + ".sav", outcome.IsSuccess() ? "succeeded" : "failed");
                    }
                }
                else
                {
                    spdlog::error("creative_component_hooks::OnFinishedRecordWrite_Hook: attempted to read from '{}', invalid file.", Path.c_str());
                }
            }
        }

        return 0;
    }, Object, 0, nullptr);
}

void creative_component_hooks::OnFinishedLoading_Hook(UObject* Object, UObject*, void*)
{
    if (Object->IsA(UPlaysetLevelStreamComponent::StaticClass()))
    {
        AFortVolume* Owner = static_cast<AFortVolume*>(static_cast<UPlaysetLevelStreamComponent*>(Object)->GetOwner());

        creative_component::LoadIsland(Owner);
    }
}

void creative_component_hooks::SaveOutLevelRecord_Hook(UFortLevelSaveComponent* Self)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"creative_component_hooks::SaveOutLevelRecord_Hook: saving island.");
    }

    if (Self->GetOwner())
    {
        if (Self->GetOwner()->Cast<AFortVolume>())
        {
            if (Self->GetOwner()->Cast<AFortVolume>()->EraVolumeInfo().bIsFeaturedCreativeIsland ||
                Self->GetOwner()->Cast<AFortVolume>()->EraVolumeInfo().bPendingReload)
            {
                if (log_component::enable_debug_logs)
                {
                    spdlog::info(L"creative_component_hooks::SaveOutLevelRecord_Hook: ABORTING! The target island is featured and cannot be edited.");
                }
                
                return;
            }
        }
    }
    
    // Call base function to perform saving
    reinterpret_cast<void (*)(UFortLevelSaveComponent*)>(CreateDefaultObject<ULevelSaveComponent>()->VTable[SaveOutLevelRecord_Index])(Self);
}

bool creative_component_hooks::SaveArrayToFile_Hook(TArray<uint8>& Self, const TCHAR* FileName, void* FileManager, uint32 WriteFlags)
{
    if (const TCHAR* NewFileName = creative_component::GetEraSaveFileName(FileName))
    {
        return SaveArrayToFile(Self, NewFileName ? NewFileName : FileName, FileManager, WriteFlags);
    }

    return false;
}

bool creative_component_hooks::LoadFileToArray_Hook(TArray<uint8>& Self, const TCHAR* FileName, uint32 WriteFlags)
{
    if (const TCHAR* NewFileName = creative_component::GetEraSaveFileName(FileName))
    {
        return LoadFileToArray(Self, NewFileName ? NewFileName : FileName, WriteFlags);
    }

    return false;
}

void creative_component_hooks::HandleMinigameStarted_Hook(UObject* Object)
{
    // Call native function
    HandleMinigameStarted(Object);

    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"creative_component_hooks::HandleMinigameStarted_Hook: processing players.");   
    }
    
    AFortMinigame* Minigame = static_cast<AFortMinigame*>(Object);
    
    TArrayManager Players = TArrayManager<FMinigamePlayer>(&Minigame->PlayerArray().Players(), FMinigamePlayer::GetStructSize());
    
    for (FMinigamePlayer& Player : Players)
    {
        if (log_component::enable_debug_logs)
        {
            spdlog::info(L"creative_component_hooks::HandleMinigameStarted_Hook: processing {}", Player.PlayerState()->GetName().c_str());
        }
        
        if  (!Player.PlayerState() || !Player.PlayerState()->Owner())
        {
            continue;
        }
        
        AFortPlayerController* PlayerController = reinterpret_cast<AFortPlayerController*>(Player.PlayerState()->Owner());
        
        // clear inventory
        inventory_component::clear_primary_inventory(PlayerController, false);
        
        // Remove phone on minigame start ( in case game fails to )
        if (UFortItem* Item = inventory_component::get_inventory_item_by_definition(PlayerController, FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/Prototype/WID_CreativeTool.WID_CreativeTool")))
        {
            inventory_component::remove_item(PlayerController, Item->GetItemGuid(), -1, false);
        }

        // Get a respawn location
        FVector Loc;
        FRotator Rot;
        bool bOutRespawningOnGround;
        if (static_cast<AFortMinigame*>(Object)->DetermineSpawnLocation(static_cast<AFortPlayerStateAthena*>(Player.PlayerState()), &Loc, &Rot, &bOutRespawningOnGround))
        {
            // Destroy current pawn, and restart
            if (AFortPawn* PlayerPawn = reinterpret_cast<AFortPlayerController*>(Player.PlayerState()->Owner())->Pawn())
            {
                PlayerPawn->K2_DestroyActor();
                
                (*GWorld)->AuthorityGameMode()->RestartPlayerAtTransform(PlayerController, UKismetMathLibrary::MakeTransform(Loc, Rot, FVector(1, 1, 1)));

                if (!bOutRespawningOnGround)
                {
                    PlayerController->Pawn()->BeginSkydiving(false);
                }

                if (log_component::enable_debug_logs)
                {
                    spdlog::info(L"creative_component_hooks::HandleMinigameStarted_Hook: restarted {} at picked Spawn Location. Skydiving: {}", Player.PlayerState()->GetName().c_str(), !bOutRespawningOnGround);
                }
            }
        }
        else
        {
            spdlog::error(L"creative_component_hooks::HandleMinigameStarted_Hook: DetermineSpawnLocation failed.");
        }
    }
}

void creative_component_hooks::HandleMinigameEnded_Hook(UObject* Object)
{
    // Call native function
    HandleMinigameEnded(Object);
    
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"creative_component_hooks::HandleMinigameEnded_Hook: processing players.");   
    }
    
    AFortMinigame* Minigame = static_cast<AFortMinigame*>(Object);
    FUniqueNetIdRepl AccountIdOfOwner{};
    bool bFoundOwner = false;
    
    for (UActorComponent* Component : Minigame->Volume()->BlueprintCreatedComponents())
    {
        if (Component)
        {
            if (Component->IsA(UFortLevelSaveComponent::StaticClass()))
            {
                memcpy(&AccountIdOfOwner, &static_cast<UFortLevelSaveComponent*>(Component)->AccountIdOfOwner(), sizeof FUniqueNetIdRepl);
                bFoundOwner = true;
                break;
            }
        }
    }

    if (bFoundOwner)
    {
        TArrayManager Players = TArrayManager<FMinigamePlayer>(&Minigame->PlayerArray().Players(), FMinigamePlayer::GetStructSize());
    
        for (FMinigamePlayer& Player : Players)
        {
            if (!Player.PlayerState() || !Player.PlayerState()->Owner())
            {
                continue;
            }
            
            AFortPlayerController* PlayerController = reinterpret_cast<AFortPlayerController*>(Player.PlayerState()->Owner());

            // clear inventory
            inventory_component::clear_primary_inventory(PlayerController, false);

            // If it's the island owner, give them the phone back
            if (UFortKismetLibrary::EqualEqual_UniqueNetIdReplUniqueNetIdRepl(PlayerController->NetConnection()->PlayerID(), AccountIdOfOwner))
            {
                inventory_component::add_item(PlayerController, FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/Prototype/WID_CreativeTool.WID_CreativeTool"), 1);
            }
        }
    }
}

void creative_component_hooks::Server_SetCanEditCreativeIsland_Hook(AFortPlayerStateAthena* Self, FFrame& StackFrame)
{
    if (!creative_component::is_creative_enabled())
    {
        spdlog::error(L"creative_component_hooks::Server_SetCanEditCreativeIsland_Hook: Called outside creative. Aborted.");
    }
    
    GET_PARAMETER(bool, bCanEdit);   
    GET_PARAMETER(TArray<FString>, WhiteList);

    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"player_component_hooks::Server_SetCanEditCreativeIsland_Hook received permissions update request ({}, {})...", bCanEdit, WhiteList.Num());
    }

    if (creative_component::is_creative_enabled())
    {
        if (AFortVolume* Volume = (*GWorld)->GameState()->VolumeManager()->GetVolumeForPlayerState(Self))
        {
            for (const auto Component : Volume->BlueprintCreatedComponents())
            {
                if (Component)
                {
                    if (Component->IsA(UFortLevelSaveComponent::StaticClass()))
                    {
                        UFortLevelSaveComponent* LevelSaveComponent = reinterpret_cast<UFortLevelSaveComponent*>(Component);

                        // Change permissions
                        if (STATIC_HASPROPERTY(LevelSaveComponent, PlotPermissions))
                        {
                            LevelSaveComponent->PlotPermissions().Permission() = bCanEdit;
                        
                            // Recreate WhiteList array
                            TArrayManager WhiteListArray = TArrayManager(&LevelSaveComponent->PlotPermissions().WhiteList(), sizeof FString);
                            WhiteListArray.Clear(FindPropertyByName(LevelSaveComponent->PlotPermissions().GetClass(), TEXT("WhiteList")));
                            for (FString& WhitelistElement : WhiteList)
                            {
                                LevelSaveComponent->PlotPermissions().WhiteList().Add(std::wstring(WhitelistElement.c_str()));
                            }

                            LevelSaveComponent->OnRep_PlotPermissionsChanged();

                            if (log_component::enable_debug_logs)
                            {
                                spdlog::info(L"Changed plot permissions for {}. New permission: '{}', WhiteList.Num: {}", Self->GetName().c_str(), LevelSaveComponent->PlotPermissions().Permission() ? L"Public" : L"Private", LevelSaveComponent->PlotPermissions().WhiteList().Num());
                            }
                        }
                        else
                        {
                            if (auto Minigame = Self->Owner()->Cast<AFortPlayerControllerAthena>()->GetMinigame())
                            {
                                if (log_component::enable_debug_logs)
                                {
                                    spdlog::info(L"player_component_hooks::Server_SetCanEditCreativeIsland_Hook changing everyone's bCanEdit permissions.");
                                }

                                // Safe
                                if (*GWorld && (*GWorld)->NetDriver())
                                {
                                    for (UNetConnection* Connection : (*GWorld)->NetDriver()->ClientConnections())
                                    {
                                        if (Connection)
                                        {
                                            if (!memory_manager::is_bad_read_pointer(Connection->PlayerController()) && Connection->PlayerController())
                                            {
                                                if (!memory_manager::is_bad_read_pointer(Connection->PlayerController()->PlayerState()) && Connection->PlayerController()->PlayerState())
                                                {
                                                    if (Minigame->IsPlayerInOurVolume(Connection->PlayerController()->PlayerState()))
                                                    {
                                                        // Change permission
                                                        Connection->PlayerController()->PlayerState()->bCanEditCreativeIsland() = bCanEdit;
                                                            
                                                        if (log_component::enable_debug_logs)
                                                        {
                                                            spdlog::info(L"player_component_hooks::Server_SetCanEditCreativeIsland_Hook updated permission for {} as player is in the Minigame's volume.",
                                                                Connection->PlayerController()->PlayerState()->GetFullName().c_str());
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                // Now set individual permissions
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (log_component::enable_debug_logs)
        {
            spdlog::error(L"player_component_hooks::Server_SetCanEditCreativeIsland_Hook received permissions update request, but server isn't in Creative mode.");
        }
    }
}

void creative_component_hooks::NativeEnteringPlayspaceEvent_Hook(AFortMinigame* Self, class APlayerState* PlayerState, class AFortVolume* EnteredVolume)
{
    NativeEnteringPlayspaceEvent(Self, PlayerState, EnteredVolume);
}

void creative_component_hooks::NativeExitedPlayspaceEvent_Hook(AFortMinigame* Self, class APlayerState* PlayerState, class AFortVolume* ExitedVolume)
{
    NativeExitedPlayspaceEvent(Self, PlayerState, ExitedVolume);
    
    // After fortnite's own code, make sure there's nothing in the inventory anymore.
    // This is not only used as one is permanently leaving the island, but also when the island is left by temporarily crossing its boundaries

    if (!memory_manager::is_bad_read_pointer(PlayerState) && PlayerState)
    {
        if (!memory_manager::is_bad_read_pointer(PlayerState->Owner()) && PlayerState->Owner() && PlayerState->Owner()->Cast<AFortPlayerControllerAthena>())
        {
            AFortPlayerControllerAthena* PlayerController = PlayerState->Owner()->Cast<AFortPlayerControllerAthena>();
            
            // clear inventory ( in case game fails to )
            inventory_component::clear_primary_inventory(PlayerController, false);
        
            // Remove phone ( in case game fails to )
            if (UFortItem* Item = inventory_component::get_inventory_item_by_definition(PlayerController, FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/Prototype/WID_CreativeTool.WID_CreativeTool")))
            {
                inventory_component::remove_item(PlayerController, Item->GetItemGuid(), -1, false);
            }

            // reset perms, just in case
            PlayerState->Cast<AFortPlayerStateAthena>()->bCanEditCreativeIsland() = false;

            if (log_component::enable_debug_logs)
            {
                spdlog::info(L"creative_component_hooks::NativeExitedPlayspaceEvent_Hook: Cleared inventory for: {} (Leaving Playspace)", PlayerController->GetName().c_str());
                spdlog::info(L"creative_component_hooks::NativeExitedPlayspaceEvent_Hook: Reset island permissions for: {} (Leaving Playspace)", PlayerController->GetName().c_str());
            }
        }
    }
}

bool creative_component_hooks::ServerLoadPlotForPortal_Validate_Hook(AFortPlayerControllerAthena* Self, FFrame& StackFrame)
{
    // Always validate as true
    return true;
}

void creative_component_hooks::ServerLoadPlotForPortal_Impl_Hook(AFortPlayerControllerAthena* Self, AFortAthenaCreativePortal* Portal, FString& PlotItemId)
{
    if (creative_component::is_creative_enabled() &&
        Portal && Portal->LinkedVolume() &&
        Portal->CurrentPopulation() == 0)
    {
        // Process island reload when anyone joins.
        Portal->LinkedVolume()->EraVolumeInfo().bPendingReload = true;
    
        // Determine island status
        Portal->LinkedVolume()->EraVolumeInfo().bIsFeaturedCreativeIsland = PlotItemId == std::wstring(L"featured");

        // Close portal
        Portal->bPortalOpen() = false;
        Portal->PortalOpenChanged();
        
        // Reset island
        if (log_component::enable_debug_logs)
        {
            spdlog::info(L"creative_component_hooks::ServerLoadPlotForPortal_Impl_Hook is changing island destination.");
        }
        (*GWorld)->GameState()->VolumeManager()->ResetVolumeAndLevelRecord(Portal->LinkedVolume(), Self);
    }
}
    
void creative_component_hooks::HandleVolumeStateChanged_Hook(AFortVolumeManager* Self, FFrame& StackFrame)
{
    GET_PARAMETER(uint8_t, NewState);
    GET_PARAMETER(AFortVolume*, Volume);
    
    HandleVolumeStateChanged(Self, StackFrame);
    
    if (NewState == 3)
    {
        if (Volume->EraVolumeInfo().bPendingReload)
        {
            Volume->ServerClearVolume();
            
            for (UActorComponent* Component : Volume->BlueprintCreatedComponents())
            {
                if (Component)
                {
                    if (Component->IsA(UFortLevelSaveComponent::StaticClass()))
                    {
                        UFortLevelSaveComponent* LevelSaveComponent = static_cast<UFortLevelSaveComponent*>(Component);
                
                        LevelSaveComponent->ConstructNewLevelRecord(Volume);
                    }
                }
            }

            // Reopen
            Volume->LinkedPortals()[0]->bPortalOpen() = true;
            Volume->LinkedPortals()[0]->PortalOpenChanged();
        }
    }
}