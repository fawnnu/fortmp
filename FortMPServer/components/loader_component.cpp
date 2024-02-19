#include "include.hpp"
#include "loader_component.hpp"

#include "anticheat_component.hpp"
#include "bots_component.hpp"
#include "challenges_component.hpp"
#include "function_component.hpp"
#include "looting_component.hpp"
#include "net_component.hpp"
#include "pickup_component.hpp"
#include "team_component.hpp"
#include "config_manager.hpp"
#include "director_client.hpp"
#include "era_mutators_component.hpp"
#include "fort_mutators_component.hpp"
#include "log_component.hpp"
#include "player_component.hpp"
#include "playlist_component.hpp"
#include "replication_component.hpp"

#include "../gui/renderer.hpp"
#include "../utils/webhook_manager.hpp"

void loader_component::pre_init()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    // Season 5 crash fix
    if (GetEngineVersionEasy() < 421)
    {
        REGISTER_FUNCTION_DETOUR(loader_component_hooks::SetCurrentPlaylistId, memory_manager::find_function_by_string(L"PLAYLIST: Playlist Object found in AFortGameStateAthena::SetCurrentPlaylistId()"), loader_component_hooks::SetCurrentPlaylistId_Hook);
    }

    REGISTER_FUNCTION_DETOUR(loader_component_hooks::BuildingFoundation_PostUpdate, memory_manager::find_pattern("48 89 5C 24 ?? 57 48 83 EC ?? 49 8B F8 48 8B D9 E8 ?? ?? ?? ?? F6 83 ?? ?? 00 00"), loader_component_hooks::BuildingFoundation_PostUpdate_Hook);

    DetourTransactionCommit();
}

void loader_component::load_asset(FName Path, FName InAssetName)
{
    TArray<FAssetData> OutAssets;
    if (UAssetRegistryHelpers::GetAssetRegistry()->GetAssetsByPath(Path, OutAssets, true, false))
    {
        TArrayManager Assets = OutAssets;
        for (FAssetData& Asset : Assets)
        {
            if (Asset.PackageName() == InAssetName)
            {
                LoadObject(Asset.ObjectPath().c_str());
            }
        }

        // Log
        spdlog::info(L"loader_component::load_assets_at loaded {} assets.", Assets.Num());
    }
}

loader_component::loader_component()
{
    AFortPlayerController* Default__FortPlayerController = FindObject<AFortPlayerController>(L"/Script/FortniteGame.Default__FortPlayerController");

    if (const uintptr_t BeginPlayAddress = memory_manager::find_function_by_string(L"MusicManager class default object is null as FortPlayerController::BeginPlay is called"))
    {
        int BeginPlayIndex = 0;
    
        for (int i = 0; i < INT_MAX; i++)
        {
            if (Default__FortPlayerController->VTable[i] == reinterpret_cast<LPVOID>(BeginPlayAddress))
            {
                BeginPlayIndex = i;
            
                break;
            }
        }

        // Hook begin play for the classes we need
        REGISTER_VTABLE_HOOK(CreateDefaultObject<ABuildingContainer>()->VTable, BeginPlayIndex, loader_component_hooks::ABuildingContainer_BeginPlay);
        REGISTER_VTABLE_HOOK(CreateDefaultObject<AFortGameModeAthena>()->VTable, BeginPlayIndex, loader_component_hooks::AFortGameModeAthena_BeginPlay);
    }
    else
    {
        spdlog::error(L"Failed to hook loader_component BeginPlay. Invalid BeginPlayAddress");
    }
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    REGISTER_FUNCTION_DETOUR(loader_component_hooks::UpdateLevelStreaming, memory_manager::find_function_by_string(L"STAT_UpdateLevelStreamingTime"), loader_component_hooks::UpdateLevelStreaming_Hook);
    REGISTER_FUNCTION_DETOUR(loader_component_hooks::UpdateLevelStreaming, memory_manager::find_pattern("E8 ? ? ? ? 48 8B CF E8 ? ? ? ? 84 C0 75 0E FF 15 ? ? ? ? 84 C0 0F 84 ? ? ? ?", true, 1), loader_component_hooks::UpdateLevelStreaming_Hook);
    REGISTER_FUNCTION_DETOUR(loader_component_hooks::SetGameMode, memory_manager::find_function_by_string(L"Failed to spawn GameMode actor."), loader_component_hooks::SetGameMode_Hook);
    REGISTER_FUNCTION_DETOUR(loader_component_hooks::GetCalendarEventInformation, memory_manager::find_function_by_string(L"GetCalendarEventInformation(%s) called without an event manager"), loader_component_hooks::GetCalendarEventInformation_Hook);
    
    DetourTransactionCommit();

    REGISTER_PROCESS_EVENT_DETOUR(L"/Script/AIModule.EnvQueryInstanceBlueprintWrapper.EQSQueryDoneSignature__DelegateSignature", loader_component_hooks::EQSQueryDoneSignature__DelegateSignature_Hook);
    REGISTER_CUSTOM_UFUNCTION(OnGamePhaseUpdated, AFortGameStateAthena::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(WaitForMapInfo, AFortGameStateAthena::StaticClass());
    if (GetCL() == 4526925)
    {
        REGISTER_CUSTOM_UFUNCTION(SwapButterflyEventPOIs, AFortGameModeAthena::StaticClass());
    }
}

void loader_component_hooks::BuildingFoundation_PostUpdate_Hook(ABuildingFoundation* BuildingFoundation, uint8_t State,
    void* ReservedValues)
{
    // CL specific fix due to offset use on property
    try
    {
        auto config = config_manager::get_server_config();
        for (auto& elem : config["startenabled_foundations"].items())
        {
            if (GetCL() == elem.value()["target_cl"].get<int>())
            {
                std::string sFoundationName = elem.value()["foundation"].get<std::string>();
                TO_WSTRING(sFoundationName, wFoundationName);

                if (BuildingFoundation == FindObject<ABuildingFoundation>(wFoundationName.c_str()))
                {
                    if(BuildingFoundation->DynamicFoundationType() == 2)
                    {
                        // Enable the foundation (DynamicFoundationType is originally StartDisabled so we gotta set this)
                        UKismetSystemLibrary::SetBoolPropertyByName(BuildingFoundation, TEXT("bFoundationEnabled"), true);
                        BuildingFoundation->OnRep_FoundationEnabled(false);
        
                        BuildingFoundation->DynamicFoundationType() = 1;

                        BuildingFoundation->OnRep_DynamicFoundationTransform();
            
                        spdlog::warn(L"BuildingFoundation_PostUpdate: Enabling {}", UKismetSystemLibrary::GetPathName(BuildingFoundation).c_str());
                    }

                    if(BuildingFoundation->DynamicFoundationType() == 1)
                    {
                        // credit: Callum/darkblade/callumboynton for property setting
                        auto Property = FindPropertyByName(BuildingFoundation->Class, TEXT("bFoundationEnabled"));

                        if (Property)
                        {
                            uint8* ByteValue = reinterpret_cast<uint8*>(__int64(BuildingFoundation) + *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(Property) + CoreOffsets::Offset_Internal));
                            constexpr uint8 Mask = 0x8;
                            *ByteValue = *ByteValue & ~Mask | Mask;
                        }
                    }
                }
            }
        }
    }
    catch (nlohmann::json::exception& err)
    {
    }
    
    BuildingFoundation_PostUpdate(BuildingFoundation, State, ReservedValues);
    
    BuildingFoundation->OnRep_LevelToStream();
    
}

void loader_component::start_match(UFortPlaylistAthena* in_playlist)
{
    constexpr int listen_port = 7777;
    
    auto config = config_manager::get_server_config();

    // Get UI Ready
    if (!config["nullrhi"].get<bool>())
    {
        renderer::LoadAssets();
    }
    
    // Listen for connections
    const FURL in_url((*GWorld)->GetName().c_str(), listen_port);

    //make sure we weren't listening already
    if (net_component::listen(in_url))
    {
        // Load every asset in /Game/Athena/Items/Cosmetics
        {
            TArray<FAssetData> OutAssets;
            if (UAssetRegistryHelpers::GetAssetRegistry()->GetAssetsByPath(TEXT("/Game/Athena/Items/Cosmetics"), OutAssets, true, false))
            {
                TArrayManager Assets = OutAssets;
                for (FAssetData& Asset : Assets)
                {
                    LoadObject(Asset.ObjectPath().c_str());
                }

                // Log
                spdlog::info(L"loader_component::start_match loaded {} cosmetic assets.", Assets.Num());
            }
        }
        // Load every asset in /Game/Athena/Items/Quests
        {
            TArray<FAssetData> OutAssets;
            if (UAssetRegistryHelpers::GetAssetRegistry()->GetAssetsByPath(TEXT("/Game/Athena/Items/Quests"), OutAssets, true, false))
            {
                TArrayManager Assets = OutAssets;
                for (FAssetData& Asset : Assets)
                {
                    LoadObject(Asset.ObjectPath().c_str());
                }

                // Log
                spdlog::info(L"loader_component::start_match loaded {} quest assets.", Assets.Num());
            }
        }
        
        REGISTER_PROCESS_EVENT_DETOUR(L"/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations_C.SwapPOI", loader_component_hooks::SwapPOI_Hook);
        
        if (in_playlist == nullptr)
        {
            // See if server.json is overriding the playlist
            std::string sOverridePlaylist = config["override_playlist"];
            std::wstring wOverridePlaylist = std::wstring(sOverridePlaylist.begin(), sOverridePlaylist.end());

            if (UFortPlaylistAthena* override_playlist = FindObject<UFortPlaylistAthena>(wOverridePlaylist.c_str(), false, true); override_playlist && override_playlist->IsA(UFortPlaylistAthena::StaticClass()))
            {
                in_playlist = override_playlist;
            }
            else if (era_playlist* playlist = playlist_component::find_playlist(sOverridePlaylist))
            {
                if (STATIC_HASPROPERTY((*GWorld)->AuthorityGameMode(), CurrentPlaylistId))
                {
                    (*GWorld)->GameState()->CurrentPlaylistId() = playlist->PlaylistId;
                }
                if (STATIC_HASPROPERTY((*GWorld)->AuthorityGameMode(), bSquadPlay))
                {
                    (*GWorld)->AuthorityGameMode()->bSquadPlay() = playlist->bSquadPlay;
                }

                team_component::max_team_size_override = playlist->team_size;
            }

            bool bExists_bEnableCreativeMode = false;
            if (in_playlist)
            {
                bExists_bEnableCreativeMode = in_playlist->HasProperty(TEXT("bEnableCreativeMode"));
            }
            
            if (in_playlist == nullptr)
            {
                // Use default playlist if none specified or if it's creative (Disabled starting from v8.00 due to a lot of functionality being stripped)
                in_playlist = FindObject<UFortPlaylistAthena>(L"/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo");
            }
        }

        // Initialize playlist
        if (initialize_playlist(in_playlist) == false)
        {
            spdlog::error(L"loader_component::start_match failed: failed to initialize playlist.");
            
            return;
        }

        // Load additional playlist levels
        (*GWorld)->GameState()->HandlePlaylistStreamedLevels(in_playlist);

        // Set aircraft behavior
        if ((*GWorld)->GameState()->HasProperty(L"AirCraftBehavior") &&
            in_playlist != nullptr &&
            in_playlist->HasProperty(L"AirCraftBehavior"))
        {
            (*GWorld)->GameState()->AirCraftBehavior() = in_playlist->AirCraftBehavior();
        }
        
        // Update max players
        auto config = config_manager::get_server_config();
        update_max_players(config["player_cap_override"]);
        
        // Spawn all floor loot
        spawn_floor_loot(FindObject(L"Tiered_Athena_FloorLoot_01_C", false, true), L"Loot_AthenaFloorLoot");
        spawn_floor_loot(FindObject(L"Tiered_Athena_FloorLoot_Warmup_C", false, true), L"Loot_AthenaFloorLoot_Warmup");

        // Handle foundations
        handle_unloaded_foundations();
        
        // initialize vending machines
        initialize_vending_machines();

        // initialize BGAs
        initialize_bga();

        // Handle build specific additions
        handle_cl_specific_additions();

        // Initialize era_teams
        team_component::InitializeTeams();

        // Initialize bot managers and mutator
        bots_component::initialize_data();

        // Weapon patches
        looting_component::parse_weapon_hotfixes();

        // Initialize challenges hooks
        challenges_component::Initialize();
        
        // Update gamemode for clients
        (*GWorld)->GameState()->GameModeClass() = FindObject(L"/Game/Athena/Athena_GameMode.Athena_GameMode_C");

        (*GWorld)->GameState()->OnRep_GameModeClass();
        
        // Update default pawn class
        (*GWorld)->AuthorityGameMode()->DefaultPawnClass() = APlayerPawn_Athena_C::StaticClass();
        
        // setup warmup time
#ifdef _RELEASE
        (*GWorld)->AuthorityGameMode()->WarmupCountdownDuration() = config["warmup_duration"];
        (*GWorld)->AuthorityGameMode()->WarmupEarlyCountdownDuration() = config["warmup_duration"];
#elif _TESTING
        (*GWorld)->AuthorityGameMode()->WarmupCountdownDuration() = 600.f;
        (*GWorld)->AuthorityGameMode()->WarmupEarlyCountdownDuration() = 600.f;
#endif

        if ((*GWorld)->AuthorityGameMode()->HasProperty(L"EndGameKickPlayersDelay"))
        {
            (*GWorld)->AuthorityGameMode()->EndGameKickPlayersDelay() = 30.f;   
        }
        
        // Bind delegate for gamephase updates
        (*GWorld)->GameState()->GamePhaseChanged().Bind(TScriptDelegate((*GWorld)->GameState(), TEXT("OnGamePhaseUpdated")));
        
        // Start match
        if (GetEngineVersionEasy() > 423)
        {
            (*GWorld)->AuthorityGameMode()->StartMatch();
        }

#ifdef _RELEASE
        // Disable GC during match
        if (STATIC_HASPROPERTY((*GWorld)->AuthorityGameMode(), bDisableGCOnServerDuringMatch))
        {
            (*GWorld)->AuthorityGameMode()->bDisableGCOnServerDuringMatch() = true;
        }
#endif

        // Notify gamemode of server being ready
        (*GWorld)->AuthorityGameMode()->OnServerReady();

        // Implement waiter for Map Info
        (*GWorld)->GameState()->WaitForMapInfo();
        
        // Apply an optional keychain (stored in /Binaries/Win64 of your game version), keychains are used to decrypt encrypted paks which got started using around season 8
        // Example keychain file format (Server Only) (12.41, Travis Scott (Jerky) event): _:RELEASED:8734362B2A2A8B0FBC9EDE6160627E1D:8ZsoLeTeezxjoIxnNfUrNf61XfqMEKfnyTb3u4o/X6g=
        // Format: _:RELEASED:{PakGuid}:{PakAesKeyToBase64}:{(if applicable) CosmeticId}
        // Potentially could be moved to server_config on a later date.
        UKismetSystemLibrary::ExecuteConsoleCommand(L"ApplyContentKeyChain keychain");

#ifdef _RELEASE
            // Only on public servers
            /*if (gameserver_client::Client->GameServer_Status.private_matchmaking_key.length() == 0)
            {
                // Notify era services
                gameserver_client::Client->GameServer_Status.accepting_players = false;
                gameserver_client::Client->GameServer_Status.status = "ready";
                gameserver_client::Client->GameServer_Status.mode = GetPlaylistEasy() ? GetPlaylistEasy()->sGetName() : std::to_string((*GWorld)->GameState()->CurrentPlaylistId());
                gameserver_client::Client->SendUpdate();
            }
            else*/ // Run different code on private
            {
                gameserver_client::Client->GameServer_Status.accepting_players = true;
                gameserver_client::Client->GameServer_Status.status = "online";
                gameserver_client::Client->GameServer_Status.mode = GetPlaylistEasy() ? GetPlaylistEasy()->sGetName() : std::to_string((*GWorld)->GameState()->CurrentPlaylistId());
                gameserver_client::Client->SendUpdate(); 
            }

            /*
             * Tell the director the gameserver is ready
             */
            director_client::send_ready_event();
#endif
        
        // Set default floating island location on season 6
        if(GetFortniteVersionEasy() >= 6 && GetFortniteVersionEasy() < 7)
        {
            ABP_IslandScripting_C* island_scripting = ABP_IslandScripting_C::get_island_scripting();

            if (island_scripting)
            {
                if (!era_mutators_component::IsActive("no_cube"))
                {
                    FVector default_floating_island_position = island_scripting->IslandPosition();

                    // Activate the beam in the sky for fortnitemares
                    if (island_scripting->HasProperty(L"IsDeimosActive"))
                    {
                        island_scripting->IsDeimosActive() = true;
                    }

                    // Update the minimap on 6.21
                    if (island_scripting->HasProperty(L"UpdateMap"))
                    {
                        island_scripting->UpdateMap() = true;
                        island_scripting->OnRep_UpdateMap();
                    }

                    ACUBE_Simple_C* cube = island_scripting->CubeBP();

                    if (cube)
                    {
                        // Updated cube progress
                        cube->CrackProgress() = 1.0;
                        cube->DrainActive() = true;
                        cube->DrainAmount() = 1.0;
                    }

                    spdlog::warn(L"start_match: Floating island at location {} {} {}", ABP_IslandScripting_C::get_island_scripting()->GetName().c_str(), default_floating_island_position.X, default_floating_island_position.Y, default_floating_island_position.Z);
                }
                else
                {
                    island_scripting->DisableVortexGust() = true;
                    island_scripting->OnRep_DisableVortexGust();
                    island_scripting->MapIconState() = 1;
                    island_scripting->OnRep_MapIconState();
                    island_scripting->LevelActorsDeletedInt() = 1;
                    island_scripting->OnRep_LevelActorsDeletedInt();
                    island_scripting->UpdateMap() = true;
                    island_scripting->OnRep_UpdateMap();
                }
            }
        }

        if (!era_mutators_component::IsActive("domino_event"))
        {
            UObject* fortmp_bp_loader = FindObject(L"/Game/fortmp/content_simple/athena/maps/bp_fortmp_map_loader.bp_fortmp_map_loader_C", true, false);
            if (fortmp_bp_loader)
            {
                SpawnDefaultActor(*GWorld, fortmp_bp_loader);   
            }
        }
        else
        {
            (*GWorld)->GameState()->GamePhase() = 2; // Warmup
        }
        
        spdlog::info(L"Match has started in world '{}' and listening for connections", (*GWorld)->GetName().c_str());
    }
}

void loader_component::handle_unloaded_foundations()
{
    TArray<ABuildingFoundation*> BuildingFoundations = UGameplayStatics::GetAllActorsOfClass<ABuildingFoundation>(*GWorld, ABuildingFoundation::StaticClass());

    for (ABuildingFoundation* BuildingFoundation : BuildingFoundations)
    {
        if (!BuildingFoundation->HasProperty(TEXT("bFoundationEnabled")) || !BuildingFoundation->bFoundationEnabled())
        {
            try
            {
                auto config = config_manager::get_server_config();
                for (auto& elem : config["startenabled_foundations"].items())
                {
                    if (GetCL() == elem.value()["target_cl"].get<int>())
                    {
                        std::string sFoundationName = elem.value()["foundation"].get<std::string>();
                        TO_WSTRING(sFoundationName, wFoundationName);

                        if (BuildingFoundation == FindObject<ABuildingFoundation>(wFoundationName.c_str()))
                        {
                            // Enable the foundation (DynamicFoundationType is originally StartDisabled so we gotta set this)
                            UKismetSystemLibrary::SetBoolPropertyByName(BuildingFoundation, TEXT("bFoundationEnabled"), true);
                            BuildingFoundation->OnRep_FoundationEnabled(false);
                            BuildingFoundation->DynamicFoundationType() = 1;
                            BuildingFoundation->OnRep_DynamicFoundationTransform();
                            BuildingFoundation->EnableReadyForPostUpdate();

                            spdlog::warn(L"handle_unloaded_foundations: Enabling {} (As the game did not attempt loading it)", UKismetSystemLibrary::GetPathName(BuildingFoundation).c_str());
                            
                            loader_component_hooks::BuildingFoundation_PostUpdate(BuildingFoundation, 1, nullptr);
                        }
                    }
                }
            }
            catch (nlohmann::json::exception& err)
            {
            }
        }
    }
}

bool loader_component::initialize_playlist(UFortPlaylistAthena* in_playlist)
{
    if (loader_component_hooks::SetCurrentPlaylistId)
    {
        if (STATIC_HASPROPERTY(in_playlist, PlaylistId))
        {
            loader_component_hooks::SetCurrentPlaylistId((*GWorld)->GameState(), in_playlist->PlaylistId());

            return true;
        }

        return false;
    }
    else
    {
        if (STATIC_HASPROPERTY((*GWorld)->GameState(), CurrentPlaylistData))
        {
            (*GWorld)->GameState()->CurrentPlaylistData() = in_playlist;

            // Load playlist on the server
            (*GWorld)->GameState()->OnRep_CurrentPlaylistData();
        }
        else if (STATIC_HASPROPERTY((*GWorld)->GameState(), CurrentPlaylistInfo))
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
                spdlog::error(L"Could not set playlist '{}', as playlist data pointer is invalid",
                    in_playlist->GetName().c_str());
                
                return false;
            }
        }
    }
    
    return true;
}

void loader_component::update_max_players(int MaxPlayers)
{
    if ((*GWorld)->AuthorityGameMode()->GameSession())
    {
        // Update max player count
        (*GWorld)->AuthorityGameMode()->GameSession()->MaxPlayers() = MaxPlayers;
    }
}

int loader_component::get_max_players()
{
    if ((*GWorld)->AuthorityGameMode()->GameSession())
    {
        return (*GWorld)->AuthorityGameMode()->GameSession()->MaxPlayers();
    }
    
    return 100; // Default   
}

void loader_component::spawn_floor_loot(UObject* Class, FName LootTier)
{
    TArray<AActor*> floor_loot_actors = UGameplayStatics::GetAllActorsOfClass(*GWorld, Class);
        
    for (AActor* floor_loot_actor : floor_loot_actors)
    {
        std::map<UFortItemDefinition*, int> loot_to_spawn = looting_component::spawn_loot(LootTier);

        ABuildingContainer* floor_loot_container = static_cast<ABuildingContainer*>(floor_loot_actor);
        
        for (const auto& [ item_definition, count ] : loot_to_spawn)
        {
            // Spawn pickup with proper loot spawn location
            pickup_component::spawn_pickup(item_definition, floor_loot_container->CalculateLootSpawnLocation(), count, looting_component::get_default_loaded_ammo(item_definition));
        }

        // Set as searched if needed
        UKismetSystemLibrary::SetBoolPropertyByName(floor_loot_actor, L"bAlreadySearched", static_cast<ABuildingContainer*>(floor_loot_actor)->bStartAlreadySearched_Athena());

        // Actor might be dormant
        floor_loot_actor->ForceNetUpdate();
    }
}

void loader_component::initialize_vending_machines()
{
    TArray<AB_Athena_VendingMachine_C*> vending_machine_actors = UGameplayStatics::GetAllActorsOfClass<AB_Athena_VendingMachine_C>(*GWorld, FindObject(L"B_Athena_VendingMachine_C", false, true));
        
    for (AB_Athena_VendingMachine_C* vending_machine_actor : vending_machine_actors)
    {
        looting_component::set_vending_machine_loot(vending_machine_actor);
    }

    // destroy all vendings if the mutator exists
    era_mutators_component::Execute("NoVending", nullptr);
}

void loader_component::initialize_llamas()
{
    // Get gamestate
    AFortGameStateAthena* GameState = (*GWorld)->GameState();

    AFortAthenaMapInfo* MapInfo = GameState->MapInfo();
    
    if (MapInfo)
    {
        // Spawn the llamas
        if (MapInfo->HasProperty(L"LlamaQuantityMin") && MapInfo->HasProperty(L"LlamaQuantityMax"))
        {
            for (int i = 0; i < UKismetMathLibrary::RandomIntegerInRange(MapInfo->LlamaQuantityMin().GetValueAtLevel(0, nullptr), MapInfo->LlamaQuantityMax().GetValueAtLevel(0, nullptr)); i++)
            {
                // Pick random location
                if (PickSupplyDropLocation)
                {
                    while (true)
                    {
                        FVector SpawnLocation(0, 0, 0);
                        FVector Center(1, 1, 10000);
                        PickSupplyDropLocation(MapInfo, &SpawnLocation, &Center, 100000);

                        if (SpawnLocation.Z <= 0)
                        {
                            spdlog::error(L"loader_component::initialize_llamas Picked invalid location, retrying...");
                        
                            continue;
                        }

                        FTransform SpawnTransform;
                        SpawnTransform.Rotation = FQuat();
                        SpawnTransform.Translation = SpawnLocation;
                        SpawnTransform.Scale3D = FVector(1, 1, 1);
                        auto NewLlama = UGameplayStatics::BeginDeferredActorSpawnFromClass(*GWorld, MapInfo->LlamaClass(), SpawnTransform, 0, nullptr);

                        if (NewLlama)
                        {
                            // Get ground location, and finish spawning
                            SpawnTransform.Translation = reinterpret_cast<AFortAthenaSupplyDrop*>(NewLlama)->FindGroundLocationAt(SpawnTransform.Translation);

                            spdlog::info(L"Spawned a Llama supply drop [{} {} {}]", SpawnTransform.Translation.X, SpawnTransform.Translation.Y, SpawnTransform.Translation.Z);
                        
                            UGameplayStatics::FinishSpawningActor(NewLlama, SpawnTransform);
                        }
                        else
                        {
                            continue;
                        }

                        break;
                    }
                }
                else
                {
                    spdlog::error(L"loader_component::initialize_llamas failed, invalid PickSupplyDropLocation");
                }
            }
        }
    }
    else
    {
        spdlog::error(L"loader_component::initialize_llamas failed, invalid MapInfo");
    }
}

void loader_component::initialize_bga()
{
    TArray<ABGAConsumableSpawner*> BGAConsumableSpawners = UGameplayStatics::GetAllActorsOfClass<ABGAConsumableSpawner>(*GWorld, ABGAConsumableSpawner::StaticClass());

    for (ABGAConsumableSpawner* BGAConsumableSpawner : BGAConsumableSpawners)
    {
        if (UEnvQueryInstanceBlueprintWrapper* QueryInstance = UEnvQueryManager::RunEQSQuery(*GWorld, FindObject<UEnvQuery>(L"/Game/Athena/BuildingActors/ConsumableBGAs/EQS/EQS_ConsumableBGAPlacement_Circle_M.EQS_ConsumableBGAPlacement_Circle_M", true) ? FindObject<UEnvQuery>(L"/Game/Athena/BuildingActors/ConsumableBGAs/EQS/EQS_ConsumableBGAPlacement_Circle_M.EQS_ConsumableBGAPlacement_Circle_M", true) : BGAConsumableSpawner->QueryTemplate(), BGAConsumableSpawner, 3, nullptr))
        {
            QueryInstance->OnQueryFinishedEvent().Bind(TScriptDelegate(QueryInstance, L"EQSQueryDoneSignature__DelegateSignature"));
        }
    }
}

void loader_component::handle_duplicated_foundations()
{
    // DEPRECATED
    std::vector<FVector> FoundationLocationsList;

    for (ABuildingFoundation* BuildingFoundation : UGameplayStatics::GetAllActorsOfClass<ABuildingFoundation>(*GWorld, ABuildingFoundation::StaticClass()))
    {
        // Get location
        FVector Location = BuildingFoundation->K2_GetActorLocation();

        // If this location has already been added to the Foundations List 
        if (std::find(FoundationLocationsList.begin(), FoundationLocationsList.end(), Location) != FoundationLocationsList.end())
        {
            if (FindPropertyByName(ABuildingFoundation::StaticClass(), TEXT("bServerStreamedInLevel")))
            {
                UKismetSystemLibrary::SetBoolPropertyByName(BuildingFoundation, TEXT("bServerStreamedInLevel"), false);

                // Log
                spdlog::info(L"Unloaded '{}': location is overridden.", UKismetSystemLibrary::GetPathName(BuildingFoundation).c_str());
            }
        }
        else
        {
            FoundationLocationsList.push_back(Location);
        }
    }   
}

void loader_component::handle_cl_specific_additions()
{
    // Spawn the week 4 star, on 4.1
    if (GetCL() == 4053532)
    {
        FVector SpawnLocation = FVector(-44330, 59200, 1230);
        FRotator SpawnRotation = FRotator();
        FActorSpawnParameters SpawnParameters{};
        if (UObject* SpawnedActor = SpawnActor(*GWorld, FindObject(L"/Game/Athena/Items/QuestInteractables/HiddenStars/Prop_HiddenStars_Interactable_Triangulate_02.Prop_HiddenStars_Interactable_Triangulate_02_C"), &SpawnLocation, &SpawnRotation, SpawnParameters))
        {
            spdlog::info(L"Successfully spawned '{}' at [{} {} {}]", UKismetSystemLibrary::GetPathName(SpawnedActor).c_str(), SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
        }
    }

    // Execute the rift_s4_finale mutator
    if (GetCL() == 4159770)
    {
        era_mutators_component::Execute("rift_s4_finale_no_cracks", nullptr);
        era_mutators_component::Execute("rift_s4_finale", nullptr);
    }

    // Execute the cube_s5 mutator
    if (GetCL() == 4363240)
    {
        era_mutators_component::Execute("cube_spawn", nullptr);
        era_mutators_component::Execute("birthday_bus", nullptr);
    }

    // 1.11 & S7, christmas bus
    if (GetCL() == 3807424 || GetFortniteVersionEasy() >= 7 && GetFortniteVersionEasy() < 8)
    {
        era_mutators_component::Execute("winter_bus", nullptr);
    }

    if (GetFortniteVersionEasy() >= 7 && GetFortniteVersionEasy() < 8)
    {
        era_mutators_component::Execute("holiday_supplydrop", nullptr);
    }
    if (GetCL() == 4526925)
    {
        era_mutators_component::Execute("halloween_bus", nullptr);
        era_mutators_component::Execute("no_cube", nullptr);
    }
    
    // Load dynamic builder class
    if (GetCL() >= 4363240)
    {
        StaticLoadObject(nullptr, nullptr, L"/Script/FortniteGame.FortDynamicBuilder", nullptr, 0, nullptr, true);
    }
}

void loader_component::OnMapInfoLoaded()
{
    // handle editing/other mutators
    if (STATIC_HASPROPERTY((*GWorld)->GameState(), DefaultAllowNeutralWallEditing))
    {
        if (log_component::enable_debug_logs)
        {
            spdlog::info("loader_component::start_match: Pre-check DefaultAllowNeutralWallEditing: {}", (*GWorld)->GameState()->DefaultAllowNeutralWallEditing());
        }
        
        if (era_mutators_component::IsActive("neutral_edit"))
        {
            (*GWorld)->GameState()->DefaultAllowNeutralWallEditing() = 1.0;       
        }
        else
        {
            (*GWorld)->GameState()->DefaultAllowNeutralWallEditing() = 0.0;       
        }

        if (log_component::enable_debug_logs)
        {
            spdlog::info("loader_component::start_match: Post-check DefaultAllowNeutralWallEditing: {}", (*GWorld)->GameState()->DefaultAllowNeutralWallEditing());
        }
    }
    else
    {
        if (log_component::enable_debug_logs)
        {
            spdlog::info("loader_component::start_match: DefaultAllowNeutralWallEditing is not a valid property.");
        }
    }
    
    // Implement llamas
    initialize_llamas();

    // Butterfly event storm mutator
    era_mutators_component::Execute("butterfly_begin", nullptr);
}

void loader_component::OnGamePhaseUpdated(AFortGameStateAthena* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

#ifdef _RELEASE
        static UEnum* GamePhaseEnum = FindObject<UEnum>(TEXT("/Script/FortniteGame.EAthenaGamePhase"));
        uint8_t Aircraft = GamePhaseEnum->GetEnumIndex(TEXT("Aircraft"));

    if (!era_mutators_component::IsActive("domino_event"))
    {
        if (Self->GamePhase() == Aircraft)
        {
            gameserver_client::Client->GameServer_Status.accepting_players = false;
            gameserver_client::Client->SendUpdate();

            player_component::starting_players = gameserver_client::Client->GameServer_Status.number_of_players;

            /*
             * Send a match_started event to director
             */
            if (director_client::client)
            {
                director_client::client->Send(eudp_packet({
                    {"type", "match_started"},
                    {"payload", {
                        {"starting_players", player_component::starting_players}
                    }}
                }));
            }
        }
    }
#endif

    // Mark bMatchHasStarted as false for every gamephase update, if we're playing creative
    UFortPlaylistAthena* ServerPlaylist = GetPlaylistEasy();

    if (ServerPlaylist)
    {
        fort_mutators_component::on_game_phase_updated(Self->GamePhase());

        if (era_mutators_component::IsActive("domino_event"))
        {
            (*GWorld)->AuthorityGameMode()->bMatchHasStarted() = false;
        }
        
        // Can't use STATIC_HASPROPERTY as the playlist can change
        if (ServerPlaylist->HasProperty(TEXT("bEnableCreativeMode")))
        {
            // Check if creative mode is enabled
            if (ServerPlaylist->bEnableCreativeMode())
            {
                // Allow join in progress
                (*GWorld)->AuthorityGameMode()->bMatchHasStarted() = false;
            }
        }
    }
}

void loader_component::WaitForMapInfo(AFortGameStateAthena* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self->MapInfo())
    {
        OnMapInfoLoaded();

        if (*GWorld && (*GWorld)->AuthorityGameMode())
        {
            (*GWorld)->AuthorityGameMode()->OnServerMapInfoReady();
        }
    }
    else
    {
        UKismetSystemLibrary::K2_SetTimer(Self, L"WaitForMapInfo", 1.0, false);
    }
}

void loader_component::SwapButterflyEventPOIs(AFortGameModeAthena* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    static auto FoundationsToDisable = std::vector{
        FindObject<ABuildingFoundation>(TEXT("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Lake1")),
        FindObject<ABuildingFoundation>(TEXT("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_FloatingIsland"))
    };

    if (FoundationsToDisable.size() > 0)
    {
        ABuildingFoundation* FoundationToDisable = FoundationsToDisable[FoundationsToDisable.size() - 1];
        std::erase(FoundationsToDisable, FoundationToDisable);

        spdlog::warn(L"loader_component_hooks::SwapPOI_Hook is disabling POI '{}'.", UKismetSystemLibrary::GetPathName(FoundationToDisable).c_str());
    
        UKismetSystemLibrary::SetBoolPropertyByName(FoundationToDisable, TEXT("bFoundationEnabled"), false);
        UKismetSystemLibrary::SetBoolPropertyByName(FoundationToDisable, TEXT("bServerStreamedInLevel"), false);
        FoundationToDisable->SetDynamicFoundationEnabled(false);
        FoundationToDisable->OnRep_FoundationEnabled(true);
        FoundationToDisable->OnRep_ServerStreamedInLevel();
        FoundationToDisable->OnRep_DynamicFoundationTransform();
        FName LevelToUnload = FoundationToDisable->LevelToStream();
        loader_component_hooks::BuildingFoundation_PostUpdate(FoundationToDisable, 0, nullptr);
        FoundationToDisable->OnRep_LevelToStream();
        FoundationToDisable->DisableReadyForPostUpdate();
        UGameplayStatics::UnloadStreamLevel(*GWorld, LevelToUnload, FLatentActionInfo(0, 0, TEXT("SwapButterflyEventPOIs"), Self), false);
        return;
    }

    if (FoundationsToDisable.size() == 0)
    {
        if (ABuildingFoundation* Lake2Foundation = FindObject<ABuildingFoundation>(TEXT("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Lake2")))
        {
            if (Lake2Foundation->DynamicFoundationType() == 2)
            {
                UKismetSystemLibrary::SetBoolPropertyByName(Lake2Foundation, TEXT("bFoundationEnabled"), true);
                Lake2Foundation->OnRep_FoundationEnabled(false);
                Lake2Foundation->DynamicFoundationType() = 1;
                Lake2Foundation->OnRep_DynamicFoundationTransform();
            }
            
            if (Lake2Foundation->DynamicFoundationType() == 1)
            {
                Lake2Foundation->EnableReadyForPostUpdate();
            }
            
            loader_component_hooks::BuildingFoundation_PostUpdate(Lake2Foundation, 1, nullptr);
        }
    }
}

void loader_component_hooks::UpdateLevelStreaming_Hook(UWorld* World)
{
    // Call original function first
    UpdateLevelStreaming(World);

    static FName NAME_WaitingToStart(L"WaitingToStart");
    
    if (World->GameState() && World->GameState()->MatchState() == NAME_WaitingToStart)
    {
        if (World->GameState()->WorldManager())
        {
            if (World->AuthorityGameMode())
            {
                UObject* NewGameSessionClass = AFortGameSessionDedicatedAthena::StaticClass() ? AFortGameSessionDedicatedAthena::StaticClass() : AFortGameSessionDedicated::StaticClass();
                
                if (World->AuthorityGameMode()->GameSession() &&
                    !World->AuthorityGameMode()->GameSession()->IsA(NewGameSessionClass))
                {
                    World->AuthorityGameMode()->GameSession() = SpawnDefaultActor<AGameSession>(World, NewGameSessionClass);
                    World->AuthorityGameMode()->FortGameSession() = World->AuthorityGameMode()->GameSession();
                }
            
                // allows vehicles to spawn
                if (STATIC_HASPROPERTY((*GWorld)->AuthorityGameMode(), bMapInfoInitialized))
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
                loader_component::start_match((*GWorld)->AuthorityGameMode()->OnSetPlaylist());
            }
        }
    }
}

void loader_component_hooks::EQSQueryDoneSignature__DelegateSignature_Hook(UObject* Object, UObject* Function, void* Params)
{
    UEnvQueryInstanceBlueprintWrapper* QueryInstance = static_cast<UEnvQueryInstanceBlueprintWrapper*>(Object);

    UObject* Querier = QueryInstance->QueryResult().Get()->Owner().Get();

    if (Querier->IsA(ABGAConsumableSpawner::StaticClass()))
    {
        ABGAConsumableSpawner* ConsumableSpawner = static_cast<ABGAConsumableSpawner*>(Querier);
        
        FFortLootTierData* LootTierData = looting_component::get_random_loot_tier_data(ConsumableSpawner->SpawnLootTierGroup());
        FFortLootPackageData* LootPackageData = looting_component::get_random_loot_package_data(LootTierData);

        if (LootTierData == nullptr)
        {
            return;
        }
        if (LootPackageData == nullptr)
        {
            return;
        }
        
        UBGAConsumableWrapperItemDefinition* ItemDefinition = static_cast<UBGAConsumableWrapperItemDefinition*>(LootPackageData->ItemDefinition().Get());

        if (ItemDefinition->IsA(UBGAConsumableWrapperItemDefinition::StaticClass()))
        {
            UObject* ConsumableClass = ItemDefinition->ConsumableClass().Get();

            // Get the query results as locations
            TArray<FVector> Locations = QueryInstance->GetResultsAsLocations();

            const int NumActorsToSpawn = LootTierData->NumLootPackageDrops();

            if (Locations.Num() >= NumActorsToSpawn)
            {
                // Now get the first locations and spawn an actor for each one of them
                for (int i = 0; i < NumActorsToSpawn; i++)
                {
                    FActorSpawnParameters SpawnActorParameters{};

                    SpawnActor(*GWorld, ConsumableClass, &Locations[i], nullptr, SpawnActorParameters);
                }
            }
        }
    }
}

void loader_component_hooks::SwapPOI_Hook(UObject* Object, UObject* Function, void* Params)
{
    if (*GWorld)
    {
        if ((*GWorld)->AuthorityGameMode())
        {
            // Player goes into the butterfly level; swap POI
            (*GWorld)->AuthorityGameMode()->SwapButterflyEventPOIs();

            // Storm resume
            era_mutators_component::Execute("butterfly_end", nullptr);
        }
    }
}

void loader_component_hooks::GetCalendarEventInformation_Hook(UObject* WorldContextObject, FString* EventName, bool* bIsEventActive, FTimespan* TimeSinceBegin, FTimespan* TimeUntilEnd, float* TimespanRatio)
{
    // Do nothing for now;
    if (EventName->IsValid())
    {
        const std::string Name = EventName->AnsiString();

        if (era_mutators_component::IsActive(Name))
        {
            if (bIsEventActive)
            {
                *bIsEventActive = true;

                if (log_component::enable_debug_logs)
                {
                    spdlog::info("loader_component_hooks::GetCalendarEventInformation_Hook: activating '{}' event (Valid Era Mutator).", Name.c_str());
                }
            }
        }
    }
}

void loader_component_hooks::AFortGameModeAthena_BeginPlay_Hook(AFortGameModeAthena* Target)
{
    // Mark world as ready
    UKismetSystemLibrary::SetBoolPropertyByName(Target, L"bWorldIsReady", true);

    AFortGameModeAthena_BeginPlay(Target);
}

void loader_component_hooks::ABuildingContainer_BeginPlay_Hook(ABuildingContainer* Target)
{
    ABuildingContainer_BeginPlay(Target);

    if (looting_component::get_random_loot_tier_data(looting_component::redirect_tier_group(Target->SearchLootTierGroup())) == nullptr)
    {
        UKismetSystemLibrary::SetBoolPropertyByName(Target, TEXT("bAlreadySearched"), true);
        
        // Actor might be dormant
        Target->ForceNetUpdate();
    }
}

bool loader_component_hooks::SetGameMode_Hook()
{
    if (*GWorld)
    {
#ifdef _RELEASE
        if (gameserver_client::Client)
        {
            // Notify era services
            gameserver_client::Client->GameServer_Status.status = "offline";
            gameserver_client::Client->GameServer_Status.accepting_players = false;
            gameserver_client::Client->GameServer_Status.number_of_players = 0;
            gameserver_client::Client->GameServer_Status.mode = "";
            gameserver_client::Client->SendUpdate();

            /*
             * Notify director
             */
            if (director_client::client)
            {
                director_client::client->Send(eudp_packet({
                    {"type", "offline"},
                    {"payload", nlohmann::json()}
                }));
            }
            
            // Leave channel
            gameserver_client::Client->SendLeave();

            // Re-register
            gameserver_client::Client->Register();
        }
#endif

        if (loader_component::bPendingShutdown)
        {
            // Log
            spdlog::info(L"Closing server: bPendingShutdown is true.");
            
            // Unregister the server
            gameserver_client::Client->SendLeave();
            
            // Close director connection
            if (director_client::client)
            {
                director_client::client->Close();
            }
            
            // Quit
            RequestExitWithStatus(true, 0);
        }

        // Clear temporary arrays
        team_component::TeamsMap.clear();
        team_component::TeamsList.clear();
        team_component::MainTeamsList.clear();
        player_component::EncryptionTokenList.clear();
        player_component::global_player_id = 0;
        player_component::starting_players = 0;
        loader_component::bLogged85Percent = false;
        loader_component::bLogged95Percent = false;
        player_component::Matches.clear();
        pickup_component::GlobalPickupStream.clear();
        anticheat_component::IllegalSniperActionsCount.clear();
        anticheat_component::Initialize();
        replication_component::BlacklistedActor.clear();
        net_component::ReplicationBlacklistPerConnection.clear();
        
        AFortGameModeAthena* AuthorityGameMode = SpawnDefaultActor<AFortGameModeAthena>(*GWorld, FindObject(L"/Game/Athena/Server/Athena_GameMode.Athena_GameMode_C", true));

        if (AuthorityGameMode == nullptr)
        {
            if (era_mutators_component::IsActive("domino_event"))
            {
                AuthorityGameMode = SpawnDefaultActor<AFortGameModeAthena>(*GWorld, FindObject(L"/Game/fortmp/content_complex/domino/StellarBellum_Content/StellarBellum_GameMode.StellarBellum_GameMode_C", true));
            }
            else
            {
                AuthorityGameMode = SpawnDefaultActor<AFortGameModeAthena>(*GWorld, FindObject(L"/Game/Athena/Athena_GameMode.Athena_GameMode_C", true));
            }
        }

        if (AuthorityGameMode && (*GWorld)->AuthorityGameMode() == nullptr)
        {
            (*GWorld)->AuthorityGameMode() = AuthorityGameMode;

            spdlog::info(L"loader_component_hooks::SetGameMode_Hook: AuthorityGameMode created.");
            
            return true;
        }
    }

    return false;
}

void loader_component_hooks::SetCurrentPlaylistId_Hook(AFortGameStateAthena* Self, int PlaylistId)
{
    // Steal the function   
}