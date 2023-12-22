#include "include.hpp"
#include "creative_component.hpp"

#include "inventory_component.hpp"
#include "log_component.hpp"
#include "looting_component.hpp"
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
    
    REGISTER_POST_PROCESS_EVENT_DETOUR(L"/Script/FortniteGame.LevelSaveBaseComponent.OnFinishedRecordRead", creative_component_hooks::OnFinishedRecordRead_Hook);
    REGISTER_POST_PROCESS_EVENT_DETOUR(L"/Script/FortniteGame.LevelSaveBaseComponent.OnFinishedRecordWrite", creative_component_hooks::OnFinishedRecordWrite_Hook);
    REGISTER_POST_PROCESS_EVENT_DETOUR(L"/Script/FortniteGame.PlaysetLevelStreamComponent.OnFinishedLoading", creative_component_hooks::OnFinishedLoading_Hook);
    REGISTER_PROCESS_EVENT_DETOUR(L"/Script/FortniteGame.FortMinigame.HandleMinigameStarted", creative_component_hooks::HandleMinigameStarted_Hook);
    REGISTER_POST_PROCESS_EVENT_DETOUR(L"/Script/FortniteGame.FortMinigame.HandleMinigameEnded", creative_component_hooks::HandleMinigameEnded_Hook);
    REGISTER_CALL_DETOUR(L"/Script/FortniteGame.FortAthenaCreativePortal.TeleportPlayerToLinkedVolume", creative_component_hooks::TeleportPlayerToLinkedVolume_Hook);
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    REGISTER_UFUNCTION_DETOUR(creative_component_hooks::ServerGiveCreativeItem, L"/Script/FortniteGame.FortPlayerControllerAthena.ServerGiveCreativeItem", creative_component_hooks::ServerGiveCreativeItem_Hook);
    REGISTER_FUNCTION_DETOUR(creative_component_hooks::AddMiniGamePlayer, memory_manager::find_function_by_string(L".AddMinigamePlayer - %s added"), creative_component_hooks::AddMiniGamePlayer_Hook);
    REGISTER_FUNCTION_DETOUR(creative_component_hooks::RemoveMiniGamePlayer, memory_manager::find_function_by_string(L".RemoveMinigamePlayer - %s removed"), creative_component_hooks::RemoveMiniGamePlayer_Hook);
    REGISTER_FUNCTION_DETOUR(creative_component_hooks::SetTeamForPlayer, memory_manager::find_pattern("48 89 6C 24 ? 56 48 83 EC 20 80 B9 ? ? ? ? ? 41 0F B6 F0 48 8B EA 75 6E 48 8B 01 FF 90 ? ? ? ? 48 85 C0 74 60"), creative_component_hooks::SetTeamForPlayer_Hook);
    REGISTER_UFUNCTION_DETOUR(creative_component_hooks::ServerTeleportToPlaygroundLobbyIsland, L"/Script/FortniteGame.FortPlayerControllerAthena.ServerTeleportToPlaygroundLobbyIsland", creative_component_hooks::ServerTeleportToPlaygroundLobbyIsland_Hook);
    REGISTER_UFUNCTION_DETOUR(creative_component_hooks::ResetMyCurrentCreativePlot, L"/Script/FortniteGame.FortPlayerControllerAthena.ResetMyCurrentCreativePlot", creative_component_hooks::ResetMyCurrentCreativePlot_Hook);
    INIT_POINTER(LoadPlayset, memory_manager::find_function_by_string(L"UPlaysetLevelStreamComponent::LoadPlayset Error: no owner for %s"));

    REGISTER_CALL_DETOUR_WITH_SAVE(memory_manager::find_pattern("E8 ? ? ? ? 0F B6 F8 44 89 75 EF 44 39 75 F3 74 0B 33 D2 48 8D 4D E7 E8 ? ? ? ?"), creative_component_hooks::SaveArrayToFile);
    REGISTER_CALL_DETOUR_WITH_SAVE(memory_manager::find_pattern("E8 ? ? ? ? 84 C0 75 38 80 3D ? ? ? ? ? 72 2A 48 8B 05 ? ? ? ? 4C 8D 45 28 48 89 45 28"), creative_component_hooks::LoadFileToArray);
    
    DetourTransactionCommit();
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
    AddMiniGamePlayer(MiniGame, PlayerState);
    
    if(MiniGame->Volume())
    {
        if(MiniGame->Volume()->LinkedPortals()[0])
        {
            MiniGame->Volume()->LinkedPortals()[0]->CurrentPopulation()++;
            
            spdlog::info(L"creative_component:AddMiniGamePlayer_Hook: Updating Island population {}", MiniGame->Volume()->LinkedPortals()[0]->CurrentPopulation());
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
        Request.SetKey(account_id + "/" + sPlaysetName + ".sav");
        std::wstring VolumeName = Owner->GetName().c_str();
        TO_STRING(VolumeName, sVolumeName);
        std::string Path = AccountSavePath.append("/").append(sVolumeName).append("LevelSave.sav");
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
        }

        // Load level record
        LevelSaveComponent->LoadLevelRecord();
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
                            std::string Path = AccountSavePath.append("/").append(sLocalFileName).append("LevelSave.sav");
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

                        // TODO: Proper
                        auto NewVolume = (*GWorld)->GameState()->VolumeManager()->SpawnVolume(FindObject(L"/Game/Athena/BuildingActors/FortVolumeActor_PrefabGrenades.FortVolumeActor_PrefabGrenades_C", true), Playset, creative_portal->LinkedVolume()->K2_GetActorLocation(), creative_portal->LinkedVolume()->K2_GetActorRotation());
                        if (NewVolume) NewVolume->K2_DestroyActor();
                        
                        // Log
                        spdlog::info(L"Created and loaded Island ('{}') for portal '{}'", Plot->GetName().c_str(), creative_portal->GetName().c_str());
                    }
                    if (Component->IsA(UFortLevelSaveComponent::StaticClass()))
                    {
                        UFortLevelSaveComponent* LevelSaveComponent = static_cast<UFortLevelSaveComponent*>(Component);

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
                        
                        LevelSaveComponent->AccountIdOfOwner() = creative_portal->OwningPlayer();
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
    
            PlayerPawn->K2_TeleportTo(Portal->LinkedVolume()->K2_GetActorLocation(), FRotator());
            
            static_cast<AFortPlayerPawnAthena*>(PlayerPawn)->TeleportToSkyDive(18000);

            if (!PlayerPawn->Controller())
            {
                return;
            }
            
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

void creative_component_hooks::HandleMinigameStarted_Hook(UObject* Object, UObject*, void*)
{
    AFortMinigame* Minigame = static_cast<AFortMinigame*>(Object);
    
    TArrayManager Players = TArrayManager<FMinigamePlayer>(&Minigame->PlayerArray().Players(), FMinigamePlayer::GetStructSize());
    
    for (FMinigamePlayer& Player : Players)
    {
        if  (!Player.PlayerState() || !Player.PlayerState()->Owner())
        {
            continue;
        }
        
        AFortPlayerController* PlayerController = reinterpret_cast<AFortPlayerController*>(Player.PlayerState()->Owner());

        // Remove phone on minigame start ( in case game fails to )
        if (UFortItem* Item = inventory_component::get_inventory_item_by_definition(PlayerController, FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/Prototype/WID_CreativeTool.WID_CreativeTool")))
        {
            inventory_component::remove_item(PlayerController, Item->GetItemGuid(), -1, false);
        }

        if (!Player.ClaimedPlayerStart())
        {
            Player.ClaimedPlayerStart() = Minigame->ChoosePlayerStart(PlayerController);
        }
        
        if (Player.ClaimedPlayerStart())
        {
            // Destroy current pawn, and restart
            reinterpret_cast<AFortPlayerController*>(Player.PlayerState()->Owner())->Pawn()->K2_DestroyActor();
            
            (*GWorld)->AuthorityGameMode()->RestartPlayerAtPlayerStart(PlayerController, Player.ClaimedPlayerStart());

            Player.ClaimedPlayerStart() = nullptr;
        }
        else
        {
            // Get a respawn location
            FVector Loc;
            FRotator Rot;
            bool bOutRespawningOnGround;
            if (static_cast<AFortMinigame*>(Object)->DetermineSpawnLocation(static_cast<AFortPlayerStateAthena*>(Player.PlayerState()), &Loc, &Rot, &bOutRespawningOnGround))
            {
                // Destroy current pawn, and restart
                reinterpret_cast<AFortPlayerController*>(Player.PlayerState()->Owner())->Pawn()->K2_DestroyActor();
                
                (*GWorld)->AuthorityGameMode()->RestartPlayerAtTransform(PlayerController, UKismetMathLibrary::MakeTransform(Loc, Rot, FVector(1, 1, 1)));

                if (!bOutRespawningOnGround)
                {
                    PlayerController->Pawn()->BeginSkydiving(false);
                }
            }
        }
    }
}

void creative_component_hooks::HandleMinigameEnded_Hook(UObject* Object, UObject*, void*)
{
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
