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
#include "era_mutators_component.hpp"
#include "player_component.hpp"
#include "playlist_component.hpp"

#include "../gui/renderer.hpp"
#include "../utils/webhook_manager.hpp"

void loader_component::pre_init()
{
    // Season 5 crash fix
    if (GetEngineVersionEasy() < 421)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
    
        REGISTER_FUNCTION_DETOUR(loader_component_hooks::SetCurrentPlaylistId, memory_manager::find_function_by_string(L"PLAYLIST: Playlist Object found in AFortGameStateAthena::SetCurrentPlaylistId()"), loader_component_hooks::SetCurrentPlaylistId_Hook);

        DetourTransactionCommit();
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
        REGISTER_VTABLE_HOOK(CreateDefaultObject<ABuildingFoundation>()->VTable, BeginPlayIndex, loader_component_hooks::ABuildingFoundation_BeginPlay);

        if (const AFortAthenaExitCraftSpawner* DefaultObject = CreateDefaultObject<AFortAthenaExitCraftSpawner>())
        {
            REGISTER_VTABLE_HOOK(DefaultObject->VTable, BeginPlayIndex, loader_component_hooks::AFortAthenaExitCraftSpawner_BeginPlay);

            static UFunction* FunctionObject = FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaExitCraftSpawner.StartExitCraftSpawnTimer");

            if(FunctionObject)
            {
                auto start = memory_manager::scan_for(reinterpret_cast<uintptr_t>(FunctionObject->get_native_function()), {0xE9}, 0, false);
                
                auto result = (((uintptr_t)start + 5) + *(int32_t*)((uintptr_t)start + 1));
                
                auto stripped_leas = memory_manager::get_all_stripped_leas_in_function(result);

                for (int i = 0; i < stripped_leas.size(); ++i)
                {
                    uintptr_t func = stripped_leas[i];
                    
                    memory_manager::lea_hook(func, reinterpret_cast<uintptr_t>(ExitCraftOnTimerFinished));
                }
            }
        }
    }
    else
    {
        spdlog::error(L"Failed to hook ABuildingContainer_BeginPlay. Invalid BeginPlayAddress");
    }
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    REGISTER_FUNCTION_DETOUR(loader_component_hooks::UpdateLevelStreaming, memory_manager::find_function_by_string(L"STAT_UpdateLevelStreamingTime"), loader_component_hooks::UpdateLevelStreaming_Hook);
    REGISTER_FUNCTION_DETOUR(loader_component_hooks::UpdateLevelStreaming, memory_manager::find_pattern("E8 ? ? ? ? 48 8B CF E8 ? ? ? ? 84 C0 75 0E FF 15 ? ? ? ? 84 C0 0F 84 ? ? ? ?", true, 1), loader_component_hooks::UpdateLevelStreaming_Hook);
    REGISTER_FUNCTION_DETOUR(loader_component_hooks::SetGameMode, memory_manager::find_function_by_string(L"Failed to spawn GameMode actor."), loader_component_hooks::SetGameMode_Hook);
    
    DetourTransactionCommit();

    REGISTER_PROCESS_EVENT_DETOUR(L"/Script/AIModule.EnvQueryInstanceBlueprintWrapper.EQSQueryDoneSignature__DelegateSignature", loader_component_hooks::EQSQueryDoneSignature__DelegateSignature_Hook);
    REGISTER_CUSTOM_UFUNCTION(OnGamePhaseUpdated, AFortGameStateAthena::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(WaitForMapInfo, AFortGameStateAthena::StaticClass());
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
            
            if (in_playlist == nullptr || GetFortniteVersionEasy() >= 8 && bExists_bEnableCreativeMode && in_playlist->bEnableCreativeMode() == true)
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

        // initialize vending machines
        initialize_vending_machines();

        // initialize BGAs
        initialize_bga();

        // Disable duplicate building foundations
        handle_duplicated_foundations();

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
        
        // Mark world as ready
        UKismetSystemLibrary::SetBoolPropertyByName((*GWorld)->AuthorityGameMode(), L"bWorldIsReady", true);

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
#endif

        spdlog::info(L"Match started in world '{}' and listening for connections",
            (*GWorld)->GetName().c_str());
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

                        // Log
                        spdlog::info(L"Picked LLAMA SpawnLocation [{} {} {}]", SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);

                        FTransform SpawnTransform;
                        SpawnTransform.Rotation = FQuat();
                        SpawnTransform.Translation = SpawnLocation;
                        SpawnTransform.Scale3D = FVector(1, 1, 1);
                        auto NewLlama = UGameplayStatics::BeginDeferredActorSpawnFromClass(*GWorld, MapInfo->LlamaClass(), SpawnTransform, 0, nullptr);

                        if (NewLlama)
                        {
                            // Get ground location, and finish spawning
                            SpawnTransform.Translation = reinterpret_cast<AFortAthenaSupplyDrop*>(NewLlama)->FindGroundLocationAt(SpawnTransform.Translation);

                            spdlog::info(L"Finished spawning LLAMA [{} {} {}]", SpawnTransform.Translation.X, SpawnTransform.Translation.Y, SpawnTransform.Translation.Z);
                        
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

        // Load dynamic builder class
        StaticLoadObject(nullptr, nullptr, L"/Script/FortniteGame.FortDynamicBuilder", nullptr, 0, nullptr, true);
    }

    // 1.11, christmas bus
    if (GetCL() == 3807424)
    {
        era_mutators_component::Execute("winter_bus", nullptr);
    }
}

void loader_component::OnMapInfoLoaded()
{
    // Implement llamas
    initialize_llamas();
}

void loader_component::OnGamePhaseUpdated(AFortGameStateAthena* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

#ifdef _RELEASE
    static UEnum* GamePhaseEnum = FindObject<UEnum>(TEXT("/Script/FortniteGame.EAthenaGamePhase"));
    uint8_t Aircraft = GamePhaseEnum->GetEnumIndex(TEXT("Aircraft"));

    if (Self->GamePhase() == Aircraft)
    {
        gameserver_client::Client->GameServer_Status.accepting_players = false;
        gameserver_client::Client->SendUpdate();

        player_component::starting_players = gameserver_client::Client->GameServer_Status.number_of_players;
    }
#endif

    // Mark bMatchHasStarted as false for every gamephase update, if we're playing creative
    UFortPlaylistAthena* ServerPlaylist = GetPlaylistEasy();

    if (ServerPlaylist)
    {
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
    }
    else
    {
        UKismetSystemLibrary::K2_SetTimer(Self, L"WaitForMapInfo", 1.0, false);
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

void loader_component_hooks::ABuildingFoundation_BeginPlay_Hook(ABuildingFoundation* Target)
{
    ABuildingFoundation_BeginPlay(Target);

    if (Target)
    {
        // Set the foundation type
        if (STATIC_HASPROPERTY(Target, DynamicFoundationType))
        {
            Target->DynamicFoundationType() = 0;
        }

        // Mark the level as streamed in
        if (STATIC_HASPROPERTY(Target, bServerStreamedInLevel))
        {
            UKismetSystemLibrary::SetBoolPropertyByName(Target, TEXT("bServerStreamedInLevel"), true);
        }
    }
}

void loader_component_hooks::AFortAthenaExitCraftSpawner_BeginPlay_Hook(AFortAthenaExitCraftSpawner* Target)
{
    AFortAthenaExitCraftSpawner_BeginPlay(Target);

    if (Target)
    {
        // Start the timer to spawn ExitCrafts
        Target->StartExitCraftSpawnTimer();
        
        spdlog::info(L"ExitCraft {} has just started a spawn timer.", Target->GetName().c_str());
    }
}

void loader_component::ExitCraftOnTimerFinished(AFortAthenaExitCraftSpawner* ExitCraftSpawner, FFrame& StackFrame)
{
    if (ExitCraftSpawner)
    {
        spdlog::info(L"Exit Craft Spawner timer finished, spawning!");
        
        AFortGameModeAthena* game_mode = (*GWorld)->AuthorityGameMode();
        AFortGameStateAthena* game_state = (*GWorld)->GameState();
        
	    if (!game_mode) return;
	    if (!game_state) return;

        AFortAthenaMutator_Heist* heist_mutator = nullptr;

        TArray<AActor*> out_actors = UGameplayStatics::GetAllActorsOfClass(*GWorld, AFortAthenaMutator_Heist::StaticClass());

        for (AActor* out_actor : out_actors)
        {
            heist_mutator = static_cast<AFortAthenaMutator_Heist*>(out_actor);
        }

        if (!heist_mutator)
        {
            spdlog::error(L"Couldn't find Heist mutator!");
            return;
        }

        FHeistExitCraftData* CraftData = heist_mutator->FindCraftData(ExitCraftSpawner);
        if (!CraftData)
        {
            spdlog::error(L"CraftData was not found!");
            return;
        }
  
        heist_mutator->NumSpawnedExitCrafts()++;
        heist_mutator->NumUnspawnedExitCrafts()--;
  
        UFortAthenaExitCraftInfo* spawn_info = ExitCraftSpawner->ExitCraftInfo();
  
        UObject* van_class = ExitCraftSpawner->ExitCraftInfo()->ExitCaftClass();
  
        FVector location = ExitCraftSpawner->K2_GetActorLocation();
  
        FVector start_location = location;
        start_location.Z += spawn_info->ExitCraftInfo().ExitCraftZOffset().GetValueAtLevel(0, nullptr);
	  
        FRotator rotation = ExitCraftSpawner->K2_GetActorRotation();
        
        FActorSpawnParameters spawn_parameters = {};

        AFortAthenaExitCraft* van = static_cast<AFortAthenaExitCraft*>(SpawnActor(*GWorld, van_class, &start_location, &rotation, spawn_parameters));
        if (!van) return;

        van->ExitCraftInfo() = spawn_info;

        van->CurrentState() = 1;
        //van->OnNewState(1);

        van->CraftSpecialActorID() = ExitCraftSpawner->SpawnerSpecialActorID();
  
        CraftData->SpawnedExitCraft() = van;
        CraftData->ExitCraftState() = 2;
        CraftData->SpawnTime() = (*GWorld)->TimeSeconds();
        CraftData->bIsUsed() = false;
        CraftData->bHasDeparted() = false;
  
        heist_mutator->OnExitCraftSpawned(van, ExitCraftSpawner);
  
        FVector TargetLocation = location;
        TargetLocation.Z += spawn_info->ExitCraftInfo().ExitCraftTargetZOffset().GetValueAtLevel(0, nullptr);
  
        van->LocationLerpData().PositionLerp_Start() = start_location;
        van->LocationLerpData().PositionLerp_End() = TargetLocation;
        van->LocationLerpData().PositionLerp_Target() = TargetLocation;
        van->LocationLerpData().TotalLerpTime() = spawn_info->ExitCraftInfo().ExitCraftTimeToHoverLocation().GetValueAtLevel(0, nullptr);
  
        van->OnRep_LocationLerpData();
        
        auto SpecialActorData = game_state->SpecialActorData();
        FSpecialActorRepDataArray* actorRepList = &SpecialActorData->SpecialActorRepList();
        auto SpecialActorList = TArrayManager<FSpecialActorRepData>(&actorRepList->SpecialActorList(), FSpecialActorRepData::GetStructSize());

	    int SpawnerIndex = -1;
	    FSpecialActorRepData* SpawnerRepData = nullptr;

        for (int i = 0; i < SpecialActorList.Num(); ++i)
        {
            FSpecialActorRepData* RepData = &SpecialActorList[i];
            if (!RepData) continue;
    
            if (RepData->SpecialActor() != ExitCraftSpawner) continue;
    
            SpawnerIndex = i;
            SpawnerRepData = RepData;
            break;
        }
        
	    FSpecialActorRepData* VanRepData = (FSpecialActorRepData*)FMemory_Realloc(0,FSpecialActorRepData::GetStructSize(),0);
	    memcpy(VanRepData,SpawnerRepData,FSpecialActorRepData::GetStructSize());
    
	    VanRepData->MostRecentArrayReplicationKey = -1;
	    VanRepData->ReplicationID = -1;
	    VanRepData->ReplicationKey = -1;
    
	    VanRepData->SpecialActorID() = van->CraftSpecialActorID();
	    VanRepData->CategoryTag() = heist_mutator->ExitCraftTag();
	    VanRepData->SpecialActor() = van;
    
	    memcpy(&VanRepData->MainIconBrush(), &spawn_info->CraftMinimapIconBrush(),FSlateBrush::GetStructSize());
	    VanRepData->MinimapScale() = spawn_info->CraftMinimapIconScale();
    
	    memcpy(&VanRepData->CompassIconBrush(),&spawn_info->CraftCompassIconBrush(),FSlateBrush::GetStructSize());
	    VanRepData->CompassScale() = spawn_info->CraftCompassIconScale();
    
	    VanRepData->AddedToClientTime() = reinterpret_cast<UWorld*>(GWorld)->TimeSeconds();
    
	    VanRepData->LastLocationReplicationTime() = 0.0f;
	    VanRepData->PrevLocationReplicatedTime() = 0.0f;
    
	    VanRepData->LastYawReplicationTime() = 0.0f;
	    VanRepData->PrevYawReplicatedTime() = 0.0f;
    
	    VanRepData->LastRepLocation() = FVector{0,0,0};
	    VanRepData->PrevRepLocation() = FVector{0,0,0};
	    VanRepData->LerpStartLocation() = start_location;
    
	    VanRepData->bActorIsRelevant() = true;
    
	    SpecialActorList.Add(VanRepData);
    
	    FSpecialActorRepData* RefoundVanData = nullptr;
	    for (int i = 0; i < SpecialActorList.Num(); ++i)
	    {
	    	FSpecialActorRepData* RepData = &SpecialActorList[i];
    
	    	if (RepData->SpecialActor() == van)
	    	{
	    		RefoundVanData = RepData;
	    		break;
	    	}
	    }

        actorRepList->MarkItemDirty(*RefoundVanData);
        
	    memcpy(VanRepData,RefoundVanData,FSpecialActorRepData::GetStructSize());
    
	    heist_mutator->OnSpecialActorAdded(*SpawnerRepData);
        heist_mutator->OnSpecialActorRemoved(*SpawnerRepData);
    
        static void* special_actor_list = FindPropertyByName(FSpecialActorRepDataArray::GetClass(), L"SpecialActorList");
            
	    SpecialActorList.Remove(SpawnerIndex, special_actor_list);
        
	    actorRepList->MarkArrayDirty();
    
	    FMemory_Free(VanRepData);
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

            // Leave channel
            gameserver_client::Client->SendLeave();
            Sleep(5000);

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
            
            // Quit
            RequestExitWithStatus(true, 0);
        }

        // Clear temporary arrays
        team_component::TeamsMap.clear();
        team_component::TeamsList.clear();
        team_component::MainTeamsList.clear();
        player_component::EncryptionTokenList.clear();
        player_component::Profiles.clear();
        player_component::Players.clear();
        player_component::DamageDealtMap.clear();
        player_component::PlayerStatsMap.clear();
        player_component::OwnedEmotes.clear();
        player_component::global_player_id = 0;
        player_component::starting_players = 0;
        player_component::PendingVariantsData.clear();
        pickup_component::GlobalPickupStream.clear();
        anticheat_component::IllegalSniperActionsCount.clear();
        anticheat_component::Initialize();
        
        AFortGameModeAthena* AuthorityGameMode = SpawnDefaultActor<AFortGameModeAthena>(*GWorld, FindObject(L"/Game/Athena/Server/Athena_GameMode.Athena_GameMode_C", true));

        if (AuthorityGameMode == nullptr)
        {
            AuthorityGameMode = SpawnDefaultActor<AFortGameModeAthena>(*GWorld, FindObject(L"/Game/Athena/Athena_GameMode.Athena_GameMode_C", true));
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