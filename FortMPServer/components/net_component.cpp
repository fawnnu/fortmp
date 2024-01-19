#include "include.hpp"
#include "net_component.hpp"

#include "challenges_component.hpp"
#include "director_client.hpp"
#include "era_mutators_component.hpp"
#include "inventory_component.hpp"
#include "log_component.hpp"
#include "player_component.hpp"
#include "replication_component.hpp"
#include "creative_component.hpp"
#include "qos_component.hpp"

net_component::net_component()
{
    ReplicationBlacklistPerConnection = {};
    
    if (uintptr_t set_world_address = memory_manager::find_pattern("FF 90 ? ? ? ? ? 89 ? 9F 49 8D ? 28 ? 89 ? A7 4C 8D 4D 9F 48 8B 03 4C 8D 45 C7 48 8B CB FF 90 ? ? ? ?"))
    {
        INIT_POINTER(SetWorld, FindObject(L"/Script/OnlineSubsystemUtils.Default__IpNetDriver")->VTable[*reinterpret_cast<int*>(set_world_address + 2) / 8]);
    }

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(net_component_hooks::NetConnection__Close, memory_manager::find_function_by_string(L"UNetConnection::Close: %s, Channels: %i, Time: %s"), net_component_hooks::NetConnection__Close_Hook);
    REGISTER_FUNCTION_DETOUR(net_component_hooks::TickFlush, memory_manager::find_function_by_string(L"STAT_NetTickFlush"), net_component_hooks::TickFlush_Hook);
    REGISTER_FUNCTION_DETOUR(net_component_hooks::TickFlush, memory_manager::find_function_by_string(L"UDemoNetDriver::TickFlush: ReplayStreamer ERROR: %s", {0xe8, 0x100, 0x100, 0x100, 0x0}, 1), net_component_hooks::TickFlush_Hook);
    REGISTER_FUNCTION_DETOUR(net_component_hooks::GetBool, memory_manager::find_function_by_string(L"UseAudioThread", {0xe8}, 1, false), net_component_hooks::GetBool_Hook);

    // Only individual CLs
    if (GetCL() == 4363240)
    {
        REGISTER_FUNCTION_DETOUR(net_component_hooks::ReplicateProperties, memory_manager::find_function_by_string(L"FRepLayout::ReplicatePropertiesUsingChangelistState: Entire history sent for: %s"), net_component_hooks::ReplicateProperties_Hook);
    }
    
    // Only legacy replication builds
    if (UFortReplicationGraph::StaticClass() == nullptr)
    {
        REGISTER_CALL_DETOUR_WITH_ADDRESS(memory_manager::find_pattern("E8 ? ? ? ? 48 8B 8E ? ? ? ? 48 85 C9 74 0D 48 3B CF 74 08 48 8B D3"), FlushActorDormancy);
    }
    
    DetourTransactionCommit();
}

void net_component::pre_init()
{
    std::vector<async_wrapper*> async_initializers;

    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(CreateNamedNetDriver, memory_manager::find_function_by_string(L"PlayReplay: failed to create demo net d", { 0xe8, 0x100, 0x100, 0x100 , 0x0 }, 1));
    }));async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(FindNamedNetDriver, memory_manager::find_function_by_string(L"PlayReplay: failed to create demo net d",{0xe8, 0x100, 0x100, 0x100 , 0x0}, 1, false));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(InitListen, memory_manager::find_function_by_string(L"Failed to init net driver ListenURL: %s: %s"));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(SetWorld, memory_manager::find_function_by_string(L"Demo playback failed: %s", { 0xE8 }, 1));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(FlushDormancy, memory_manager::find_function_by_string(L"FlushDormancy: %s. Connection: %s"));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(NetConnection__Close, memory_manager::find_function_by_string(L"UNetConnection::Close: %s, Channels: %i, Time: %s"));
    }));

    async_wrapper::wait_for_wrappers(async_initializers);
}

bool net_component::listen(FURL in_url)
{
    if ((*GWorld)->NetDriver())
    {
        //A netdriver has been set already, therefore we shouldn't attempt listening again!
        return false;
    }
    
    const FName NAME_GameNetDriver(L"GameNetDriver");
    
    if (CreateNamedNetDriver(*GEngine, *GWorld, NAME_GameNetDriver, NAME_GameNetDriver))
    {
        if (UNetDriver* net_driver = FindNamedNetDriver(*GEngine, *GWorld, NAME_GameNetDriver))
        {
            // set NetDriver in World
            (*GWorld)->NetDriver() = net_driver;

            // set World in NetDriver
            SetWorld(net_driver, *GWorld);
            
            auto LevelCollections = TArrayManager<FLevelCollection>(&(*GWorld)->LevelCollections(), FLevelCollection::GetStructSize());

            // set netdriver in levelcollections, RPCs will now work
            LevelCollections[0].NetDriver = net_driver;
            LevelCollections[1].NetDriver = net_driver;

            void* in_notify = reinterpret_cast<void*>(__int64(*GWorld) + sizeof(UObject));

            // Error caused by InitListen (if any)
            FString in_error;

            if (!InitListen(net_driver, in_notify, in_url, false, in_error))
            {
                return false;
            }

            if ((net_driver->MaxInternetClientRate() < net_driver->MaxClientRate()) && (net_driver->MaxInternetClientRate() > 2500))
            {
                net_driver->MaxClientRate() = net_driver->MaxInternetClientRate();
            }

#ifdef _RELEASE
            // Notify Era services of the port
            if (gameserver_client::Client)
            {
                gameserver_client::Client->GameServer_Status.port = in_url.Port;
                gameserver_client::Client->SendUpdate();
            }
#endif
            
            return true;
        }
    }

    return false;
}

void net_component::FlushActorDormancy(UNetDriver* net_driver, AActor* Actor)
{
    if (Actor == nullptr)
    {
        return;
    }

    for (int i = 0; i < net_driver->ClientConnections().Num(); i++)
    {
        UNetConnection* NetConnection = net_driver->ClientConnections()[i];

        if (NetConnection && FlushDormancy)
        {
            FlushDormancy(NetConnection, Actor);
        }
    }
}

void net_component::BlockPropertyReplicationPerConnection(UNetConnection* Connection, UObject* Class, std::wstring PropertyName)
{
    if (Connection && Class && PropertyName.length() > 0)
    {
        ReplicationBlacklistPerConnection[Connection].push_back(std::make_pair(Class, FindPropertyByName(Class, PropertyName.c_str())));

        // Log
        spdlog::info(L"Blocked property '{}' [{}] from replicating in '{}'", PropertyName.c_str(), Class->GetName().c_str(), Connection->GetName().c_str());
    }
}

void net_component_hooks::TickFlush_Hook(UNetDriver* NetDriver, float DeltaSeconds)
{
    while (!game_thread_queue.empty())
    {
        // Execute code on game thread
        game_thread_queue.front()();
        game_thread_queue.pop_front();
    }

    while (!live_events.empty())
    {
        // Execute live event
        era_mutators_component::Execute(std::string(live_events.front()), nullptr);
        
        live_events.pop_front();
    }

    while (!event_replies.empty())
    {
        reply_info* reply = event_replies.front();

        if (reply)
        {
            if (reply->Callback)
            {
                reinterpret_cast<void (*)(std::string, nlohmann::json)>(reply->Callback)(reply->Channel, reply->Payload);
            }
        }

        event_replies.pop_front();
        
        delete reply;
    }

    while (!challenges_progression_queue.empty())
    {
        progression_entry* challenges_progression = challenges_progression_queue.front();

        if (challenges_progression)
        {
            std::string challenge = challenges_progression->challenge;
            std::string objective = challenges_progression->objective;
            int value = challenges_progression->value;
            std::string AccountID = challenges_progression->player;
        
            TO_WSTRING(AccountID, wAccountID);

            if (AFortPlayerController* PlayerController = player_component::get_player_by_account_id(wAccountID))
            {
                if (log_component::enable_debug_logs)
                {
                    if (!memory_manager::is_bad_read_pointer(PlayerController->MyPlayerInfo()) && PlayerController->MyPlayerInfo())
                    {
                        TO_WSTRING(objective, wObjective);
                        TO_WSTRING(challenge, wChallenge);
                
                        spdlog::info(L"Player '{}' has progressed challenge '{}' [OBJECTIVE: '{}', VALUE: {}]", PlayerController->MyPlayerInfo()->PlayerName().c_str(), wChallenge.c_str(), wObjective.c_str(), value);
                    }
                }

                // Working live progression
                if (challenge.find("Quest:") != std::string::npos)
                {
                    challenge = challenge.substr(challenge.find(":") + 1);

                    TO_WSTRING(objective, wObjective);
                    TO_WSTRING(challenge, wChallenge);

                    if (UFortQuestItemDefinition* QuestItemDefinition = FindObject<UFortQuestItemDefinition>(wChallenge.c_str(), false, true))
                    {
                        TArrayManager UpdatedObjectiveStats = TArrayManager<FFortUpdatedObjectiveStat>(&PlayerController->UpdatedObjectiveStats(), FFortUpdatedObjectiveStat::GetStructSize());

                        bool bQuestAlreadyExists = false;
                        for (FFortUpdatedObjectiveStat& ObjectiveStat : UpdatedObjectiveStats)
                        {
                            if (ObjectiveStat.Quest() == QuestItemDefinition &&
                                ObjectiveStat.BackendName() == wObjective.c_str())
                            {
                                bQuestAlreadyExists = true;
                                ObjectiveStat.StatValue() = value;
                            
                                break;
                            }
                        }

                        if (bQuestAlreadyExists == false)
                        {
                            FFortUpdatedObjectiveStat* NewObjectiveStat = reinterpret_cast<FFortUpdatedObjectiveStat*>(alloca(FFortUpdatedObjectiveStat::GetStructSize()));
                            ZeroMemory(NewObjectiveStat, FFortUpdatedObjectiveStat::GetStructSize());
                            NewObjectiveStat->Quest() = QuestItemDefinition;
                            NewObjectiveStat->BackendName() = wObjective.c_str();
                            NewObjectiveStat->StatValue() = value;

                            // Send live progression to client
                            UpdatedObjectiveStats.Add(NewObjectiveStat);
                        }
                    }
                }
            }
        }
        else
        {
            spdlog::error(L"net_component_hooks::TickFlush_Hook caught an invalid challenge progression object. Ignoring...");
        }
        
        challenges_progression_queue.pop_front();

        if (challenges_progression)
        {
            delete challenges_progression;
        }
    }

    while (!kick_events_queue.empty())
    {
        auto kick_event = kick_events_queue.front();
        
        if (kick_event.contains("account_id"))
        {
            std::string sAccountID = kick_event["account_id"];

            if (sAccountID.size())
            {
                std::wstring wAccountID = std::wstring(sAccountID.begin(), sAccountID.end());

                if (wAccountID.size())
                {
                    if (auto player = player_component::get_player_by_account_id(wAccountID))
                    {
                        if (!memory_manager::is_bad_read_pointer(player) && player)
                        {
                            if (!memory_manager::is_bad_read_pointer(player->MyPlayerInfo()) && player->MyPlayerInfo())
                            {
                                spdlog::info(L"Kicked '{}' [account_id: {}] [KICK EVENT]", player->MyPlayerInfo()->PlayerName().c_str(), wAccountID.c_str());
                            }
                        
                            player->ClientReturnToMainMenu(TEXT(""));
                                
                            break;
                        }
                    }
                }
            }
        } 
        
        kick_events_queue.pop_front();
    }

    // Acknowledge pending heartbeats
#ifdef _RELEASE
    gameserver_client::Client->LocalNonAckHeartbeats = 0;

    if ((*GWorld)->AuthorityGameMode())
    {
        if ((*GWorld)->AuthorityGameMode()->AlivePlayers().Num() > 0)
        {
            gameserver_client::Client->LocalNonAckHeartbeatsForPlayers = 0;
        }
    }
#endif
    
    if (NetDriver->ClientConnections().Num() > 0 && !NetDriver->ClientConnections()[0]->InternalAck())
    {
        // Update all clients
        replication_component::ServerReplicateActors(NetDriver, DeltaSeconds);
    }

    return TickFlush(NetDriver, DeltaSeconds);
}

void net_component_hooks::GetBool_Hook(void* Self, const wchar_t* Section, const wchar_t* Key, bool* Value, void* File)
{
    if (wcsstr(Key, TEXT("bDDoSDetection")) ||
        wcsstr(Key, TEXT("bDDoSAnalytics")))
    {
        if (Value)
        {
            *Value = true;

            return;
        }
    }
    
    return GetBool(Self, Section, Key, Value, File);
}

void net_component_hooks::NetConnection__Close_Hook(UNetConnection* Self)
{
    if (Self)
    {
        if (!memory_manager::is_bad_read_pointer(Self->PlayerController()) && Self->PlayerController())
        {
            AFortPlayerController* FortPlayerController = Self->PlayerController();

            // Get account ID
            std::string account_id = player_component::get_account_id(FortPlayerController);

            if (account_id.size() > 0)
            {
                // Send summary, on creative
                if (creative_component::is_creative_enabled())
                {
                    challenges_component::SendSummary(FortPlayerController, 2);
                }
                
                // Mark as disconnecting
                static_cast<AFortPlayerControllerAthena*>(FortPlayerController)->EraPlayerInfo().bIsDisconneting = true;
                
                std::map<std::string, std::string> parameters{
                };
                gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "phx_leave", parameters);

                /*
                 * If we're in Warmup, notify director of an available player
                 */
                if (*GWorld && (*GWorld)->GameState())
                {
                    static UEnum* GamePhaseEnum = FindObject<UEnum>(TEXT("/Script/FortniteGame.EAthenaGamePhase"));
                    uint8_t Warmup = GamePhaseEnum->GetEnumIndex(TEXT("Warmup"));
                    if ((*GWorld)->GameState()->GamePhase() == Warmup || creative_component::is_creative_enabled())
                    {
                        if (director_client::client)
                        {
                            director_client::client->Send(eudp_packet({
                                {"type", "player_left"},
                                {"payload", {
                                    {"account_id", account_id}
                                }}
                            }));
                        }
                    }
                }
                
                // Log
                if (!memory_manager::is_bad_read_pointer(FortPlayerController->MyPlayerInfo()) && FortPlayerController->MyPlayerInfo())
                {
                    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> name_converter;
                    std::wstring wPlayer_name = FortPlayerController->MyPlayerInfo()->PlayerName().c_str();
                    std::string player_name = name_converter.to_bytes(wPlayer_name);
                    
                    spdlog::info("Player '{}' left its channel [ACCOUNT_ID: {}]", player_name.c_str(), account_id.c_str());

                    // Drop items
                    inventory_component::clear_primary_inventory(FortPlayerController, true);   
                }
            }
        }
    }
    
    return NetConnection__Close(Self);
}

bool net_component_hooks::ReplicateProperties_Hook(void* RepLayout, void* __restrict RepState, void* __restrict RepChangelistState, const uint8* __restrict Data, UObject* ObjectClass, UActorChannel* OwningChannel, void* Writer, const FReplicationFlags& RepFlags)
{
    // Iterate through the blacklisted properties
    if (net_component::ReplicationBlacklistPerConnection.contains(OwningChannel->Connection()))
    {
        for (std::pair<UObject*, void*> val : net_component::ReplicationBlacklistPerConnection[OwningChannel->Connection()])
        {
            UObject* BlacklistedClass = val.first;
            void* BlacklistedProperty = val.second;

            // Check if we're replicating the blacklisted class
            if (ObjectClass == BlacklistedClass)
            {
                // Look through the 'Parents' cmd array in RepLayout for this specific property
                for (auto& Parent : *reinterpret_cast<TArray<FRepParentCmd>*>(reinterpret_cast<uintptr_t>(RepLayout) + 0x10))
                {
                    // Found property
                    if (Parent.Property == BlacklistedProperty)
                    {
                        // Save the old condition
                        ELifetimeCondition OldCondition = Parent.Condition;

                        // Change the condition before calling ReplicateProperties
                        if (RepFlags.bNetOwner)
                        {
                            Parent.Condition = COND_SkipOwner;
                        }
                        else
                        {
                            Parent.Condition = COND_OwnerOnly;
                        }
                        
                        bool ReturnValue = ReplicateProperties(RepLayout, RepState, RepChangelistState, Data, ObjectClass, OwningChannel, Writer, RepFlags);

                        // Go back to the old condition
                        Parent.Condition = OldCondition;
                        
                        return ReturnValue;
                    }
                }
            }
        }
    }
    
    return ReplicateProperties(RepLayout, RepState, RepChangelistState, Data, ObjectClass, OwningChannel, Writer, RepFlags);   
}