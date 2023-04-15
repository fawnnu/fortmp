#include "include.hpp"
#include "loader_component.hpp"

#include "function_component.hpp"
#include "looting_component.hpp"
#include "net_component.hpp"
#include "pickup_component.hpp"
#include "team_component.hpp"
#include "config_manager.hpp"
#include "player_component.hpp"
#include "playlist_component.hpp"

#include "../gui/renderer.hpp"
#include "../utils/webhook_manager.hpp"

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
}

void loader_component::start_match(UFortPlaylistAthena* in_playlist)
{
    constexpr int listen_port = 7777;

    // Get UI Ready
    renderer::LoadAssets();
    
    // Listen for connections
    const FURL in_url((*GWorld)->GetName().c_str(), listen_port);

    //make sure we weren't listening already
    if (net_component::listen(in_url))
    {
        if (in_playlist == nullptr)
        {
            // See if server.json is overriding the playlist
            auto config = config_manager::get_server_config();
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
            
            if (in_playlist == nullptr)
            {
                // Use default playlist if none specified
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

        // initialize llamas
        initialize_llamas();

        // initialize BGAs
        initialize_bga();

        // Disable duplicate building foundations
        handle_duplicated_foundations();

        // Initialize era_teams
        team_component::InitializeTeams();

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
            (*GWorld)->AuthorityGameMode()->EndGameKickPlayersDelay() = 15.f;   
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

        spdlog::info(L"Match started in world '{}' and listening for connections",
            (*GWorld)->GetName().c_str());
    }
}

bool loader_component::initialize_playlist(UFortPlaylistAthena* in_playlist)
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

        for (const auto& [ item_definition, count ] : loot_to_spawn)
        {
            // Spawn pickup
            pickup_component::spawn_pickup(item_definition, floor_loot_actor->K2_GetActorLocation(), count, looting_component::get_default_loaded_ammo(item_definition));
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

    AFortAthenaMapInfo* MapInfo = GameState->MapInfo() ? GameState->MapInfo() : SpawnDefaultActor<AFortAthenaMapInfo>(*GWorld, FindObject(L"/Game/Athena/Balance/MapInfos/DefaultMapInfo.DefaultMapInfo_C"));
    
    if (MapInfo)
    {
        // Create a new FortSupplyDropInfo
        UFortSupplyDropInfo* NewSupplyDropInfo = static_cast<UFortSupplyDropInfo*>(UGameplayStatics::SpawnObject(FindObject(L"/Script/FortniteGame.FortSupplyDropInfo"), *GWorld));

        if (NewSupplyDropInfo == nullptr)
        {
            spdlog::error(L"loader_component::initialize_llamas: Failed, invalid NewSupplyDropInfo");
            
            return;
        }
        
        // Set the supply drop info's class
        if (STATIC_HASPROPERTY(MapInfo, LlamaClass))
        {
            NewSupplyDropInfo->SupplyDropClass() = MapInfo->LlamaClass();

            // Create a new FFortSupplyDropGamePhaseData (Allocate the struct on the heap to avoid issues with garbage collection)
            FFortSupplyDropGamePhaseData* NewGamePhaseData = static_cast<FFortSupplyDropGamePhaseData*>(malloc(FFortSupplyDropGamePhaseData::GetStructSize()));

            // Initialize our struct's memory
            ZeroMemory(NewGamePhaseData, FFortSupplyDropGamePhaseData::GetStructSize());

            // Add it to the supply drop info
            auto GamePhaseData = TArrayManager<FFortSupplyDropGamePhaseData>(&NewSupplyDropInfo->GamePhaseData(), FFortSupplyDropGamePhaseData::GetStructSize());

            GamePhaseData.Add(NewGamePhaseData);

            // Now create a sub phase data
            FFortSupplyDropSubPhaseData* NewSubPhaseData = static_cast<FFortSupplyDropSubPhaseData*>(malloc(FFortSupplyDropSubPhaseData::GetStructSize()));

            // Initialize our struct's memory
            ZeroMemory(NewSubPhaseData, FFortSupplyDropSubPhaseData::GetStructSize());

            auto SubPhaseData = TArrayManager<FFortSupplyDropSubPhaseData>(&GamePhaseData[0].SubPhaseData(), FFortSupplyDropSubPhaseData::GetStructSize());

            // Add the sub phase data to our GamePhaseData
            SubPhaseData.Add(NewSubPhaseData);

            // Now set SupplyDropMinPlacementHeight and SupplyDropMaxPlacementHeight, copy them from the default supply drop info
            if (UFortSupplyDropInfo* DefaultSupplyDrop = FindObject<UFortSupplyDropInfo>(L"/Game/Athena/Balance/MapInfos/DefaultSupplyDrop.DefaultSupplyDrop", true))
            {
                auto DefaultGamePhaseData = TArrayManager<FFortSupplyDropGamePhaseData>(&DefaultSupplyDrop->GamePhaseData(), FFortSupplyDropGamePhaseData::GetStructSize());
                
                GamePhaseData[0].SupplyDropMinPlacementHeight() = DefaultGamePhaseData[0].SupplyDropMinPlacementHeight();
                GamePhaseData[0].SupplyDropMaxPlacementHeight() = DefaultGamePhaseData[0].SupplyDropMaxPlacementHeight();
            }

            // Spawn the llamas
            for (int i = 0; i < UKismetMathLibrary::RandomIntegerInRange(MapInfo->LlamaQuantityMin().GetValueAtLevel(0, nullptr), MapInfo->LlamaQuantityMax().GetValueAtLevel(0, nullptr)); i++)
            {
                // Call the function to spawn the llamas
                if (SpawnSupplyDrop != nullptr)
                {
                    FRotator NewRotation{};

                    // New llama
                    AFortAthenaSupplyDrop* NewSupplyDrop = SpawnSupplyDrop(MapInfo, nullptr, 0, 0, NewSupplyDropInfo, &NewRotation);

                    // Log
                    spdlog::info(L"Spawned Llama at {} {} {}", NewSupplyDrop->K2_GetActorLocation().X, NewSupplyDrop->K2_GetActorLocation().Y, NewSupplyDrop->K2_GetActorLocation().Z);
                }
                else
                {
                    spdlog::error(L"Failed to spawn llama: SpawnSupplyDrop was not found.");
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
            
            // Quit
            RequestExitWithStatus(true, 0);
        }

        // Clear temporary arrays
        team_component::TeamsMap.clear();
        team_component::TeamsList.clear();
        player_component::EncryptionTokenList.clear();
        player_component::Profiles.clear();
        player_component::Players.clear();
        player_component::global_player_id = 0;
        
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
