#include "include.hpp"
#include "player_component.hpp"

#include "ability_component.hpp"
#include "anticheat_component.hpp"
#include "challenges_component.hpp"
#include "config_manager.hpp"
#include "creative_component.hpp"
#include "function_component.hpp"
#include "inventory_component.hpp"
#include "loader_component.hpp"
#include "moderation_component.hpp"
#include "net_component.hpp"
#include "team_component.hpp"

player_component::player_component()
{
    INIT_POINTER(HealthProperty, memory_manager::find_pattern("48 8B 15 ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? 48 8B CE E8 ? ? ? ? 48 85 C0 74 11 41 0F 28 D0 48 8D 54 24 ? 48 8B C8 E8 ? ? ? ? 48 8B 4C 24 ? 48 85 C9 74 05 E8 ? ? ? ? 48 8D 4E 30 E8 ? ? ? ? 44 0F 28 C0 48 8D 4E 50 F3 45 0F 5C C1 F3 44 0F 5F C7 E8 ? ? ? ? 48 8D 4E 30 44 0F 28 C8 E8 ? ? ? ?", true, 3));
    if (!HealthProperty)
    {
        INIT_POINTER(HealthProperty, memory_manager::find_pattern("48 8B 15 ? ? ? ? 48 8D 4D EF E8 ? ? ? ? 48 8D 55 EF 48 8B CE E8 ? ? ? ? 84 C0 74 05 40 B7 01 EB 03 40 32 FF 41 F6 C6 01 74 12 48 8B 4D EF 41 83 E6 FE 48 85 C9 74 05 E8 ? ? ? ?", true, 3));
    }
    
    // Initialize serverchoosepart
    if (const uintptr_t index = memory_manager::find_function_by_string(L"ServerChoosePart_Validate", {0xff, 0x90}, 2, false, false))
    {
        INIT_POINTER(ServerChoosePart_Implementation, FindObject(L"/Script/FortniteGame.Default__FortPlayerPawn")->VTable[index/8]);
    }

    INIT_POINTER(UpdateZiplineState, memory_manager::find_function_by_string(L"AFortPlayerPawn::OnRep_ZiplineState"));

    if (UFunction* ClientAddMarker = FindObject<UFunction>(L"/Script/FortniteGame.AthenaMarkerComponent.ClientAddMarker"))
    {
        uintptr_t WorldMarkerDataCtr_Call = memory_manager::scan_for(reinterpret_cast<uintptr_t>(ClientAddMarker->get_native_function()), {0xE8, WILDCARD, WILDCARD, WILDCARD, 0xFF});
        uintptr_t WorldMarkerData_Addr = WorldMarkerDataCtr_Call + 5 + *reinterpret_cast<int*>(WorldMarkerDataCtr_Call + 1);
        
        INIT_POINTER(WorldMarkerDataCtr, WorldMarkerData_Addr);
    }

    // BREAK DOWN THE REGISTER_CALL_HOOK MACRO AND USE STRIPPED FUNCTIONS GETTERS INSTEAD
    { UFunction* function_to_hook = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.SpawnToyInstance") ?
        FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.SpawnToyInstance") :
        FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.SpawnToyInstance", false, true);
        if (function_to_hook != nullptr)
        {
            const auto all_calls_in_function = memory_manager::get_all_stripped_calls_in_function_wiktor(reinterpret_cast<uintptr_t>(function_to_hook->get_native_function()));
            memory_manager::call_hook(all_calls_in_function[all_calls_in_function.size() - 1], reinterpret_cast<uintptr_t>(player_component_hooks::SpawnToyInstance_Hook));
        } else
        {
            spdlog::error(L"Failed to hook '{}', invalid function object provided", L"/Script/FortniteGame.FortPlayerController.SpawnToyInstance");
        } };
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerAcknowledgePossession, L"/Script/Engine.PlayerController.ServerAcknowledgePossession", player_component_hooks::ServerAcknowledgePossession_Hook);
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerChoosePart, L"/Script/FortniteGame.FortPlayerPawn.ServerChoosePart", player_component_hooks::ServerChoosePart_Hook);
    
    REGISTER_FUNCTION_DETOUR(player_component_hooks::PostLogin, memory_manager::find_function_by_string(L"FortGameModeAthena: Player [%s] PostLogin"), player_component_hooks::PostLogin_Hook);
    
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerPlayEmoteItem, L"/Script/FortniteGame.FortPlayerController.ServerPlayEmoteItem", player_component_hooks::ServerPlayEmoteItem_Hook);
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerPlaySprayItem, L"/Script/FortniteGame.FortPlayerController.ServerPlaySprayItem", player_component_hooks::ServerPlaySprayItem_Hook);
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::MovingEmoteStopped, L"/Script/FortniteGame.FortPawn.MovingEmoteStopped", player_component_hooks::MovingEmoteStopped_Hook);
    
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerSendZiplineState, L"/Script/FortniteGame.FortPlayerPawn.ServerSendZiplineState", player_component_hooks::ServerSendZiplineState_Hook);
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerAddMapMarker, L"/Script/FortniteGame.AthenaMarkerComponent.ServerAddMapMarker", player_component_hooks::ServerAddMapMarker_Hook);
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerReturnToMainMenu, L"/Script/FortniteGame.FortPlayerController.ServerReturnToMainMenu", player_component_hooks::ServerReturnToMainMenu_Hook);
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerReadyToStartMatch, L"/Script/FortniteGame.FortPlayerController.ServerReadyToStartMatch", player_component_hooks::ServerReadyToStartMatch_Hook);
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerPlaySquadQuickChatMessage, L"/Script/FortniteGame.FortPlayerControllerAthena.ServerPlaySquadQuickChatMessage", player_component_hooks::ServerPlaySquadQuickChatMessage_Hook);

    REGISTER_FUNCTION_DETOUR(player_component_hooks::OnTargetDataReadyCallback, memory_manager::find_function_by_string(L"STAT_FortGA_Ranged_OnTargetDataReadyCB"), player_component_hooks::OnTargetDataReadyCallback_Hook);
    
    if (GetEngineVersionEasy() < 420)
    {
        // Hooking this on later versions causes issues
        REGISTER_FUNCTION_DETOUR(player_component_hooks::GetHeroDefinition, memory_manager::find_function_by_string(L"Trying to apply homebase effects on player setup but hero or herotype is not set.", { 0xE8, 0x100, 0x100, 0x100, 0xff }, 1), player_component_hooks::GetHeroDefinition_Hook);
    }
    
    DetourTransactionCommit();

    // Finish spawning a player
    REGISTER_PROCESS_EVENT_DETOUR(L"/Script/Engine.GameModeBase.K2_OnRestartPlayer", player_component_hooks::K2_OnRestartPlayer_Hook);
    REGISTER_PROCESS_EVENT_DETOUR(L"/Script/FortniteGame.FortPawn.OnDamageServer", player_component_hooks::OnDamageServer_Hook);

    REGISTER_CUSTOM_UFUNCTION(OnPawnLandedImplementation, AFortPawn::StaticClass())
    REGISTER_CUSTOM_UFUNCTION(GatherDistanceTraveled, AFortPlayerControllerAthena::StaticClass())
}

AActor* player_component_hooks::SpawnToyInstance_Hook(AFortPlayerController* PlayerController, UObject* ToyClass,
    FTransform SpawnPosition)
{
    if (PlayerController && !memory_manager::is_bad_read_pointer(PlayerController))
    {
        AB_PlayerSpawnedBall_C* toy_instance = static_cast<AB_PlayerSpawnedBall_C*>(UGameplayStatics::SpawnActor(ToyClass, SpawnPosition, 1, PlayerController->PlayerState()));
    
        return toy_instance;   
    }
    
    return nullptr;
}

void player_component_hooks::MovingEmoteStopped_Hook(AFortPawn* FortPawn, FFrame& StackFrame)
{
    MovingEmoteStopped(FortPawn, StackFrame);
    
    if (!FortPawn)
    {
        return;
    }
    
    AFortPlayerController* Controller = FortPawn->Controller();
    if (!Controller)
    {
        return;
    }
    
    UKismetSystemLibrary::SetBoolPropertyByName(FortPawn, L"bMovingEmote", false);
    UKismetSystemLibrary::SetBoolPropertyByName(FortPawn, L"bMovingEmoteForwardOnly", false);

    if (STATIC_HASPROPERTY(FortPawn, EmoteWalkSpeed))
    {
        FortPawn->EmoteWalkSpeed() = 0.0f;
    }
}

void player_component::OnPawnLandedImplementation(AFortPlayerPawn* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self == nullptr || Self->Controller() == nullptr)
    {
        return;
    }

    if (STATIC_HASPROPERTY(Self, bIsSkydivingFromBus))
    {
        if (Self->bIsSkydivingFromBus())
        {
            if (Self->CurrentWeapon() == nullptr)
            {
                Self->Controller()->ServerExecuteInventoryItem(inventory_component::get_pickaxe(Self->Controller())->ItemEntry().ItemGuid());
            }

            // Progress land challenges
            challenges_component::SendLandingEvent(Self->Controller(), Self);
        }
    }
}

void player_component::GatherDistanceTraveled(AFortPlayerControllerAthena* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (!memory_manager::is_bad_read_pointer(Self) && Self)
    {
        if (!memory_manager::is_bad_read_pointer(Self->Pawn()) && Self->Pawn())
        {
            if (PlayerStatsMap[Self].LastPos.IsValid())
            {
                const FVector CurrentLocation = Self->Pawn()->K2_GetActorLocation();
                const FVector LastPos = PlayerStatsMap[Self].LastPos;

                const float Distance = sqrtf((CurrentLocation.X - LastPos.X) * (CurrentLocation.X - LastPos.X) +
                                            (CurrentLocation.Y - LastPos.Y) * (CurrentLocation.Y - LastPos.Y) +
                                            (CurrentLocation.Z - LastPos.Z) * (CurrentLocation.Z - LastPos.Z));

                PlayerStatsMap[Self].DistanceTraveled += Distance;
            }
            
            PlayerStatsMap[Self].LastPos = Self->Pawn()->K2_GetActorLocation();
        }
        
        // Re-execute the same function in 1 second
        UKismetSystemLibrary::K2_SetTimer(Self, L"GatherDistanceTraveled", 1, false);
    }
}

bool player_component::Login(const FUniqueNetIdRepl& PlayerID, const FString& Options)
{
    // Only let people with a valid encryption token join
#ifdef _RELEASE
    if (UGameplayStatics::HasOption(Options, TEXT("EncryptionToken")))
    {
        FString EncryptionToken = UGameplayStatics::ParseOption(Options, TEXT("EncryptionToken"));

        if (EncryptionToken.IsValid())
        {
            for (int i = 0; i < EncryptionTokenList.size(); i++)
            {
                auto Token = EncryptionTokenList[i];
                 
                if (wcsstr(Token.c_str(), EncryptionToken.c_str()))
                {
                    EncryptionTokenList.erase(EncryptionTokenList.begin() + i);

                    spdlog::info(L"EncryptionToken successfully validated.");
                    
                    return true;
                }
            }

            spdlog::error(L"Client sent '{}' as EncryptionToken, but the server could not validate it. Valid encryption tokens: {}", EncryptionToken.c_str(), EncryptionTokenList.size());
        }
        else
        {
            spdlog::error(L"Invalid EncryptionToken received, kicking...");
        }

        InvalidTokensReceived++;

        if (InvalidTokensReceived >= 10)
        {
            // Mark as pending shutdown
            loader_component::bPendingShutdown = true;

            // Log
            spdlog::warn(L"loader_component::bPendingShutdown changed to true: InvalidTokensReceived exceeds maximum allowed value.");
        }
        
        // Limit for invalid tokens
        if (InvalidTokensReceived >= 150)
        {
            // Leave Era Services
            gameserver_client::Client->SendLeave();

            // Close
            RequestExitWithStatus(true, 0);
        }
    }
    else
    {
        spdlog::error(L"Player tried to join without EncryptionToken, kicking...");
    }
    return false;
#endif

    return true;
}

void player_component::PostLogin(AFortPlayerController* FortPlayerController)
{
    if (FortPlayerController)
    {
        // PHX JOIN the player's channel
        std::string account_id = get_account_id(FortPlayerController);

        if (account_id.size() > 0)
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            std::string team_id;

            for (auto [wteam_id, team_info] : team_component::TeamsMap)
            {
                if (std::find(team_info.TeamMembersControllers.begin(), team_info.TeamMembersControllers.end(), FortPlayerController) != team_info.TeamMembersControllers.end())
                {
                    TO_STRING(wteam_id, steam_id);
                    team_id = steam_id;
                    
                    break;
                }
            }
            
            if (team_id.length() == 0)
            {
                team_id = account_id;
            }
            
            std::map<std::string, std::string> parameters{
                    {"server", gameserver_client::Client->UUID},
                    {"team", team_id}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "phx_join", parameters);

            // Initialize stats
            PlayerStatsMap[FortPlayerController] = FPlayerStats();
            static_cast<AFortPlayerControllerAthena*>(FortPlayerController)->GatherDistanceTraveled();
            
            // Log
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> name_converter;
            std::wstring wPlayer_name = FortPlayerController->MyPlayerInfo()->PlayerName().c_str();
            std::string player_name = name_converter.to_bytes(wPlayer_name);
            
            spdlog::info("Player '{}' joined its channel. [SERVER: {}, TEAM: {}, ACCOUNT_ID: {}]", player_name.c_str(), gameserver_client::Client->UUID.c_str(), team_id.c_str(), account_id.c_str());
        }
    }
}

void player_component::Setup(AFortPlayerController* NewPlayer, const FString& Options)
{
    if (UGameplayStatics::HasOption(Options, TEXT("EncryptionToken")))
    {
        FString Token = UGameplayStatics::ParseOption(Options, TEXT("EncryptionToken"));

        if (Token.IsValid())
        {
            // Encryption token
            std::wstring wToken = Token.c_str();

            // Register this player in the local Players map
            std::wstring wAccountID = wToken.substr(0, wToken.find(L":"));
            Players[wAccountID] = NewPlayer;

            for (auto& [token, data] : Profiles)
            {
                if (token == wToken)
                {
                    if (data.contains("admin"))
                    {
                        if (static_cast<bool>(data["admin"]))
                        {
                            // Give admin permission
                            NewPlayer->CheatManager() = static_cast<UCheatManager*>(UGameplayStatics::SpawnObject(UCheatManager::StaticClass(), NewPlayer));

                            if (data.contains("display_name"))
                            {
                                spdlog::info("Player '{}' joined as an admin [AUTHORIZED BY ERA SERVICES].", std::string(data["display_name"]));
                            }
                        }
                    }

                    if (data.contains("level"))
                    {
                        if (static_cast<int>(data["level"]) > 0)
                        {
                            initialize_player_banner_level(NewPlayer, data["level"]);
                        }
                    }
            
                    if (data.contains("locker"))
                    {
                        nlohmann::json locker = data["locker"];

                        if (locker.contains("emotes"))
                        {
                            nlohmann::json emotes = locker["emotes"];

                            // Emote ownership
                            if (emotes.is_array())
                            {
                                for (auto& elem : emotes.items())
                                {
                                    std::string emote = elem.value();

                                    if (emote.find(":") != std::string::npos)
                                    {
                                        emote = emote.substr(emote.find(":") + 1);
                                        TO_WSTRING(emote, wEmote);

                                        if (UAthenaDanceItemDefinition* EmoteAsset = FindObject<UAthenaDanceItemDefinition>(wEmote.c_str(), false, true))
                                        {
                                            OwnedEmotes[NewPlayer].push_back(EmoteAsset);
                                        }
                                    }
                                }
                            }
                        }

                        for (auto& elem : locker.items())
                        {
                            if (elem.value().is_object())
                            {
                                if (elem.value().contains("asset"))
                                {
                                    nlohmann::json asset = elem.value()["asset"];
                                    
                                    // If it's a valid string
                                    if (asset.is_string() && std::string(asset).find(":") != std::string::npos)
                                    {
                                        // Banners
                                        if (locker.contains("banner") &&
                                            locker["banner"].contains("asset") &&
                                            locker.contains("banner_color") &&
                                            locker["banner_color"].contains("asset"))
                                        {
                                            if (asset == locker["banner"]["asset"])
                                            {
                                                initialize_player_banner_icon(NewPlayer, asset);
                                        
                                                continue;
                                            }
                                            if (asset == locker["banner_color"]["asset"])
                                            {
                                                initialize_player_banner_color(NewPlayer, asset);
                                        
                                                continue;
                                            }
                                        }
                                
                                        std::string value = std::string(asset).substr(std::string(asset).find(":")+1);

                                        UObject* Object = FindObject(std::wstring(value.begin(), value.end()).c_str(), false, true);

                                        // Check if the backend didn't send a literal random item definition
                                        UAthenaGameData* athena_game_data = UFortGameData::get_athena_game_data();

                                        if (athena_game_data)
                                        {
                                            if (athena_game_data->HasProperty("ShuffleTileItemDefinitions"))
                                            {
                                                for (auto shuffle_tile_soft_item : athena_game_data->ShuffleTileItemDefinitions())
                                                {
                                                    if (UFortItemDefinition* ShuffleTileItem = shuffle_tile_soft_item.Get())
                                                    {
                                                        if (Object == ShuffleTileItem)
                                                        {
                                                            Object = nullptr;
                                                        }
                                                    }
                                                } 
                                            }
                                        }
                                        
                                        if (Object)
                                        {
                                            if (Object->IsA(UAthenaCharacterItemDefinition::StaticClass()))
                                            {
                                                static_cast<AFortPlayerControllerAthena*>(NewPlayer)->CustomizationLoadout().Character() = static_cast<UAthenaCharacterItemDefinition*>(Object);
                                            }
                                            else if (Object->IsA(UAthenaBackpackItemDefinition::StaticClass()))
                                            {
                                                static_cast<AFortPlayerControllerAthena*>(NewPlayer)->CustomizationLoadout().Backpack() = static_cast<UAthenaBackpackItemDefinition*>(Object);
                                            }
                                            else if (Object->IsA(UAthenaPickaxeItemDefinition::StaticClass()))
                                            {
                                                static_cast<AFortPlayerControllerAthena*>(NewPlayer)->CustomizationLoadout().Pickaxe() = static_cast<UAthenaPickaxeItemDefinition*>(Object);
                                            }
                                            else if (Object->IsA(UAthenaGliderItemDefinition::StaticClass()))
                                            {
                                                static_cast<AFortPlayerControllerAthena*>(NewPlayer)->CustomizationLoadout().Glider() = static_cast<UAthenaGliderItemDefinition*>(Object);
                                            }
                                            else if (Object->IsA(UAthenaSkyDiveContrailItemDefinition::StaticClass()))
                                            {
                                                static_cast<AFortPlayerControllerAthena*>(NewPlayer)->CustomizationLoadout().SkyDiveContrail() = static_cast<UAthenaSkyDiveContrailItemDefinition*>(Object);
                                            }
                                        }
                                    }
                                }
                                if (elem.value().contains("variants"))
                                {
                                    nlohmann::json variants = elem.value()["variants"];

                                    if (variants.is_array())
                                    {
                                        for (auto& elem_variant : variants.items())
                                        {
                                            if (elem_variant.value().is_object())
                                            {
                                                if (elem_variant.value().contains("active") &&
                                                    elem_variant.value()["active"].is_string() &&
                                                    elem_variant.value().contains("channel") &&
                                                    elem_variant.value()["channel"].is_string() &&
                                                    elem_variant.value().contains("owned") &&
                                                    elem_variant.value()["owned"].is_array())
                                                {
                                                    std::string active = elem_variant.value()["active"];
                                                    TO_WSTRING(active, wActive);
                                                    std::string channel = elem_variant.value()["channel"];
                                                    TO_WSTRING(channel, wChannel);
                                                    nlohmann::json owned = elem_variant.value()["owned"];

                                                    // Convert variant names
                                                    wActive = std::wstring(L"Cosmetics.Variant.Property.") + wActive;
                                                    wChannel = std::wstring(L"Cosmetics.Variant.Channel.") + wChannel;

                                                    FMcpVariantChannelInfo* NewVariantInfo = new FMcpVariantChannelInfo;
                                                    NewVariantInfo->ActiveVariantTag() = FName(wActive.c_str());
                                                    NewVariantInfo->VariantChannelTag() = FName(wChannel.c_str());

                                                    if (owned.is_array())
                                                    {
                                                        for (auto& elem_owned : owned.items())
                                                        {
                                                            if (elem_owned.value().is_string())
                                                            {
                                                                std::string sOwned = elem_owned.value();
                                                                TO_WSTRING(sOwned, wOwned);
                                                                wOwned = std::wstring(L"Cosmetics.Variant.Property.") + wOwned;

                                                                NewVariantInfo->OwnedVariantTags().GameplayTags.Add(FGameplayTag(FName(wOwned.c_str())));
                                                            }
                                                        }

                                                        // Add NewVariantInfo
                                                        if (FindPropertyByName(static_cast<AFortPlayerControllerAthena*>(NewPlayer)->CustomizationLoadout().GetClass(), L"CharacterVariantChannels"))
                                                        {
                                                            TArrayManager<FMcpVariantChannelInfo>(&static_cast<AFortPlayerControllerAthena*>(NewPlayer)->CustomizationLoadout().CharacterVariantChannels(), FMcpVariantChannelInfo::GetStructSize()).Add(NewVariantInfo);
                                                        }

                                                        free(NewVariantInfo);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    break;
                }
            }
        }
    }
}

bool player_component::is_live_spectator(APlayerController* player_controller)
{
    STATIC_HASPROPERTY((*GWorld)->AuthorityGameMode(), LiveSpectatorPlayerControllerClass);   
    STATIC_HASPROPERTY((*GWorld)->AuthorityGameMode(), LiveBroadcastPlayerControllerClass);

    if (bExists_LiveBroadcastPlayerControllerClass && bExists_LiveSpectatorPlayerControllerClass)
    {
        if (player_controller->IsA((*GWorld)->AuthorityGameMode()->LiveBroadcastPlayerControllerClass()) ||
            player_controller->IsA((*GWorld)->AuthorityGameMode()->LiveSpectatorPlayerControllerClass()))
        {
            return true;
        }
    }
    
    return false;
}

void player_component::ApplyBackpack(AFortPlayerControllerAthena* NewPlayer)
{
    if (NewPlayer->Pawn())
    {
        if (FindPropertyByName(FFortAthenaLoadout::GetClass(), TEXT("Backpack")))
        {
            if (NewPlayer->CustomizationLoadout().Backpack())
            {
                if (STATIC_HASPROPERTY(NewPlayer->CustomizationLoadout().Backpack(), CharacterParts))
                {
                    if (NewPlayer->CustomizationLoadout().Backpack()->CharacterParts().Num())
                    {
                        NewPlayer->Pawn()->ServerChoosePart(NewPlayer->CustomizationLoadout().Backpack()->CharacterParts()[0]->CharacterPartType(), NewPlayer->CustomizationLoadout().Backpack()->CharacterParts()[0]);
                    }
                }
            }
        }
    }
}

void player_component::play_emote_item(AFortPlayerController* player_controller, UAthenaDanceItemDefinition* dance_asset, bool b_requires_validation)
{
    if (dance_asset)
    {
        if (player_controller->Pawn() && player_controller->Pawn()->AbilitySystemComponent())
        {
#ifdef _RELEASE
            if (b_requires_validation)
            {
                // Make sure the player owns the asset
                if (std::find(OwnedEmotes[player_controller].begin(), OwnedEmotes[player_controller].end(), dance_asset) == OwnedEmotes[player_controller].end())
                {
                    // Log
                    spdlog::error(L"Received attempt to play non-owned emote item.");
                
                    return;
                }
            }
#endif
            
            UFortGameData* game_data = UFortGameData::get_game_data();
            
            auto new_ability_spec = FGameplayAbilitySpec();
        
            // set the ability in the newly created struct
            if (dance_asset->IsA(UAthenaSprayItemDefinition::StaticClass()))
            {
                new_ability_spec.Ability = CreateDefaultObject<UObject>(game_data->SprayGameplayAbility().Get());
                
                // Notify backend that we're dancing
                challenges_component::SendSprayInteraction(player_controller);
            }
            else if (dance_asset->IsA(UAthenaToyItemDefinition::StaticClass()))
            {
                auto toy_asset = static_cast<UAthenaToyItemDefinition*>(dance_asset);
                new_ability_spec.Ability = CreateDefaultObject<UObject>(toy_asset->ToySpawnAbility().Get());
            }
            else
            {
                new_ability_spec.Ability = CreateDefaultObject<UObject>(game_data->EmoteGameplayAbility().Get());
                
                // Notify backend that we're dancing
                challenges_component::SendDanceInteraction(player_controller);
            }
        
            new_ability_spec.SourceObject = dance_asset; // set the sourceobject, in this case the emote we want to execute
            new_ability_spec.Level = 1; 
            new_ability_spec.InputID = -1;
            new_ability_spec.ReplicationID = -1;
            new_ability_spec.ReplicationKey = -1;
            new_ability_spec.MostRecentArrayReplicationKey = -1;
            new_ability_spec.Handle = rand();

            const auto unk = new DWORD();

            // plays the emote by activating the ability.
            // GiveAbility has an additional DWORD* param, which we will simply create and use ourselves
            // by using abilities for emoting, you won't have to deal with stopping the montage yourself when the pawn moves
            GiveAbilityAndActivateOnce(player_controller->Pawn()->AbilitySystemComponent(), unk, new_ability_spec);
            delete unk;
            
            // Traversal emotes implementation ( CR. wiktorwiktor12 for help)
            STATIC_HASPROPERTY(dance_asset, bMovingEmote);
            STATIC_HASPROPERTY(dance_asset, bMoveForwardOnly);
            STATIC_HASPROPERTY(dance_asset, WalkForwardSpeed);
            STATIC_HASPROPERTY(player_controller->Pawn(), EmoteWalkSpeed);
            STATIC_HASPROPERTY(player_controller->Pawn(), LastEmoteItemDef);
            STATIC_HASPROPERTY(player_controller->Pawn(), LastReplicatedEmoteExecuted);
            if (bExists_bMovingEmote &&
                bExists_bMoveForwardOnly &&
                bExists_WalkForwardSpeed &&
                bExists_EmoteWalkSpeed &&
                bExists_LastEmoteItemDef &&
                bExists_LastReplicatedEmoteExecuted)
            {
                UKismetSystemLibrary::SetBoolPropertyByName(player_controller->Pawn(), L"bMovingEmote", dance_asset->bMovingEmote());
                UKismetSystemLibrary::SetBoolPropertyByName(player_controller->Pawn(), L"bMovingEmoteForwardOnly", dance_asset->bMoveForwardOnly());
                player_controller->Pawn()->EmoteWalkSpeed() = dance_asset->WalkForwardSpeed();

                player_controller->Pawn()->LastEmoteItemDef() = dance_asset;
                player_controller->Pawn()->LastReplicatedEmoteExecuted() = dance_asset;
            }
        }
    }
}

AFortPlayerPawn* player_component::create_player(AFortPlayerController* player_controller, FTransform spawn_transform)
{
    AFortPlayerPawn* player_pawn = (*GWorld)->AuthorityGameMode()->SpawnDefaultPawnAtTransform(player_controller, spawn_transform);

    if (player_pawn)
    {
        player_controller->Possess(player_pawn);
        
        UObject* ability_set = FindObject(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer") ?
            FindObject(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer") : FindObject(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_DefaultPlayer.GAS_DefaultPlayer");
        
        // Apply Athena ability set
        ability_component::apply_ability_set(player_pawn, ability_set);

        // Apply customization loadout
        player_pawn->PlayerState()->ApplyCharacterCustomization(player_pawn);
        
        return player_pawn;
    }

    return nullptr;
}

std::string player_component::get_account_id(AFortPlayerController* target_player_controller)
{
    for (auto [account_id, player_controller] : Players)
    {
        if (!memory_manager::is_bad_read_pointer(player_controller, false) && player_controller)
        {
            if (player_controller == target_player_controller)
            {
                std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
                
                return converter.to_bytes(account_id);
            }
        }
    }

    return "";
}

AFortPlayerController* player_component::get_player_by_account_id(std::wstring in_account_id)
{
    for (auto [account_id, player_controller] : Players)
    {
        if (account_id == in_account_id)
        {
            if (!memory_manager::is_bad_read_pointer(player_controller) && player_controller)
            {
                return player_controller;
            }
        }
    }
    
    return nullptr;   
}

void player_component::handle_player_damaged(AFortPlayerPawn* HitActor, AFortPlayerController* Instigator, AActor* DamageCauser, int Damage, FName BoneName)
{
    if (Instigator)
    {
        if (DamageCauser->IsA(AFortWeapon::StaticClass()))
        {
            FDamageDealtMapValue& Value = DamageDealtMap[Instigator];
            FPlayerStats& PlayerStats = PlayerStatsMap[Instigator];

            if (UFortItemDefinition* ItemDefinition = static_cast<AFortWeapon*>(DamageCauser)->WeaponData())
            {
                // Bump hits 
                if (ItemDefinition->IsA(UFortWeaponRangedItemDefinition::StaticClass()))
                {
                    // HeadShots
                    if (BoneName == FName(TEXT("HEAD")))
                    {
                        PlayerStats.HeadShots++;

                        // Track headshot damage
                        Value.HeadshotDamage += Damage;
                    }
                    else
                    {
                        PlayerStats.Hits++;
                    }
                }

                // Damage taken
                if (HitActor)
                {
                    if (HitActor->IsA(APlayerPawn_Athena_C::StaticClass()))
                    {
                        if (AFortPlayerController* PlayerController = HitActor->Controller())
                        {
                            PlayerStatsMap[PlayerController].DamageTaken += Damage;
                        }
                    }
                }
                
                if (STATIC_HASPROPERTY(ItemDefinition, GameplayTags))
                {
                    switch (get_damage_causer(ItemDefinition->GameplayTags()))
                    {
                    case EDamageCauser::Pickaxe:
                        Value.PickaxeDamage += Damage;
                        break;
                    case EDamageCauser::Sniper:
                        Value.SniperDamage += Damage;
                        break;
                    case EDamageCauser::Pistol:
                        {
                            if (static_cast<AFortWeapon*>(DamageCauser)->IsSuppressed())
                            {
                                Value.SuppressedPistolDamage += Damage;
                            }
                            else
                            {
                                Value.PistolDamage += Damage;
                            }
                            break;
                        }
                    case EDamageCauser::Shotgun:
                        Value.ShotgunDamage += Damage;
                        break;
                    case EDamageCauser::AssaultRifle:
                        Value.AssaultRifleDamage += Damage;
                        break;
                    case EDamageCauser::Grenade:
                        Value.GrenadeDamage += Damage;
                        break;
                    case EDamageCauser::SMG:
                        {
                            if (static_cast<AFortWeapon*>(DamageCauser)->IsSuppressed())
                            {
                                Value.SuppressedSMGDamage += Damage;
                            }
                            else
                            {
                                Value.SMGDamage += Damage;
                            }
                            break;
                        }
                    case EDamageCauser::GrenadeLauncher:
                        Value.GrenadeLauncherDamage += Damage;
                        break;
                    case EDamageCauser::LMG:
                        Value.LMGDamage += Damage;
                        break;
                    case EDamageCauser::Minigun:
                        Value.MinigunDamage += Damage;
                        break;
                    case EDamageCauser::RPG:
                        Value.RPGDamage += Damage;
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
}

uint8_t player_component::convert_quick_chat_index_to_message(uint8_t Index)
{
    UEnum* ETeamMemberState = FindObject<UEnum>(TEXT("/Script/FortniteGame.ETeamMemberState"));

    if (Index == 0)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("EnemySpotted")) - 1;
    }
    if (Index == 1)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("NeedBandages")) - 1;
    }
    if (Index == 2)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("NeedShields")) - 1;
    }
    if (Index == 3)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("NeedMaterials")) - 1;
    }
    if (Index == 4)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("NeedWeapon")) - 1;
    }
    if (Index == 5)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("NeedAmmoLight")) - 1;
    }
    if (Index == 6)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("NeedAmmoMedium")) - 1;
    }
    if (Index == 7)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("NeedAmmoHeavy")) - 1;
    }
    if (Index == 8)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("NeedAmmoShells")) - 1;
    }
    if (Index == 9)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("NeedAmmoRocket")) - 1;
    }

    return 0;
}

void player_component::initialize_player_banner_icon(AFortPlayerController* TargetPlayerController, std::string BannerIcon)
{
    if (BannerIcon.find("HomebaseBannerIcon:") != std::string::npos)
    {
        BannerIcon = BannerIcon.substr(BannerIcon.find(":") + 1);
        
        TO_WSTRING(BannerIcon, wBannerIcon);

        if (FindPropertyByName(FFortAthenaLoadout::GetClass(), L"BannerIconId"))
        {
            static_cast<AFortPlayerControllerAthena*>(TargetPlayerController)->CustomizationLoadout().BannerIconId() = wBannerIcon;
        }

        if (AFortPlayerStateAthena* PlayerState = TargetPlayerController->PlayerState())
        {
            if (STATIC_HASPROPERTY(PlayerState, Banner))
            {
                if (FindPropertyByName(FPlayerBannerInfo::GetClass(), L"IconId"))
                {
                    PlayerState->Banner().IconId() = wBannerIcon;
                }
            }
        }
    }
}

void player_component::initialize_player_banner_level(AFortPlayerController* TargetPlayerController, int Level)
{
    if (AFortPlayerStateAthena* PlayerState = TargetPlayerController->PlayerState())
    {
        if (STATIC_HASPROPERTY(PlayerState, Banner))
        {
            if (FindPropertyByName(FPlayerBannerInfo::GetClass(), L"Level"))
            {
                PlayerState->Banner().Level() = Level;
            }
        }
    }
}

void player_component::initialize_player_banner_color(AFortPlayerController* TargetPlayerController, std::string BannerColor)
{
    if (BannerColor.find("HomebaseBannerColor:") != std::string::npos)
    {
        BannerColor = BannerColor.substr(BannerColor.find(":") + 1);
        
        TO_WSTRING(BannerColor, wBannerColor);

        if (FindPropertyByName(FFortAthenaLoadout::GetClass(), L"BannerColorId"))
        {
            static_cast<AFortPlayerControllerAthena*>(TargetPlayerController)->CustomizationLoadout().BannerColorId() = wBannerColor;
        }
    
        if (AFortPlayerStateAthena* PlayerState = TargetPlayerController->PlayerState())
        {
            if (STATIC_HASPROPERTY(PlayerState, Banner))
            {
                if (FindPropertyByName(FPlayerBannerInfo::GetClass(), L"ColorId"))
                {
                    PlayerState->Banner().ColorId() = wBannerColor;
                }
            }
        }
    }
}

EDamageCauser player_component::get_damage_causer(FGameplayTagContainer& Tags)
{
    static std::map<FName, EDamageCauser> death_cause_map =
    {
        { L"Weapon.Ranged.Shotgun", EDamageCauser::Shotgun },
        { L"Weapon.Ranged.Shotgun.Pump", EDamageCauser::Shotgun },
        { L"Weapon.Ranged.Shotgun.Tactical", EDamageCauser::Shotgun },
        { L"Weapon.Ranged.Assault", EDamageCauser::AssaultRifle },
        { L"weapon.ranged.assault.burst", EDamageCauser::AssaultRifle },
        { L"weapon.ranged.assault.standard", EDamageCauser::AssaultRifle },
        { L"weapon.ranged.assault.silenced", EDamageCauser::AssaultRifle },
        { L"Weapon.Ranged.LMG", EDamageCauser::LMG },
        { L"Weapon.Ranged.Pistol", EDamageCauser::Pistol },
        { L"Weapon.Ranged.Sniper", EDamageCauser::Sniper },
        { L"weapon.ranged.sniper.bolt", EDamageCauser::Sniper },
        { L"Weapon.Ranged.SMG", EDamageCauser::SMG },
        { L"Weapon.Ranged.Crossbow", EDamageCauser::Sniper },
        { L"Weapon.Ranged.Minigun", EDamageCauser::Minigun },
        { L"weapon.ranged.heavy.rocket_launcher", EDamageCauser::RPG},
        { L"weapon.ranged.heavy.grenade_launcher", EDamageCauser::GrenadeLauncher},
        { L"Weapon.ranged.heavy.grenade", EDamageCauser::Grenade},
        { L"Weapon.Melee.Impact.Pickaxe", EDamageCauser::Pickaxe}
    };

    for (FGameplayTag gameplay_tag : Tags.GameplayTags)
    {
        if (death_cause_map.contains(gameplay_tag.TagName))
        {
            // Return a death cause if we have it mapped
            return death_cause_map[gameplay_tag.TagName];
        }
    }

    return EDamageCauser::None;
}

void player_component_hooks::ServerAcknowledgePossession_Hook(APlayerController* PlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(UObject*, P);

    // Update acknowledged pawn
    PlayerController->AcknowledgedPawn() = P;
    
    return ServerAcknowledgePossession(PlayerController, StackFrame);
}

void player_component_hooks::ServerReturnToMainMenu_Hook(AFortPlayerController* FortPlayerController)
{
    // Make the player return to main menu
    FortPlayerController->ClientReturnToMainMenu(L"");   
}

void player_component_hooks::ServerChoosePart_Hook(AFortPlayerPawn* FortPlayerPawn, FFrame& StackFrame)
{
    auto config = config_manager::get_server_config();
    if (config["scp_enabled"] == true)
    {
        ServerChoosePart(FortPlayerPawn, StackFrame);
    }
}

void player_component_hooks::PostLogin_Hook(AFortGameModeAthena* FortGameModeAthena, APlayerController* NewPlayer)
{
    // Call native function
    PostLogin(FortGameModeAthena, NewPlayer);

    // Kick the player if a ban record exists
    if (moderation_component::is_player_banned(static_cast<AFortPlayerController*>(NewPlayer)))
    {
        spdlog::info(L"Player '{}' attempted to join but is banned. Kicking...", static_cast<AFortPlayerController*>(NewPlayer)->MyPlayerInfo()->PlayerName().c_str());
        static_cast<AFortPlayerController*>(NewPlayer)->ClientReturnToMainMenu(L"You have been banned from this server!");
    }

    // Grant admin permission if the player is an admin
    moderation_component::grant_admin_permission_safe(static_cast<AFortPlayerController*>(NewPlayer));

    // If livespectator, make sure the player is an admin
    if (player_component::is_live_spectator(NewPlayer))
    {
        if (NewPlayer->CheatManager() == nullptr)
        {
            static_cast<AFortPlayerController*>(NewPlayer)->ClientReturnToMainMenu(L"Only Server Admins are allowed to live spectate.");
            
            return;
        }
    }
    
    // Only proceed if this is an athena playercontroller
    if (NewPlayer->IsA(FortGameModeAthena->PlayerControllerClass()))
    {
        // Make sure the player is joining for the first time (ONLY FOR NON-ADMINS)
        if (NewPlayer->PlayerState()->NumRejoins() > 0 && NewPlayer->CheatManager() == nullptr)
        {
            spdlog::error(L"Player {} attempted to rejoin. Kicking...", static_cast<AFortPlayerController*>(NewPlayer)->MyPlayerInfo()->PlayerName().c_str());
                
            static_cast<AFortPlayerController*>(NewPlayer)->ClientReturnToMainMenu(L"You aren't allowed to rejoin. Please restart the game with a different name and try again.");

            return;
        }

        // Register quest manager
        if(STATIC_HASPROPERTY(NewPlayer, XPComponent))
        {
            static_cast<AFortPlayerController*>(NewPlayer)->XPComponent()->bRegisteredWithQuestManager() = true;
        }
            
        // Spawn QuickBars
        AFortQuickBars* new_quickbars = SpawnDefaultActor<AFortQuickBars>(*GWorld, FindObject(L"/Script/FortniteGame.FortQuickBars"));
            
        AFortQuickBars*& player_quickbars = NewPlayer->HasProperty(L"QuickBars") ?
            static_cast<AFortPlayerController*>(NewPlayer)->QuickBars() : static_cast<AFortPlayerController*>(NewPlayer)->ClientQuickBars();

        // Set quickbars
        player_quickbars = new_quickbars;
            
        // Set ownership
        player_quickbars->SetOwner(NewPlayer);
        
        // Assign team
        if (team_component::find_team(static_cast<AFortPlayerController*>(NewPlayer)) == nullptr)
        {
            spdlog::error(L"Couldn't find a valid team for player '{}'", static_cast<AFortPlayerController*>(NewPlayer)->MyPlayerInfo()->PlayerName().c_str());
        
            static_cast<AFortPlayerController*>(NewPlayer)->ClientReturnToMainMenu(L"");
        }

#ifdef _RELEASE
        team_component::set_team(static_cast<AFortPlayerController*>(NewPlayer), team_component::find_era_team(static_cast<AFortPlayerController*>(NewPlayer), true)->get_team_index(), team_component::find_era_team(static_cast<AFortPlayerController*>(NewPlayer), false)->get_team_index());
#elif _TESTING
        team_component::set_team(static_cast<AFortPlayerController*>(NewPlayer), 2, 2);
#endif
        
        // Make pawn invulnerable on spawn island
        UKismetSystemLibrary::SetBoolPropertyByName(static_cast<AFortPlayerController*>(NewPlayer)->Pawn(), L"bIsInvulnerable", true);

        // Prevent camera from shaking while spectating or when using the guided missile
        NewPlayer->PlayerCameraManager()->ViewRollMin() = 0.f;
        NewPlayer->PlayerCameraManager()->ViewRollMax() = 0.f;

        // Bind OnStartedRespawn delegate
        if (STATIC_HASPROPERTY(NewPlayer, OnStartedRespawn))
        {
            static_cast<AFortPlayerControllerAthena*>(NewPlayer)->OnStartedRespawn().Bind(TScriptDelegate(static_cast<AFortPlayerController*>(NewPlayer), L"OnPlayerStartedRespawn__DelegateSignature"));
        }

        // Set Player ID (Sets teams on traps automatically)
        NewPlayer->PlayerState()->WorldPlayerId() = player_component::global_player_id++;
        
        // Drop loading screen
        static_cast<AFortPlayerController*>(NewPlayer)->bHasServerFinishedLoading() = true;

        // Update ping variable
        UKismetSystemLibrary::SetBoolPropertyByName(NewPlayer->PlayerState(), TEXT("bShouldUpdateReplicatedPing"), true);
        
        // Notify gamemode
        (*GWorld)->AuthorityGameMode()->OnPlayerJoined(NewPlayer);

#ifdef _RELEASE
        // Notify era services
        gameserver_client::Client->GameServer_Status.number_of_players = FortGameModeAthena->AlivePlayers().Num();
        gameserver_client::Client->SendUpdate();
#endif

        // Notify backend
        player_component::PostLogin(static_cast<AFortPlayerController*>(NewPlayer));

        spdlog::info(L"Player '{}' joined and spawned", static_cast<AFortPlayerController*>(NewPlayer)->MyPlayerInfo()->PlayerName().c_str());
    }
}

UObject* player_component_hooks::GetHeroDefinition_Hook(UObject* Hero)
{
    static UObject* hero_definition = FindObject(L"/Game/Athena/Heroes/HID_001_Athena_Commando_F.HID_001_Athena_Commando_F", true) ?
        FindObject(L"/Game/Athena/Heroes/HID_001_Athena_Commando_F.HID_001_Athena_Commando_F", true) :
        FindObject(L"/Game/Athena/Heroes/HID_Commando_Athena_25.HID_Commando_Athena_25", true);

    if (hero_definition == nullptr)
    {
        // Failed to find hero definition
        spdlog::error(L"Failed to override hero definition for hero '{}', as specified hero definition does not exist",
            Hero->GetName().c_str());
    }

    return hero_definition;
}

void player_component_hooks::ServerPlaySprayItem_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(UAthenaSprayItemDefinition*, SprayAsset);
    
    player_component::play_emote_item(FortPlayerController, SprayAsset);
}

void player_component_hooks::ServerPlayEmoteItem_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(UAthenaDanceItemDefinition*, EmoteAsset);

    player_component::play_emote_item(FortPlayerController, EmoteAsset);
}

void player_component_hooks::ServerSendZiplineState_Hook(AFortPlayerPawn* FortPlayerPawn, FFrame& StackFrame)
{
    GET_PARAMETER(FZiplinePawnState, InZiplineState);

    // Update pawn zipline state
    memcpy(&FortPlayerPawn->ZiplineState(), &InZiplineState, FZiplinePawnState::GetStructSize());

    // Update component
    if (UpdateZiplineState)
    {
        UpdateZiplineState(FortPlayerPawn);
    }

    if (InZiplineState.bJumped())
    {
        // CREDIT: WIKTORWIKTOR12
        float ZiplineJumpDampening = FortPlayerPawn->ZiplineJumpDampening().Eval(0);
        float ZiplineJumpStrength = FortPlayerPawn->ZiplineJumpStrength().Eval(0);

        FVector Velocity = FortPlayerPawn->GetVelocity();

        FVector LaunchVector;

        if (ZiplineJumpDampening * Velocity.X >= -750.0)
        {
            LaunchVector.X = fminf(ZiplineJumpDampening * Velocity.X, 750.0);
        }
        else
        {
            LaunchVector.X = -750.0;
        }
        
        if (ZiplineJumpDampening * Velocity.Y >= -750.0)
        {
            LaunchVector.Y = fminf(ZiplineJumpDampening * Velocity.Y, 750.0);
        }
        else
        {
            LaunchVector.Y = -750.0;
        }
        
        LaunchVector.Z = ZiplineJumpStrength;

        FortPlayerPawn->LaunchCharacterJump(LaunchVector, false, false, false, true);
    }
}

void player_component_hooks::ServerReadyToStartMatch_Hook(AFortPlayerController* FortPlayerController)
{
    ServerReadyToStartMatch(FortPlayerController);
    
    if (STATIC_HASPROPERTY(FortPlayerController->PlayerState(), bHasStartedPlaying))
    {
        if (FortPlayerController->PlayerState()->bHasStartedPlaying() == false)
        {
            // Grant harvesting tool
            inventory_component::add_item(FortPlayerController, static_cast<AFortPlayerControllerAthena*>(FortPlayerController)->CustomizationLoadout().Pickaxe() ? static_cast<AFortPlayerControllerAthena*>(FortPlayerController)->CustomizationLoadout().Pickaxe()->WeaponDefinition() : FindObject<UFortItemDefinition>(L"WID_Harvest_Pickaxe_Athena_C_T01", false, true), 1);
        
            for (FItemAndCount starting_item : (*GWorld)->AuthorityGameMode()->StartingItems())
            {
                // Grant default item
                inventory_component::add_item(FortPlayerController, starting_item.Item, starting_item.Count);
            }
        
            // Equip pickaxe
            FortPlayerController->ServerExecuteInventoryItem(inventory_component::get_pickaxe(FortPlayerController)->ItemEntry().ItemGuid());
    
            creative_component::handle_player_join(FortPlayerController->NetConnection());

            UKismetSystemLibrary::SetBoolPropertyByName(FortPlayerController->PlayerState(), L"bHasStartedPlaying", true);
        }   
    }
}

void player_component_hooks::K2_OnRestartPlayer_Hook(UObject* Object, UObject* Function, void* Params)
{
    GET_PROCESS_EVENT_PARAMETER(AFortPlayerControllerAthena*, NewPlayer);

    // Give abilities to newly spawned pawns
    if (NewPlayer->Pawn() != nullptr)
    {
        UObject* ability_set = FindObject(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer") ?
            FindObject(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer") : FindObject(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_DefaultPlayer.GAS_DefaultPlayer");
        
        ability_component::apply_ability_set(NewPlayer->Pawn(), ability_set);

        // Apply CPs
        NewPlayer->Pawn()->PlayerState()->ApplyCharacterCustomization(NewPlayer->Pawn());
        
        // Apply backpack, if specified
        player_component::ApplyBackpack(NewPlayer);

        // Equip the pickaxe on landing
        if (STATIC_HASPROPERTY(NewPlayer->Pawn(), OnPawnLanded))
        {
            NewPlayer->Pawn()->OnPawnLanded().Bind(TScriptDelegate<AFortPawn>(NewPlayer->Pawn(), TEXT("OnPawnLandedImplementation")));
        }

        // Implementable event
        (*GWorld)->AuthorityGameMode()->OnPawnFinishedSpawning(NewPlayer->Pawn());
    }
}

void player_component_hooks::ServerAddMapMarker_Hook(UAthenaMarkerComponent* MarkerComponent, FFrame& StackFrame)
{
    if (!WorldMarkerDataCtr)
    {
        spdlog::error(L"ERROR: Unable to add map marker, WorldMarkerDataCtr is invalid.");
        return;
    }
    
    GET_PARAMETER(FFortClientMarkerRequest, MarkerRequest);
    
    AFortPlayerController* PlayerController = static_cast<AFortPlayerController*>(MarkerComponent->GetOwner());

    // Construct FFortWorldMarkerData
    FFortWorldMarkerData* FortWorldMarkerData = static_cast<FFortWorldMarkerData*>(malloc(FFortWorldMarkerData::GetStructSize()));
    
    FortWorldMarkerData = WorldMarkerDataCtr(FortWorldMarkerData);

    FMarkerID MarkerID(PlayerController->PlayerState()->PlayerID(), MarkerRequest.InstanceId());
    FortWorldMarkerData->MarkerID() = MarkerID;
    FortWorldMarkerData->Owner() = PlayerController->PlayerState();
    FortWorldMarkerData->MarkerType() = MarkerRequest.MarkerType();
    FortWorldMarkerData->WorldPosition() = MarkerRequest.WorldPosition();
    FortWorldMarkerData->WorldPositionOffset() = MarkerRequest.WorldPositionOffset();
    FortWorldMarkerData->WorldNormal() = MarkerRequest.WorldNormal();

    if (MarkerRequest.MarkedActor())
    {
        FortWorldMarkerData->MarkedActor().WeakPtr = MarkerRequest.MarkedActor();
        FortWorldMarkerData->MarkedActorClass().WeakPtr = MarkerRequest.MarkedActor()->Class;

        UEnum* WorldMarkerType = FindObject<UEnum>(L"/Script/FortniteGame.EFortWorldMarkerType");
        if (FortWorldMarkerData->MarkerType() == WorldMarkerType->GetEnumIndex(L"Item"))
        {
            AFortPickup* MarkedActor = static_cast<AFortPickup*>(MarkerRequest.MarkedActor());

            if (FindPropertyByName(FFortWorldMarkerData::GetClass(), L"ItemDefinition"))
            {
                FortWorldMarkerData->ItemDefinition() = MarkedActor->PrimaryPickupItemEntry().ItemDefinition();
            }
            if (FindPropertyByName(FFortWorldMarkerData::GetClass(), L"ItemCount"))
            {
                FortWorldMarkerData->ItemCount() = MarkedActor->PrimaryPickupItemEntry().Count();
            }
        }
    }

    // Call ClientAddMarker for all the teammates
    if (auto TeamInfo = team_component::find_team(PlayerController))
    {
        for (auto TeamMember : TeamInfo->TeamMembers())
        {
            AFortPlayerControllerAthena* TeamPlayerController = static_cast<AFortPlayerControllerAthena*>(TeamMember);

            // Only do for teammates
            if (TeamPlayerController != PlayerController)
            {
                UAthenaMarkerComponent* TeamMarkerComponent = TeamPlayerController->MarkerComponent();

                if (TeamMarkerComponent)
                {
                    TeamMarkerComponent->ClientAddMarker(FortWorldMarkerData);
                }
            }
        }
    }
}

long long player_component_hooks::OnTargetDataReadyCallback_Hook(UFortGameplayAbility_RangedWeapon* RangedWeaponAbility, FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag)
{
    if (anticheat_component::ValidateTargetData(RangedWeaponAbility))
    {
        if (static bool bRun = GetEngineVersionEasy() > 422)
        {
            if (RangedWeaponAbility)
            {
                AFortWeapon* SourceWeapon = RangedWeaponAbility->CurrentWeapon();

                // Controller & pawn
                AFortPlayerPawn* Pawn = static_cast<AFortPlayerPawn*>(SourceWeapon->GetOwner());
                AFortPlayerController* Controller = Pawn->Controller();

                if (!SourceWeapon->IsProjectileWeapon())
                {
                    if (Controller)
                    {
                        TArray<uint8> HitReplaces{};
                        for (uint8 i = 0; i < InData.GetData().Num() && i < 255; ++i)
                        {
                            if (FFortGameplayAbilityTargetData_SingleTargetHit* SingleTargetHit = InData.Get(i))
                            {
                                if (SingleTargetHit->bHitReplaced())
                                {
                                    HitReplaces.Add(i);
                                }
                            }
                        }

                        Controller->ClientConfirmTargetData(InData.UniqueId, true, HitReplaces);
                    }
                }
            }
        }
    
        return OnTargetDataReadyCallback(RangedWeaponAbility, InData, ApplicationTag);
    }

    return 0;
}

void player_component_hooks::ServerPlaySquadQuickChatMessage_Hook(AFortPlayerControllerAthena* SenderPC, FFrame& StackFrame)
{
    GET_PARAMETER(FAthenaQuickChatActiveEntry, ChatEntry);
    
    if (SenderPC)
    {
        if (AFortPlayerStateAthena* PlayerState = SenderPC->PlayerState())
        {
            STATIC_HASPROPERTY(PlayerState, ReplicatedTeamMemberState);
            STATIC_HASPROPERTY(PlayerState, TeamMemberState);
            
            if (bExists_TeamMemberState &&
                bExists_ReplicatedTeamMemberState)
            {
                PlayerState->ReplicatedTeamMemberState() = player_component::convert_quick_chat_index_to_message(ChatEntry.Index());
                PlayerState->TeamMemberState() = player_component::convert_quick_chat_index_to_message(ChatEntry.Index());

                SenderPC->PlayerState()->OnRep_ReplicatedTeamMemberState();
            }
        }
        
        player_component::play_emote_item(SenderPC, FindObject<UAthenaDanceItemDefinition>(L"/Game/Athena/Items/Cosmetics/Dances/Emoji/Emoji_Comm.Emoji_Comm"), false);
    }
}

// Only use this to keep track of sniper damage, as projectiles are not taken into account by NetMulticast_Athena_BatchedDamageCues
void player_component_hooks::OnDamageServer_Hook(UObject* Self, UObject* Function, void* Params)
{
    GET_PROCESS_EVENT_PARAMETER(AFortPlayerController*, InstigatedBy);   
    GET_PROCESS_EVENT_PARAMETER(float, Damage);   
    GET_PROCESS_EVENT_PARAMETER(AActor*, DamageCauser);
    GET_PROCESS_EVENT_PARAMETER(FHitResult, HitInfo);

    if (Self && InstigatedBy && DamageCauser)
    {
        AFortWeapon* FortWeapon = nullptr;
        
        if (DamageCauser->IsA(AFortWeapon::StaticClass()))
        {
            FortWeapon = static_cast<AFortWeapon*>(DamageCauser);
        }
        else if (DamageCauser->IsA(AFortProjectileAthena::StaticClass()))
        {
            if (DamageCauser->Owner()->IsA(AFortWeapon::StaticClass()))
            {
                FortWeapon = static_cast<AFortWeapon*>(DamageCauser->Owner());
            }
        }

        if (FortWeapon)
        {
            if (Self->IsA(APlayerPawn_Athena_C::StaticClass()))
            {
                player_component::handle_player_damaged(static_cast<AFortPlayerPawn*>(Self), InstigatedBy, FortWeapon, Damage, HitInfo.BoneName());
            }
        }
    }
}

void AFortPawn::SetShield(float NewShieldVal)
{
    STATIC_FINDFUNCTION(SetShield)
    STATIC_FINDFUNCTION(SetHealth)

    if (Function_SetShield == nullptr)
    {
        if (player_component::HealthProperty)
        {
            *player_component::HealthProperty = FindPropertyByName(UFortHealthSet::StaticClass(), L"CurrentShield");
            Call(Function_SetHealth, NewShieldVal);
            *player_component::HealthProperty = FindPropertyByName(UFortHealthSet::StaticClass(), L"Health");
        }
    }
    else
    {
        Call(Function_SetShield);
    }
}