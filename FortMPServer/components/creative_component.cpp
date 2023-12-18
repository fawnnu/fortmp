#include "include.hpp"
#include "creative_component.hpp"

#include "inventory_component.hpp"
#include "log_component.hpp"
#include "looting_component.hpp"
#include "player_component.hpp"

creative_component::creative_component()
{
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
    AddMiniGamePlayer(MiniGame, StackFrame);
    
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
    RemoveMiniGamePlayer(MiniGame, StackFrame);

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
    if (Portal && PlayerPawn && PlayerPawn->Controller())
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
                if (!Component)
                {
                    return 0;    
                }
            
                if (Component->IsA(UPlaysetLevelStreamComponent::StaticClass()))
                {
                    PlaysetComponent = static_cast<UPlaysetLevelStreamComponent*>(Component);
                }
            }

            // Get file buffer & upload on CDN bucket 'creativeislands'
            if (PlaysetComponent)
            {
                // Get owner's account ID
                std::string account_id;
                for (UNetConnection* Connection : (*GWorld)->NetDriver()->ClientConnections())
                {
                    if (UFortKismetLibrary::EqualEqual_UniqueNetIdReplUniqueNetIdRepl(Connection->PlayerID(), LevelSaveComponent->AccountIdOfOwner()))
                    {
                        account_id = player_component::get_account_id(Connection->PlayerController());
                    }
                }

                if (account_id.size() <= 0)
                {
                    return 0;
                }
            
                // The file name will be the playset's name
                std::wstring wPlaysetName = PlaysetComponent->CurrentPlayset()->GetName();
                TO_STRING(wPlaysetName, sPlaysetName);

                // Get the actual file name locally, in appdata
                char* Buf;
                size_t Len;
                _dupenv_s(&Buf, &Len, "LOCALAPPDATA");
                std::string Path(Buf);
                std::wstring VolumeName = Owner->GetName();
                TO_STRING(VolumeName, sVolumeName);
                Path = Path.append(R"(\FortniteGame\Saved\SaveGames\)").append(sVolumeName).append("LevelSave.sav");

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

        for (auto Component : Owner->BlueprintCreatedComponents())
        {
            if (Component->IsA(UFortLevelSaveComponent::StaticClass()))
            {
                // Attempt loading the island record from CDN
                std::string account_id;
                for (UNetConnection* Connection : (*GWorld)->NetDriver()->ClientConnections())
                {
                    if (UFortKismetLibrary::EqualEqual_UniqueNetIdReplUniqueNetIdRepl(Connection->PlayerID(), static_cast<UFortLevelSaveComponent*>(Component)->AccountIdOfOwner()))
                    {
                        account_id = player_component::get_account_id(Connection->PlayerController());
                    }
                }

                if (account_id.size() <= 0)
                {
                    return;
                }

                // Get playset name
                std::wstring PlaysetName = static_cast<UPlaysetLevelStreamComponent*>(Object)->CurrentPlayset()->GetName();
                TO_STRING(PlaysetName, sPlaysetName);

                // Perform request
                Aws::S3::Model::GetObjectRequest Request;
                Request.SetBucket("creativeislands");
                Request.SetKey(account_id + "/" + sPlaysetName + ".sav");
                char* Buf;
                size_t Len;
                _dupenv_s(&Buf, &Len, "LOCALAPPDATA");
                std::string Path(Buf);
                std::wstring VolumeName = Owner->GetName();
                TO_STRING(VolumeName, sVolumeName);
                Path = Path.append(R"(\FortniteGame\Saved\SaveGames\)").append(sVolumeName).append("LevelSave.sav");
                Request.SetResponseStreamFactory(
                [=]() {
                       return Aws::New<Aws::FStream>("dl", Path, std::ios_base::out | std::ios_base::binary);
                }
                );
                Aws::S3::Model::GetObjectOutcome outcome = gameserver_client::Client->S3Client->GetObject(Request);
                
                if (log_component::enable_debug_logs)
                {
                    spdlog::info(L"creative_component_hooks::OnFinishedLoading_Hook: finished loading playset, loading record from disk. Download status: [Size: {}, Successful: {}]", outcome.GetResult().GetContentLength(), outcome.IsSuccess());
                }

                // If we couldn't download but we have an island file locally, delete it before loading it.
                if (!outcome.IsSuccess())
                {
                    if (std::filesystem::exists(Path))
                    {
                        std::filesystem::remove(Path);
                    }
                }
                
                static_cast<UFortLevelSaveComponent*>(Component)->LoadLevelRecord();
                break;
            }
        }
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