#include "include.hpp"
#include "player_component.hpp"

#include "ability_component.hpp"
#include "config_manager.hpp"
#include "creative_component.hpp"
#include "inventory_component.hpp"
#include "moderation_component.hpp"
#include "net_component.hpp"
#include "team_component.hpp"

player_component::player_component()
{
    // Initialize serverchoosepart
    if (const uintptr_t index = memory_manager::find_function_by_string(L"ServerChoosePart_Validate", {0xff, 0x90}, 2, false, false))
    {
        INIT_POINTER(ServerChoosePart_Implementation, FindObject(L"/Script/FortniteGame.Default__FortPlayerPawn")->VTable[index/8]);
    }

    INIT_POINTER(UpdateZiplineState, memory_manager::find_function_by_string(L"AFortPlayerPawn::OnRep_ZiplineState"));
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerAcknowledgePossession, L"/Script/Engine.PlayerController.ServerAcknowledgePossession", player_component_hooks::ServerAcknowledgePossession_Hook);
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerChoosePart, L"/Script/FortniteGame.FortPlayerPawn.ServerChoosePart", player_component_hooks::ServerChoosePart_Hook);
    
    REGISTER_FUNCTION_DETOUR(player_component_hooks::PostLogin, memory_manager::find_function_by_string(L"FortGameModeAthena: Player [%s] PostLogin"), player_component_hooks::PostLogin_Hook);
    
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerPlayEmoteItem, L"/Script/FortniteGame.FortPlayerController.ServerPlayEmoteItem", player_component_hooks::ServerPlayEmoteItem_Hook);
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerSendZiplineState, L"/Script/FortniteGame.FortPlayerPawn.ServerSendZiplineState", player_component_hooks::ServerSendZiplineState_Hook);
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerReturnToMainMenu, L"/Script/FortniteGame.FortPlayerController.ServerReturnToMainMenu", player_component_hooks::ServerReturnToMainMenu_Hook);
    
    if (GetEngineVersionEasy() < 421)
    {
        // Hooking this on later versions causes issues
        REGISTER_FUNCTION_DETOUR(player_component_hooks::GetHeroDefinition, memory_manager::find_function_by_string(L"Trying to apply homebase effects on player setup but hero or herotype is not set.", { 0xE8, 0x100, 0x100, 0x100, 0xff }, 1), player_component_hooks::GetHeroDefinition_Hook);
    }
    
    DetourTransactionCommit();

    // Finish spawning a player
    REGISTER_PROCESS_EVENT_DETOUR(L"/Script/Engine.GameModeBase.K2_OnRestartPlayer", player_component_hooks::K2_OnRestartPlayer_Hook);

    // Server choose part is false by default
    bEnableServerChoosePart = false;
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
        }

        spdlog::error(L"Invalid EncryptionToken received, kicking...");
    }
    else
    {
        spdlog::error(L"Player tried to join without EncryptionToken, kicking...");
    }    
#endif
    
    return false;   
}

void player_component::Setup(AFortPlayerController* NewPlayer, const FString& Options)
{
    if (UGameplayStatics::HasOption(Options, TEXT("EncryptionToken")))
    {
        FString Token = UGameplayStatics::ParseOption(Options, TEXT("EncryptionToken"));

        if (Token.IsValid())
        {
            std::wstring wToken = Token.c_str();
            
            // Get EncryptionToken as string
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            std::string sToken = converter.to_bytes(wToken);

            nlohmann::json server_config = config_manager::get_server_config();

            httplib::Client client("https://api.v1.internal.erafn.org");
            httplib::Headers headers = {
                { "x-era-token", "bn8HUsAfhogEq6E57ledvyGReK9n8E7XP53AbRomzvItAexNB09aXiTgFhoPvUD1" }
            };
            std::string URL = "/api/internal/users/" + sToken.substr(0, sToken.find(":")) + "/profile";
            auto Result = client.Get(URL, headers);

            // OK
            if (Result->status == 200)
            {
                nlohmann::json JsonResult = nlohmann::json::parse(Result->body);

                // Check if the player is an admin
                if (JsonResult.contains("data"))
                {
                    nlohmann::json data = JsonResult["data"];
                    if (data.contains("admin"))
                    {
                        if (static_cast<bool>(data["admin"]))
                        {
                            NewPlayer->CheatManager() = static_cast<UCheatManager*>(UGameplayStatics::SpawnObject(UCheatManager::StaticClass(), NewPlayer));

                            if (data.contains("display_name"))
                            {
                                spdlog::info("Player '{}' joined as an admin [AUTHORIZED BY ERA SERVICES].", std::string(data["display_name"]));
                            }
                        }
                    }
                    if (data.contains("profile"))
                    {
                        nlohmann::json profile = data["profile"];

                        for (auto& elem : profile.items())
                        {
                            // If it's a valid string
                            if (elem.value().is_string() && std::string(elem.value()).find(":") != std::string::npos)
                            {
                                std::string value = std::string(elem.value()).substr(std::string(elem.value()).find(":")+1);

                                UObject* Object = FindObject(std::wstring(value.begin(), value.end()).c_str(), false, true);

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
                    }
                }
            }
            else
            {
                spdlog::error("Returned status {} on '{}'", Result->status, URL.c_str());
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
    if (player_component::bEnableServerChoosePart == true)
    {
        ServerChoosePart(FortPlayerPawn, StackFrame);
    }
}

void player_component_hooks::PostLogin_Hook(AFortGameModeAthena* FortGameModeAthena, APlayerController* NewPlayer)
{
    // Call native function
    PostLogin(FortGameModeAthena, NewPlayer);

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

        // Grant harvesting tool
        inventory_component::add_item(static_cast<AFortPlayerController*>(NewPlayer), static_cast<AFortPlayerControllerAthena*>(NewPlayer)->CustomizationLoadout().Pickaxe() ? static_cast<AFortPlayerControllerAthena*>(NewPlayer)->CustomizationLoadout().Pickaxe()->WeaponDefinition() : FindObject<UFortItemDefinition>(L"WID_Harvest_Pickaxe_Athena_C_T01", false, true), 1);
            
        for (FItemAndCount starting_item : (*GWorld)->AuthorityGameMode()->StartingItems())
        {
            // Grant default item
            inventory_component::add_item(static_cast<AFortPlayerController*>(NewPlayer), starting_item.Item, starting_item.Count);
        }

        // Assign team
        if (team_component::find_team(static_cast<AFortPlayerController*>(NewPlayer)) == nullptr)
        {
            spdlog::error(L"Couldn't find a valid team for player '{}'", static_cast<AFortPlayerController*>(NewPlayer)->MyPlayerInfo()->PlayerName().c_str());
        
            static_cast<AFortPlayerController*>(NewPlayer)->ClientReturnToMainMenu(L"");
        }
    
        team_component::set_team(static_cast<AFortPlayerController*>(NewPlayer), team_component::find_team(static_cast<AFortPlayerController*>(NewPlayer))->Team());

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

        // Creative support
        creative_component::handle_player_join(NewPlayer->NetConnection());

        // Drop loading screen
        static_cast<AFortPlayerController*>(NewPlayer)->bHasServerFinishedLoading() = true;

        // Notify gamemode
        (*GWorld)->AuthorityGameMode()->OnPlayerJoined(NewPlayer);

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

void player_component_hooks::ServerPlayEmoteItem_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(UObject*, EmoteAsset);

    if (FortPlayerController->Pawn() && FortPlayerController->Pawn()->AbilitySystemComponent())
    {
        auto new_ability_spec = FGameplayAbilitySpec();
        
        static auto AthenaSprayItemDefinition = FindObject(L"/Script/FortniteGame.AthenaSprayItemDefinition");

        // set the ability in the newly created struct    TODO: ADD TOYS
        if (EmoteAsset->IsA(AthenaSprayItemDefinition))
        {
            new_ability_spec.Ability = FindObject(L"/Game/Abilities/Sprays/GAB_Spray_Generic.Default__GAB_Spray_Generic_C");
        }
        else
        {
            new_ability_spec.Ability = FindObject(L"/Game/Abilities/Emotes/GAB_Emote_Generic.Default__GAB_Emote_Generic_C");
        }
        
        new_ability_spec.SourceObject = EmoteAsset; // set the sourceobject, in this case the emote we want to execute
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
        GiveAbilityAndActivateOnce(FortPlayerController->Pawn()->AbilitySystemComponent(), unk, new_ability_spec);
    }
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
        
        // Equip the pickaxe
        NewPlayer->ServerExecuteInventoryItem(inventory_component::get_pickaxe(NewPlayer)->ItemEntry().ItemGuid());

        // Implementable event
        (*GWorld)->AuthorityGameMode()->OnPawnFinishedSpawning(NewPlayer->Pawn());
    }
}