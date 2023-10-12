#include "include.hpp"
#include "moderation_component.hpp"

#include "config_manager.hpp"
#include "elimination_component.hpp"
#include "inventory_component.hpp"
#include "loader_component.hpp"
#include "looting_component.hpp"
#include "net_component.hpp"
#include "pickup_component.hpp"
#include "player_component.hpp"
#include "playlist_component.hpp"
#include "qos_component.hpp"
#include "reviving_component.hpp"
#include "team_component.hpp"
#include "../gui/renderer.hpp"
#include "../utils/webhook_manager.hpp"

moderation_component::moderation_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(moderation_component_hooks::Login, memory_manager::find_function_by_string(L"Login: Couldn't spawn player controller of class %s"), moderation_component_hooks::Login_Hook);    
    REGISTER_UFUNCTION_DETOUR(moderation_component_hooks::ServerCheat, L"/Script/FortniteGame.FortPlayerController.ServerCheat", moderation_component_hooks::ServerCheat_Hook);
    
    DetourTransactionCommit();

    // initialize passwords
    auto config = config_manager::get_server_config();
    for (auto& elem : config["admin_passwords"].items())
    {
        if (elem.value().is_string())
        {
            auto password = std::string(elem.value());
            spdlog::info("Registered admin password: {}", password.c_str());
            passwords.push_back(std::wstring(password.begin(), password.end()).c_str());
        }
    }

    for (auto& elem : config["server_key"].items())
    {
        if (elem.value().is_string())
        {
            auto matchcode = std::string(elem.value());
            spdlog::info("Registered custom matchmaking code: {}", matchcode.c_str());
        }
    }
}

AFortPlayerController* moderation_component::get_player_by_name(FString InPlayerName)
{
    if (*GWorld)
    {
        if (UNetDriver* NetDriver = (*GWorld)->NetDriver())
        {
            for (int i = 0; i < NetDriver->ClientConnections().Num(); i++)
            {
                UNetConnection* ClientConnection = NetDriver->ClientConnections()[i];

                if (ClientConnection)
                {
                    if (AFortPlayerController* PlayerController = ClientConnection->PlayerController())
                    {
                        if (UFortRegisteredPlayerInfo* MyPlayerInfo = PlayerController->MyPlayerInfo())
                        {
                            if (MyPlayerInfo->PlayerName() == InPlayerName)
                            {
                                // Found eligible player
                                return PlayerController;
                            }
                        }
                    }
                }
            }
        }
    }
    
    return nullptr;    
}

TArray<UNetConnection*> moderation_component::get_all_players()
{
    if (*GWorld)
    {
        if (UNetDriver* NetDriver = (*GWorld)->NetDriver())
        {
            return NetDriver->ClientConnections();
        }
    }
}

void moderation_component::grant_admin_permission_safe(AFortPlayerController* JoiningPlayer)
{
    if (JoiningPlayer && JoiningPlayer->PlayerState())
    {
        // get the IP
        std::wstring wJoiningPlayerIP = JoiningPlayer->PlayerState()->SavedNetworkAddress().c_str();
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        std::string JoiningPlayerIP = converter.to_bytes(wJoiningPlayerIP);
        
        // Check if the player is a cached admin
        bool bIsAdmin = false;
        auto config = config_manager::get_server_config();
        for (auto& elem : config["server_admins"].items())
        {
            if (std::string(elem.value()) == JoiningPlayerIP)
            {
                bIsAdmin = true;
                break;
            }
        }
        for (auto& elem : config["opped_players"].items())
        {
            if (std::string(elem.value()) == JoiningPlayerIP)
            {
                bIsAdmin = true;
                break;
            }
        }
        if (bIsAdmin)
        {
            // user is now logged in as administrator, give them cheatmanager
            JoiningPlayer->CheatManager() = static_cast<UCheatManager*>(UGameplayStatics::SpawnObject(UCheatManager::StaticClass(), JoiningPlayer));
        }
        else
        {
            // If the player is not found in the admin cache, but has admin perms, add it to the cache
            if (JoiningPlayer->CheatManager())
            {
                config["server_admins"].push_back(JoiningPlayerIP);
                config_manager::save_server_config(config);
            }
        }
    }
}

bool moderation_component::is_registered_admin(AFortPlayerController* Target)
{
    if (Target == nullptr)
    {
        return false;
    }
    if (Target->PlayerState() == nullptr)
    {
        return false;
    }
    
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        
    auto config = config_manager::get_server_config();
    for (auto& elem : config["server_admins"].items())
    {
        if (std::string(elem.value()) == converter.to_bytes(Target->PlayerState()->SavedNetworkAddress().c_str()))
        {
            return true;
        }
    }
    
    return false;
}

void moderation_component::ban_player(AFortPlayerController* TargetPlayer, FString BanReason)
{
    if (TargetPlayer && BanReason.IsValid())
    {
        // Get the player's name and IP
        FString PlayerName = TargetPlayer->MyPlayerInfo()->PlayerName();
        FString PlayerIP = TargetPlayer->PlayerState()->SavedNetworkAddress();

        // Convert to std::string
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        std::string sPlayerName = converter.to_bytes(PlayerName.c_str());
        std::string sPlayerIP = converter.to_bytes(PlayerIP.c_str());

        // Create the Json object
        nlohmann::json BanInstance = {
            {sPlayerName, sPlayerIP}
        };
        
        // Save the ban
        auto config = config_manager::get_server_config();
        config["banned_players"].push_back(BanInstance);
        config_manager::save_server_config(config);

        // Kick the player
        TargetPlayer->ClientReturnToMainMenu(BanReason);
    }
}

void moderation_component::unban_player(std::string InPlayerName)
{
    std::vector<std::string> BanList;

    auto config = config_manager::get_server_config();
    int idx = -1;
    bool bFound = false;
    for (auto& elem : config["banned_players"].items())
    {
        idx++;
        if (elem.value().is_object())
        {
            for (auto& elem_ : elem.value().items())
            {
                if (std::string(elem_.key()) == InPlayerName)
                {
                    bFound = true;
                    break;
                }
            }

            if (bFound)
            {
                break;
            }
        }
    }

    if (bFound)
    {
        config["banned_players"].erase(idx);
        config_manager::save_server_config(config);
    }
}

bool moderation_component::is_player_banned(AFortPlayerController* Target)
{
    if (Target == nullptr)
    {
        return false;
    }
    if (Target->PlayerState() == nullptr)
    {
        return false;
    }
    
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

    auto config = config_manager::get_server_config();
    for (auto& elem : config["banned_players"].items())
    {
        if (elem.value().is_object())
        {
            for (auto& elem_ : elem.value().items())
            {
                if (std::string(elem_.value()) == converter.to_bytes(Target->PlayerState()->SavedNetworkAddress().c_str()))
                {
                    return true;
                }
            }
        }
    }
    
    return false;
}

void moderation_component::HandleAdminLoginCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    FString UserPassword;

    if (!FParse::Token(InStream, UserPassword, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing Password parameter."));
        return;
    }

    if (UserPassword.IsValid())
    {
        for (std::wstring password_to_check : passwords)
        {
            if (password_to_check == std::wstring(UserPassword.c_str()))
            {
                // user is now logged in as administrator, give them cheatmanager
                ExecutingPlayerController->CheatManager() = static_cast<UCheatManager*>(UGameplayStatics::SpawnObject(UCheatManager::StaticClass(), ExecutingPlayerController));

                // Get admin player info
                const std::wstring AdminPlayerName = ExecutingPlayerController->MyPlayerInfo() ? ExecutingPlayerController->MyPlayerInfo()->PlayerName().c_str() : L"INVALID";
                std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
                std::string sAdminPlayerName = converter.to_bytes(AdminPlayerName);
                const std::wstring AdminIPAddress = ExecutingPlayerController->PlayerState()->SavedNetworkAddress().c_str();
                std::string sAdminIPAddress = converter.to_bytes(AdminIPAddress);
                std::string sServerName = qos_component::best_region;
                if (auto Client = gameserver_client::Client)
                {
                    sServerName = Client->GameServer_Status.name.length() >= 2 && Client->GameServer_Status.name.length() <= 16 ? Client->GameServer_Status.name : qos_component::best_region;
                }

                // Cache IP
                auto config = config_manager::get_server_config();
                config["server_admins"].push_back(sAdminIPAddress);
                config_manager::save_server_config(config);
                        
                // Format message
                char Body[1024];
                sprintf_s(Body, "Player '%s' [%s] logged in as administrator [SERVER: %s]", sAdminPlayerName.c_str(), sAdminIPAddress.c_str(), sServerName.c_str());
                std::string sBody(Body);

                // Get webhook
                nlohmann::json payload = webhook_manager::get_webhook_by_name("webhook-adminlogin");
                payload["body"] = Body;

                // Send webhook
                webhook_manager::send_webhook(payload);

                // Notify player
                ExecutingPlayerController->ClientMessage(TEXT("Successfully logged in!"));
                        
                return;
            }
        }

        // Failed login
        ExecutingPlayerController->ClientMessage(TEXT("Invalid password."));
    }
}

void moderation_component::HandleKickPlayerCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    FString PlayerName;
            
    if (!FParse::Token(InStream, PlayerName, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing PlayerName parameter."));
        return;
    }

    FString OptionalKickReason;

    if (!FParse::Token(InStream, OptionalKickReason, 0))
    {
        OptionalKickReason = TEXT("Kicked by a server admin.");
    }

    if (PlayerName.IsValid())
    {
        if (PlayerName == L"@all")
        {
            ExecutingPlayerController->ClientMessage(TEXT("Unable to kick every player, kick just one/turn off the server."));
        }
        
        if (AFortPlayerController* TargetPlayerController = get_player_by_name(PlayerName))
        {
            if (OptionalKickReason.IsValid())
            {
                TargetPlayerController->ClientReturnToMainMenu(OptionalKickReason);
            }

            // Get name of the executing PC as string
            const std::wstring AdminPlayerName = ExecutingPlayerController->MyPlayerInfo() ? ExecutingPlayerController->MyPlayerInfo()->PlayerName().c_str() : L"INVALID";
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            std::string sAdminPlayerName = converter.to_bytes(AdminPlayerName);
            std::string sVictimPlayerName = converter.to_bytes(PlayerName.c_str());
            std::string sServerName = qos_component::best_region;
            if (auto Client = gameserver_client::Client)
            {
                sServerName = Client->GameServer_Status.name.length() >= 2 && Client->GameServer_Status.name.length() <= 16 ? Client->GameServer_Status.name : qos_component::best_region;
            }

            // Format message
            char Body[1024];
            sprintf_s(Body, "Player '%s' kicked '%s' [SERVER: %s]", sAdminPlayerName.c_str(), sVictimPlayerName.c_str(), sServerName.c_str());
            std::string sBody(Body);

            // Get webhooks
            std::vector<nlohmann::json> payloads = webhook_manager::get_webhooks_by_name("webhook-kick");

            // Send webhooks
            for (auto payload : payloads)
            {
                payload["body"] = Body;
                webhook_manager::send_webhook(payload);
            }
            
            ExecutingPlayerController->ClientMessage(TEXT("Player successfully kicked!"));
        }
        else
        {
            ExecutingPlayerController->ClientMessage(TEXT("Specified player could not be found."));
        }
    }
}

void moderation_component::HandleHelpCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    // Log everything to the player's console
    ExecutingPlayerController->ClientMessage(TEXT("--- COMMAND HELP LIST ---\ncheat ban <player name> (IP ban)\ncheat unban <banned player name>\ncheat kickplayer <player name> <optional kick reason> (Kicks a player with an optional reason)\ncheat banlist (displays all the usernames of banned people)\ncheat setwarmupduration <duration in seconds> (Sets the warmup duration, effective after a server restart)\ncheat listplayers (displays all the players in the match, dead and alive)\ncheat startmatch (Starts the bus countdown)\ncheat restartserver (Restarts the server)\ncheat resetserver (Performs a full server restart)\ncheat saferesetserver (Performs a full server restart whenever the match ends)\ncheat setservername <server name> (Sets a server name to be shown on the status website)\ncheat setserverpassword <serverpassword> (Sets a server's matchmaking key, needs restart to apply)\ncheat op <player> (Gives access to admin commands to a given player)\ncheat deop <player> (Revokes access to admin commands, doesn't work on players who are logged in as admins)\ncheat fly (Toggles fly)\ncheat setspeed <optional speed multiplier> (Changes the player speed, if no parameter is passed, speed will be set to default)\ncheat giveitem <playername> <item wid> <item count> (Gives an item to a player, if the inventory is full, spawns a pickup)\ncheat wipeinventory <playername> (Removes all primary quickbar items from inventory of a specified player)\ncheat setplaylist <playlist> (Updates the playlist, effective on restart)\ncheat toggleserverchoosepart (enable/disable scp)\ncheat kill <playername> (Kills a player)\ncheat togglekeepinventory (Only makes sense for modes with respawning, disables item drops on death)\ncheat revive <playername> (Revives a player)\ncheat infiniteammo (Enables infinite ammo for JUST you)\ncheat vanish\ncheat god <optional_playername>\ncheat launchplayer <optional_playername> (Launches the player with a height velocity of 15 000)\ncheat givewood <optional_playername>\ncheat givestone <optional_playername>\ncheat givemetal <optional_playername>\ncheat giveresources <optional_playername>\ncheat giveammo <optional_playername>\ncheat simulatebusjump <optional_playername> (Puts the player in the sky and begins skydiving mode)\ncheat spawnpickup <item wid> <item count>\ncheat changescale <scale> (Changes your scale)\ncheat loadsummon <actor full path> (Loads an actor and spawns it on the server)\ncheat spawnchest\ncheat badass <optional player name> (Gives you Save The World guns, infinite ammo, god)\nNOTE: command arguments have to be passed with \" \" if they have a space!\nExample: cheat kickplayer Owner \"Hacker, DM for appeal\""));
}

void moderation_component::HandleStartMatchCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    if (UWorld* CurrentWorld = *GWorld)
    {
        if (AFortGameModeAthena* CurrentGameMode = CurrentWorld->AuthorityGameMode())
        {
            // Update the countdown to 10 seconds, in order to start the match 
            CurrentGameMode->WarmupCountdownDuration() = 10.f;
            CurrentGameMode->WarmupEarlyCountdownDuration() = 10.f;

            ExecutingPlayerController->ClientMessage(TEXT("Starting match..."));
        }
    }
}

void moderation_component::HandleRestartServerCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    if (UWorld* CurrentWorld = *GWorld)
    {
        if (UNetDriver* CurrentNetDriver = CurrentWorld->NetDriver())
        {
            // Loop through each netconnection and close it
            for (int i = 0; i < CurrentNetDriver->ClientConnections().Num(); i++)
            {
                UNetConnection* ClientConnection = CurrentNetDriver->ClientConnections()[i];

                if (ClientConnection)
                {
                    // If there is an associated PlayerController, kick it properly
                    if (AFortPlayerController* PlayerController = ClientConnection->PlayerController())
                    {
                        PlayerController->ClientReturnToMainMenu(TEXT("Restarting Server."));
                    }
                }
            }
        }
    }
}

void moderation_component::HandleResetServerCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    if (UWorld* CurrentWorld = *GWorld)
    {
        if (UNetDriver* CurrentNetDriver = CurrentWorld->NetDriver())
        {
            // Loop through each netconnection and close it
            for (int i = 0; i < CurrentNetDriver->ClientConnections().Num(); i++)
            {
                UNetConnection* ClientConnection = CurrentNetDriver->ClientConnections()[i];

                if (ClientConnection)
                {
                    // If there is an associated PlayerController, kick it properly
                    if (AFortPlayerController* PlayerController = ClientConnection->PlayerController())
                    {
                        PlayerController->ClientReturnToMainMenu(TEXT("Restarting Server."));
                    }
                }
            }
        }
    }
    
    // Unregister the server
    gameserver_client::Client->SendLeave();
    
    // Quit
    RequestExitWithStatus(true, 0);
}

void moderation_component::HandleSafeResetServerCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    // Mark the server as pending shutdown
    loader_component::bPendingShutdown = true;

    // Notify
    ExecutingPlayerController->ClientMessage(TEXT("The server is going to perform a full restart as soon as the match ends."));
}

void moderation_component::HandleSetServerPasswordCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    // get the IP
    std::wstring wExecutingPlayerIP = ExecutingPlayerController->PlayerState()->SavedNetworkAddress().c_str();
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::string ExecutingPlayerIP = converter.to_bytes(wExecutingPlayerIP);
    
    // Make sure the player is an admin
    bool bIsAdmin = false;
    
    {
        auto config = config_manager::get_server_config();
        for (auto& elem : config["server_admins"].items())
        {
            if (std::string(elem.value()) == ExecutingPlayerIP)
            {
                bIsAdmin = true;
                break;
            }
        }
    }
    
    if (bIsAdmin)
    {
        FString InServerPassword;

        if (!FParse::Token(InStream, InServerPassword, 0))
        {
            ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing InServerPassword parameter."));
            return;
        }

        if (InServerPassword.IsValid())
        {
            std::string sInServerPassword = converter.to_bytes(InServerPassword.c_str());

            if (sInServerPassword.length() >= 4)
            {
                // Set a password
                auto config = config_manager::get_server_config();
                config["server_key"] = sInServerPassword;
                config_manager::save_server_config(config);
                
                ExecutingPlayerController->ClientMessage(TEXT("ServerPassword successfully set! Restart the server to apply"));
            }
            else
            {
                ExecutingPlayerController->ClientMessage(TEXT("ServerPassword has to be atleast 4 characters long!"));
            }
        }
    }
}

void moderation_component::HandleOpCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    // OP a player (Give them cheatmanager, without registering them as admin)
    FString TargetPlayerName;
    TArray<AFortPlayerController*> target_players;
    
    if (!FParse::Token(InStream, TargetPlayerName, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing Player parameter."));
        return;
    }

    if (TargetPlayerName.IsValid())
    {
        if (AFortPlayerController* target_player = get_player_by_name(TargetPlayerName))
        {
            target_players.Add(target_player);
        }
        else if(TargetPlayerName == L"@all")
        {
            for (UNetConnection* net_connection : get_all_players())
            {
                target_players.Add(net_connection->PlayerController());
            }
        }
        
        for (auto player_controller : target_players)
        {
            if (player_controller == nullptr)
            {
                ExecutingPlayerController->ClientMessage(TEXT("Couldn't find the specified player."));
                return;
            }

            if (player_controller->CheatManager() != nullptr)
            {
                ExecutingPlayerController->ClientMessage(TEXT("The specified player is already an admin!"));
                return;
            }
        
            // Op!
            player_controller->CheatManager() = static_cast<UCheatManager*>(UGameplayStatics::SpawnObject(UCheatManager::StaticClass(), player_controller));

            // Get IP
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            const std::wstring AdminIPAddress = player_controller->PlayerState()->SavedNetworkAddress().c_str();
            std::string sAdminIPAddress = converter.to_bytes(AdminIPAddress);
        
            // Save as opped
            auto config = config_manager::get_server_config();
            config["opped_players"].push_back(sAdminIPAddress);
            config_manager::save_server_config(config);

            ExecutingPlayerController->ClientMessage(TEXT("Successfully opped player!"));
        }
    }
}

void moderation_component::HandleDeopCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    // OP a player (Give them cheatmanager, without registering them as admin)
    FString TargetPlayerName;
    TArray<AFortPlayerController*> target_players;
    
    if (!FParse::Token(InStream, TargetPlayerName, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing Player parameter."));
        return;
    }

    if (TargetPlayerName.IsValid())
    {
        if (AFortPlayerController* target_player = get_player_by_name(TargetPlayerName))
        {
            target_players.Add(target_player);
        }
        else if(TargetPlayerName == L"@all")
        {
            for (UNetConnection* net_connection : get_all_players())
            {
                target_players.Add(net_connection->PlayerController());
            }
        }
        
        for (auto player_controller : target_players)
        {
            if (player_controller == nullptr)
            {
                ExecutingPlayerController->ClientMessage(TEXT("Couldn't find the specified player."));
                return;
            }

            if (player_controller->CheatManager() == nullptr)
            {
                ExecutingPlayerController->ClientMessage(TEXT("The specified player is not opped!"));
                return;
            }

            // Get the IP as std::string
            std::wstring wTargetPlayerIP = player_controller->PlayerState()->SavedNetworkAddress().c_str();
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            std::string TargetPlayerIP = converter.to_bytes(wTargetPlayerIP);

            // Make sure the player is not an admin
            auto config = config_manager::get_server_config();
            for (auto& elem : config["server_admins"].items())
            {
                if (std::string(elem.value()) == TargetPlayerIP)
                {
                    ExecutingPlayerController->ClientMessage(TEXT("You can't deop a server admin!"));
                    return;
                }
            }
            
            // Make sure the player is opped
            std::vector<std::string> opped_players;
            for (auto& elem : config["opped_players"].items())
            {
                opped_players.push_back(std::string(elem.value()));
            }

            if (std::find(opped_players.begin(), opped_players.end(), TargetPlayerIP) != opped_players.end())
            {
                // Remove from the vector
                opped_players.erase(std::find(opped_players.begin(), opped_players.end(), TargetPlayerIP));

                // Save the config
                config["opped_players"] = R"([])"_json;
                for (auto opped_player : opped_players)
                {
                    config["opped_players"].push_back(opped_player);
                }
                config_manager::save_server_config(config);
                
                // If the player is not a registered admin, de-op it
                player_controller->CheatManager() = nullptr;

                ExecutingPlayerController->ClientMessage(TEXT("Successfully deopped player!"));
            }
        }
    }
}

void moderation_component::HandleFlyCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    if (AFortPlayerPawn* MyFortPawn = ExecutingPlayerController->MyFortPawn())
    {
        static UEnum* EMovementMode = FindObject<UEnum>(L"/Script/Engine.EMovementMode");
        static uint8_t MOVE_Flying = EMovementMode->GetEnumIndex(TEXT("MOVE_Flying"));
        static uint8_t MOVE_Walking = EMovementMode->GetEnumIndex(TEXT("MOVE_Walking"));
        
        // Set the movement mode
        MyFortPawn->CharacterMovement()->MovementMode() = MyFortPawn->CharacterMovement()->MovementMode() == MOVE_Flying ? MOVE_Walking : MOVE_Flying;

        // Toggle bCheatFlying
        UKismetSystemLibrary::SetBoolPropertyByName(MyFortPawn->CharacterMovement(), TEXT("bCheatFlying"), !MyFortPawn->CharacterMovement()->bCheatFlying());

        ExecutingPlayerController->ClientMessage(TEXT("Toggled fly!"));
    }
}

void moderation_component::HandleSetSpeedCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    FString NewSpeedValue;

    if (!FParse::Token(InStream, NewSpeedValue, 0))
    {
        NewSpeedValue = TEXT("1");
    }

    if (AFortPlayerPawn* MyFortPawn = ExecutingPlayerController->MyFortPawn())
    {
        std::wstring wNewSpeedValue = NewSpeedValue.c_str();
        float fNewSpeedValue = std::stof(wNewSpeedValue);
        
        MyFortPawn->MovementSet()->SpeedMultiplier().CurrentValue() = fNewSpeedValue;
        
        ExecutingPlayerController->ClientMessage(TEXT("Updated speed!"));
    }
}

void moderation_component::HandleGiveItemCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    TArray<AFortPlayerController*> target_players = parse_command_target_players(InStream, ExecutingPlayerController);

    FString InWID;
    if (!FParse::Token(InStream, InWID, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing Item parameter."));
        return;
    }

    FString InCount;
    if (!FParse::Token(InStream, InCount, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Warning: Missing Count parameter. Default count: 1"));
        InCount = L"1";
    }

    for (auto player_controller : target_players)
    {
        UFortItemDefinition* TargetItemDefinition = FindObject<UFortItemDefinition>(InWID.c_str(), false, true);

        if (TargetItemDefinition == nullptr)
        {
            ExecutingPlayerController->ClientMessage(TEXT("Couldn't find the specified item."));
            return;
        }

        std::wstring wInCount = InCount.c_str();
        int Count = std::stoi(wInCount);

        // Make sure the count is positive
        if (Count > 0)
        {
            // If the given player has space in their inventory, give the item, else spawn it as a pickup
            if (player_controller->WorldInventory() && player_controller->MyFortPawn())
            {
                if (inventory_component::is_inventory_full(player_controller))
                {
                    // Spawn a pickup
                    pickup_component::spawn_pickup(TargetItemDefinition, player_controller->MyFortPawn()->K2_GetActorLocation(), Count, looting_component::get_default_loaded_ammo(TargetItemDefinition));
                }
                else
                {
                    inventory_component::add_item(player_controller, TargetItemDefinition, Count, looting_component::get_default_loaded_ammo(TargetItemDefinition));
                }

                ExecutingPlayerController->ClientMessage(TEXT("Successfully granted item!"));
            }
        }
        else
        {
            ExecutingPlayerController->ClientMessage(TEXT("Invalid count."));
        }
    }
}

void moderation_component::HandleSetPlaylistCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    FString InPlaylistName;

    if (!FParse::Token(InStream, InPlaylistName, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing Playlist parameter."));
        return;
    }

    if (InPlaylistName.IsValid())
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

        std::string sInPlaylistName = converter.to_bytes(InPlaylistName.c_str());
        
        if (FindObject(InPlaylistName.c_str(), false, true) ||
            playlist_component::find_playlist(sInPlaylistName))
        {
            // Playlist validation
            if (UFortPlaylist* PlaylistObject = FindObject<UFortPlaylist>(InPlaylistName.c_str(), false, true))
            {
                if (!playlist_component::validate_playlist_for_private(PlaylistObject))
                {
                    ExecutingPlayerController->ClientMessage(TEXT("Error: The requested playlist is currently not available. Please contact a system administrator to request that this playlist be made available."));
                    return;
                }
            }

            // Set in config
            auto config = config_manager::get_server_config();
            config["override_playlist"] = sInPlaylistName;
            config_manager::save_server_config(config);

            // notify player
            ExecutingPlayerController->ClientMessage(TEXT("Successfully set playlist."));
        }
        else
        {
            ExecutingPlayerController->ClientMessage(TEXT("Invalid playlist."));
        }
    }
}   

void moderation_component::HandleToggleSCPCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    auto config = config_manager::get_server_config();
    bool scp_enabled = config["scp_enabled"];
    config["scp_enabled"] = !scp_enabled;
    config_manager::save_server_config(config);

    ExecutingPlayerController->ClientMessage(TEXT("Successfully toggled Server Choose Part."));
}

void moderation_component::HandleBanCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    FString InPlayerName;

    if (!FParse::Token(InStream, InPlayerName, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing Player parameter."));
        return;
    }

    if (InPlayerName.IsValid())
    {
        if (InPlayerName == L"@all")
        {
            ExecutingPlayerController->ClientMessage(TEXT("Unable to ban all players."));
        }
        
        if (AFortPlayerController* TargetPlayer = get_player_by_name(InPlayerName))
        {
            if (is_registered_admin(ExecutingPlayerController))
            {
                // Ban
                ban_player(TargetPlayer, L"Banned by an Admin.");
                
                // Get name of the executing PC as string
                const std::wstring AdminPlayerName = ExecutingPlayerController->MyPlayerInfo() ? ExecutingPlayerController->MyPlayerInfo()->PlayerName().c_str() : L"INVALID";
                std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
                std::string sAdminPlayerName = converter.to_bytes(AdminPlayerName);
                std::string sVictimPlayerName = converter.to_bytes(InPlayerName.c_str());
                std::string sServerName = qos_component::best_region;
                if (auto Client = gameserver_client::Client)
                {
                    sServerName = Client->GameServer_Status.name.length() >= 2 && Client->GameServer_Status.name.length() <= 16 ? Client->GameServer_Status.name : qos_component::best_region;
                }
                
                // Format message
                char Body[1024];
                sprintf_s(Body, "Player '%s' banned '%s' [SERVER: %s]", sAdminPlayerName.c_str(), sVictimPlayerName.c_str(), sServerName.c_str());
                std::string sBody(Body);

                // Get webhooks
                std::vector<nlohmann::json> payloads = webhook_manager::get_webhooks_by_name("webhook-ban");

                // Send webhooks
                for (auto payload : payloads)
                {
                    payload["body"] = Body;
                    webhook_manager::send_webhook(payload);
                }
                
                // Log
                ExecutingPlayerController->ClientMessage(TEXT("Successfully banned player."));
            }
        }
    }
}

void moderation_component::HandleSetServerNameCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    FString InServerName;
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

    if (!FParse::Token(InStream, InServerName, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing Server Name parameter."));
        return;
    }

    if (InServerName.IsValid())
    {
        std::string sInServerName = converter.to_bytes(InServerName.c_str());

        // length checks
        if (sInServerName.length() >= 2 && sInServerName.length() <= 16)
        {
            // Save the name in config
            auto config = config_manager::get_server_config();
            config["name"] = sInServerName;
            config_manager::save_server_config(config);

#ifdef _RELEASE
            if (gameserver_client* Client = gameserver_client::Client)
            {
                // Send the update to Era Services
                Client->GameServer_Status.name = sInServerName;
                Client->SendUpdate();

                SetWindowTextA(renderer::WindowHandle, std::string("FortMP - " + Client->GameServer_Status.name).c_str());
            }
#endif
            
            ExecutingPlayerController->ClientMessage(TEXT("The server name was successfully updated!"));
        }
        else
        {
            ExecutingPlayerController->ClientMessage(L"The server name has to be longer than 2 characters and shorted than 16.");
        }
    }
}

void moderation_component::HandleUnBanCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    FString InPlayerName;
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

    if (!FParse::Token(InStream, InPlayerName, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing Player parameter."));
        return;
    }

    if (InPlayerName.IsValid())
    {
        if (is_registered_admin(ExecutingPlayerController))
        {
            std::string sInPlayerName = converter.to_bytes(InPlayerName.c_str());
            
            bool bBanRecordExists = false;
            auto config = config_manager::get_server_config();
            for (auto& elem : config["banned_players"].items())
            {
                if (elem.value().is_object())
                {
                    for (auto& elem_ : elem.value().items())
                    {
                        if (std::string(elem_.key()) == sInPlayerName)
                        {
                            bBanRecordExists = true;
                            break;
                        }
                    }

                    if (bBanRecordExists)
                    {
                        break;
                    }
                }
            }

            if (bBanRecordExists)
            {
                // Perform unban
                unban_player(sInPlayerName);
                
                // Log
                ExecutingPlayerController->ClientMessage(TEXT("Successfully unbanned player."));
            }
            else
            {
                // Log
                ExecutingPlayerController->ClientMessage(TEXT("Couldn't find ban record for the specified player."));
            }
        }
    }
}

void moderation_component::HandleListPlayersCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    ExecutingPlayerController->ClientMessage(L"LIST OF PLAYERS:");
    
    if (*GWorld)
    {
        if (UNetDriver* NetDriver = (*GWorld)->NetDriver())
        {
            for (int i = 0; i < NetDriver->ClientConnections().Num(); i++)
            {
                UNetConnection* ClientConnection = NetDriver->ClientConnections()[i];

                if (ClientConnection)
                {
                    if (AFortPlayerController* PlayerController = ClientConnection->PlayerController())
                    {
                        if (UFortRegisteredPlayerInfo* MyPlayerInfo = PlayerController->MyPlayerInfo())
                        {
                            ExecutingPlayerController->ClientMessage(MyPlayerInfo->PlayerName().c_str());
                        }
                    }
                }
            }
        }
    }
}

void moderation_component::HandleBanListCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

    ExecutingPlayerController->ClientMessage(TEXT("LIST OF BANNED PLAYERS:"));
    
    auto config = config_manager::get_server_config();
    for (auto& elem : config["banned_players"].items())
    {
        if (elem.value().is_object())
        {
            for (auto& elem_ : elem.value().items())
            {
                auto wBannedPlayer = converter.from_bytes(std::string(elem_.key()).c_str());
                ExecutingPlayerController->ClientMessage(wBannedPlayer.c_str());
            }
        }
    }
}

void moderation_component::HandleSetWarmupDurationCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    FString InDuration;

    if (!FParse::Token(InStream, InDuration, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing Duration parameter."));
        return;
    }

    std::wstring wInDuration = InDuration.c_str();
    int Duration = std::stoi(wInDuration);

    if (Duration > 10)
    {
        auto config = config_manager::get_server_config();
        config["warmup_duration"] = Duration;
        config_manager::save_server_config(config);

        // Log
        ExecutingPlayerController->ClientMessage(TEXT("Successfully updated the warmup duration."));
    }
    else
    {
        ExecutingPlayerController->ClientMessage(TEXT("The minimum duration must be 10 seconds."));
    }
}

void moderation_component::HandleWipeInventoryCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    for (auto player_controller :  parse_command_target_players(InStream, ExecutingPlayerController))
    {
        inventory_component::clear_primary_inventory(player_controller, false);
    }
}

void moderation_component::HandleKillPlayerCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    for (auto player_controller :  parse_command_target_players(InStream, ExecutingPlayerController))
    {
        // Force suicide upon this player
        player_controller->ServerSuicide();
        
        ExecutingPlayerController->ClientMessage(TEXT("Successfully killed a player."));
    }
}

void moderation_component::HandleToggleKeepInventoryCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    elimination_component::keep_inventory_after_death = !elimination_component::keep_inventory_after_death;
    
    ExecutingPlayerController->ClientMessage(TEXT("Successfully toggled Keep Inventory Command."));
}

void moderation_component::HandleReviveCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    for (auto player_controller : parse_command_target_players(InStream, ExecutingPlayerController))
    {
        // Force revive a player
        reviving_component::revive_player(ExecutingPlayerController, player_controller->Pawn(), true);

        ExecutingPlayerController->ClientMessage(TEXT("Successfully revived a player."));
    }
}

void moderation_component::HandleToggleInfiniteAmmoCommand(const TCHAR*& InStream,
    AFortPlayerController* ExecutingPlayerController)
{
    UKismetSystemLibrary::SetBoolPropertyByName(ExecutingPlayerController, L"bInfiniteAmmo", !ExecutingPlayerController->bInfiniteAmmo());
    ExecutingPlayerController->ClientMessage(TEXT("Infinite ammo toggled."));
}

void moderation_component::HandleVanishCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    UKismetSystemLibrary::SetBoolPropertyByName(ExecutingPlayerController->Pawn(), L"bHidden", !ExecutingPlayerController->Pawn()->bHidden());
    ExecutingPlayerController->Pawn()->SetPawnVisibility(ExecutingPlayerController->Pawn()->bHidden(), true);
    ExecutingPlayerController->ClientMessage(TEXT("Vanish toggled."));
}

void moderation_component::HandleGodCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    for (auto player_controller :  parse_command_target_players(InStream, ExecutingPlayerController))
    {
        UKismetSystemLibrary::SetBoolPropertyByName(player_controller->Pawn(), L"bIsInvulnerable", !player_controller->Pawn()->bIsInvulnerable());
        ExecutingPlayerController->ClientMessage(TEXT("Success."));
    }
}

void moderation_component::HandleLaunchPlayerCommand(const TCHAR*& InStream,
    AFortPlayerController* ExecutingPlayerController)
{
    for (auto player_controller :  parse_command_target_players(InStream, ExecutingPlayerController))
    {
        player_controller->Pawn()->LaunchCharacterJump(FVector{0, 0, 15000}, false, false, true, true);
        ExecutingPlayerController->ClientMessage(TEXT("Success."));
    }
}

void moderation_component::HandleGiveWoodCommand(const TCHAR*& InStream,
    AFortPlayerController* ExecutingPlayerController)
{
    for (auto player_controller :  parse_command_target_players(InStream, ExecutingPlayerController))
    {
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"WoodItemData", false, true), 500);
        ExecutingPlayerController->ClientMessage(TEXT("Success."));
    }
}

void moderation_component::HandleGiveStoneCommand(const TCHAR*& InStream,
    AFortPlayerController* ExecutingPlayerController)
{
    for (auto player_controller :  parse_command_target_players(InStream, ExecutingPlayerController))
    {
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"StoneItemData", false, true), 500);
        ExecutingPlayerController->ClientMessage(TEXT("Success."));
    }
}

void moderation_component::HandleGiveMetalCommand(const TCHAR*& InStream,
    AFortPlayerController* ExecutingPlayerController)
{
    for (auto player_controller :  parse_command_target_players(InStream, ExecutingPlayerController))
    {
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"MetalItemData", false, true), 500);
        ExecutingPlayerController->ClientMessage(TEXT("Success."));
    }
}

void moderation_component::HandleGiveResourcesCommand(const TCHAR*& InStream,
    AFortPlayerController* ExecutingPlayerController)
{
    for (auto player_controller :  parse_command_target_players(InStream, ExecutingPlayerController))
    {
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"WoodItemData", false, true), 500);
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"StoneItemData", false, true), 500);
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"MetalItemData", false, true), 500);
        ExecutingPlayerController->ClientMessage(TEXT("Success."));
    }
}

TArray<AFortPlayerController*> moderation_component::parse_command_target_players(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    TArray<AFortPlayerController*> target_players;
    
    FString InPlayerName;
    if (!FParse::Token(InStream, InPlayerName, 0))
    {
        InPlayerName = L"@s";
    }

    if (AFortPlayerController* target_player = get_player_by_name(InPlayerName))
    {
        target_players.Add(target_player);
    }
    else if(InPlayerName == L"@all")
    {
        for (UNetConnection* net_connection : get_all_players())
        {
            target_players.Add(net_connection->PlayerController());
        }
    }
    else if(InPlayerName == L"@s")
    {
        target_players.Add(ExecutingPlayerController);
    }
    else
    {
        ExecutingPlayerController->ClientMessage(TEXT("Couldn't find the specified player."));
    }
    
    return target_players;
}
void moderation_component::HandleBadassCommand(const TCHAR*& InStream,
    AFortPlayerController* ExecutingPlayerController)
{
    for (auto player_controller : parse_command_target_players(InStream, ExecutingPlayerController))
    {
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"WID_Edged_Sword_Medium_SR_Ore_T04", false, true), 1);
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"WID_Harvest_Pickaxe_SR_T05", false, true), 1);
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"WID_Pistol_Bolt_SR_Ore_T04", false, true), 1, 40);
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"WID_Sniper_BoltAction_Scope_SR_Ore_T04", false, true), 1, 40);
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"WID_Shotgun_Auto_SR_Ore_T04", false, true), 1, 40);
        UKismetSystemLibrary::SetBoolPropertyByName(player_controller, L"bInfiniteAmmo", true);
        UKismetSystemLibrary::SetBoolPropertyByName(player_controller->Pawn(), L"bIsInvulnerable", true);
        UKismetSystemLibrary::SetBoolPropertyByName(player_controller->Pawn(), L"bInfiniteDurability", true);
        UKismetSystemLibrary::SetBoolPropertyByName(player_controller->Pawn(), L"bBuildFree", true);
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"WID_Assault_Auto_SR_Ore_T04", false, true), 1, 40);
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"WID_Edged_Scythe_Laser_SR_Ore_T04", false, true), 1, 40);
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"/Game/Missions/Secondary/BluGlo/BluGloWorldItemData.BluGloWorldItemData", true, false),10);
        
        ExecutingPlayerController->ClientMessage(TEXT("You're now a real badass."));
    }
}

void moderation_component::HandleGiveAmmoCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    for (auto player_controller : parse_command_target_players(InStream, ExecutingPlayerController))
    {
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"AmmoDataRockets", false, true), 50);
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"AthenaAmmoDataBulletsHeavy", false, true), 60);
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"AthenaAmmoDataBulletsLight", false, true), 500);
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"AthenaAmmoDataBulletsMedium", false, true), 350);
        inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"AthenaAmmoDataShells", false, true), 75);
        ExecutingPlayerController->ClientMessage(TEXT("Success."));
    }
}

void moderation_component::HandleSimulateBusJumpCommand(const TCHAR*& InStream,
    AFortPlayerController* ExecutingPlayerController)
{
    for (auto player_controller : parse_command_target_players(InStream, ExecutingPlayerController))
    {
        FVector bus_final_loc = player_controller->Pawn()->K2_GetActorLocation();
        bus_final_loc.Z = 20000;
        FRotator bus_final_rot = player_controller->Pawn()->K2_GetActorRotation();
        player_controller->Pawn()->K2_TeleportTo(bus_final_loc, bus_final_rot);
        player_controller->Pawn()->BeginSkydiving(true);

        player_controller->ClientPlaySound(FindObject<USoundBase>(L"/Game/Athena/Sounds/Bus/FlyingBus_Drop_Cue.FlyingBus_Drop_Cue", true, false), 1.0f, 1.0f);
        player_controller->ClientPlaySound(FindObject<USoundBase>(L"/Game/Athena/Sounds/Bus/FlyingBus_HornDoppler_Cue.FlyingBus_HornDoppler_Cue", true, false), 1.0f, 1.0f);
        
        ExecutingPlayerController->ClientMessage(TEXT("Success."));
    }
}

void moderation_component::HandleSpawnPickupCommand(const TCHAR*& InStream,
    AFortPlayerController* ExecutingPlayerController)
{
    TArray<AFortPlayerController*> target_players;

    FString InWID;
    if (!FParse::Token(InStream, InWID, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing Item parameter."));
        return;
    }

    FString InCount;
    if (!FParse::Token(InStream, InCount, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Warning: Missing Count parameter. Default count: 1"));
        InCount = L"1";
    }
    
    target_players.Add(ExecutingPlayerController);
        
    for (auto player_controller : target_players)
    {
        UFortItemDefinition* TargetItemDefinition = FindObject<UFortItemDefinition>(InWID.c_str(), false, true);

        if (TargetItemDefinition == nullptr)
        {
            ExecutingPlayerController->ClientMessage(TEXT("Couldn't find the specified item."));
            return;
        }

        std::wstring wInCount = InCount.c_str();
        int Count = std::stoi(wInCount);

        // Make sure the count is positive
        if (Count > 0)
        {
            // If the given player has space in their inventory, give the item, else spawn it as a pickup
            if (player_controller->WorldInventory() && player_controller->MyFortPawn())
            {
                // Spawn a pickup
                pickup_component::spawn_pickup(TargetItemDefinition, player_controller->MyFortPawn()->K2_GetActorLocation(), Count, looting_component::get_default_loaded_ammo(TargetItemDefinition));

                ExecutingPlayerController->ClientMessage(TEXT("Successfully spawned item!"));
            }
        }
        else
        {
            ExecutingPlayerController->ClientMessage(TEXT("Invalid count."));
        }
    }
}

void moderation_component::HandleSpawnChestCommand(const TCHAR*& InStream,
    AFortPlayerController* ExecutingPlayerController)
{
    FVector SpawnLocation = ExecutingPlayerController->Pawn()->K2_GetActorLocation();
    SpawnLocation.X += ExecutingPlayerController->Pawn()->GetActorForwardVector().X * 350;
    SpawnLocation.Y += ExecutingPlayerController->Pawn()->GetActorForwardVector().Y * 350;
    SpawnLocation.Z += ExecutingPlayerController->Pawn()->GetActorForwardVector().Z * 100;
        
    FRotator SpawnRotation = FRotator();
    FActorSpawnParameters SpawnParameters{};
    if (UObject* SpawnedActor = SpawnActor(*GWorld, FindObject(L"/Game/Building/ActorBlueprints/Containers/Tiered_Chest_6_Parent.Tiered_Chest_6_Parent_C"), &SpawnLocation, &SpawnRotation, SpawnParameters))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Success."));
    }
}

void moderation_component::HandleChangeScaleCommand(const TCHAR*& InStream,
    AFortPlayerController* ExecutingPlayerController)
{
    TArray<AFortPlayerController*> target_players;
    
    FString InPlayerScale;
    
    if (!FParse::Token(InStream, InPlayerScale, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing scale parameter."));
        return;
    }
    std::wstring wPlayerScale = InPlayerScale.c_str();
    float fNewScaleValue = std::stof(wPlayerScale);

    FVector new_scale = FVector(fNewScaleValue, fNewScaleValue, fNewScaleValue);
    
    if (ExecutingPlayerController->Pawn())
    {
        ExecutingPlayerController->Pawn()->SetActorScale3D(new_scale);
    }
}

void moderation_component::HandleSummonCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    FString InActorClass;
    if (!FParse::Token(InStream, InActorClass, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing class parameter."));
        return;
    }
    
    if (auto actor_class = FindObject(InActorClass.c_str(), true, false))
    {
        if(actor_class->IsA(AActor::StaticClass()))
        {
            FVector SpawnLocation = ExecutingPlayerController->Pawn()->K2_GetActorLocation();
            SpawnLocation.X += ExecutingPlayerController->Pawn()->GetActorForwardVector().X * 500;
            SpawnLocation.Y += ExecutingPlayerController->Pawn()->GetActorForwardVector().Y * 500;
            SpawnLocation.Z += ExecutingPlayerController->Pawn()->GetActorForwardVector().Z * 500;
        
            FRotator SpawnRotation = FRotator();
            FActorSpawnParameters SpawnParameters{};
            if (UObject* SpawnedActor = SpawnActor(*GWorld, actor_class, &SpawnLocation, &SpawnRotation, SpawnParameters))
            {
                ExecutingPlayerController->ClientMessage(TEXT("Successfully summoned an actor!"));
            }   
        }
    }
    else
    {
        ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Invalid class, make sure to enter full path. Example: /Game/Blueprints/BluGloManager.BluGloManager_C"));
    }
}

void moderation_component::HandleSpawnDefaultBotCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    FVector SpawnLocation = ExecutingPlayerController->Pawn()->K2_GetActorLocation();
    SpawnLocation.X += ExecutingPlayerController->Pawn()->GetActorForwardVector().X * 600;
    SpawnLocation.Y += ExecutingPlayerController->Pawn()->GetActorForwardVector().Y * 600;
    SpawnLocation.Z += ExecutingPlayerController->Pawn()->GetActorForwardVector().Z * 600;
        
    FRotator SpawnRotation = FRotator();
    FTransform SpawnTransform = FTransform();
    SpawnTransform.Translation = SpawnLocation;
    
    FActorSpawnParameters SpawnParameters{};
    
    AAthena_PlayerController_C* bot_pc = static_cast<AAthena_PlayerController_C*>(SpawnDefaultActor(*GWorld, AAthena_PlayerController_C::StaticClass()));
    
    if (APlayerPawn_Athena_C* bot_pawn = static_cast<APlayerPawn_Athena_C*>(SpawnActor(*GWorld, (*GWorld)->AuthorityGameMode()->DefaultPawnClass(), &SpawnLocation, &SpawnRotation, SpawnParameters)))
    {
        bot_pawn->Controller() = bot_pc;
        
        if (AFortPlayerStateAthena* bot_state = static_cast<AFortPlayerStateAthena*>(SpawnDefaultActor(*GWorld, AFortPlayerStateAthena::StaticClass())))
        {
            if(!bot_pawn->PlayerState())
            {
                bot_pc->PlayerState() = bot_state;
                bot_state->WorldPlayerId() = player_component::global_player_id++;
                bot_pawn->PlayerState() = bot_state;
                bot_pc->WorldInventory() = SpawnDefaultActor<AFortInventory>(*GWorld, AFortInventory::StaticClass());
                bot_pc->Possess(bot_pawn);
                bot_state->ApplyCharacterCustomization(bot_pawn);
            }
        }

        ExecutingPlayerController->ClientMessage(TEXT("moderation_component: Spawned Bot"));
    }   
}

AFortPlayerController* moderation_component_hooks::Login_Hook(AFortGameModeAthena* Self, UPlayer* NewPlayer, int InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueID, FString& ErrorMessage)
{
    if (player_component::Login(UniqueID, Options))
    {
        AFortPlayerController* NewPlayerController = Login(Self, NewPlayer, InRemoteRole, Portal, Options, UniqueID, ErrorMessage);

#ifdef _RELEASE
        if (NewPlayerController)
        {
           player_component::Setup(NewPlayerController, Options);
           team_component::Login(NewPlayerController, Options);
        }
#endif

        return NewPlayerController;
    }

    // If FortMP Login fails, return
    return nullptr;
}

void moderation_component_hooks::ServerCheat_Hook(AFortPlayerController* Self, FFrame& StackFrame)
{
    GET_PARAMETER(FString, Msg);

    const TCHAR* Stream = Msg.c_str();

    if (Msg.IsValid())
    {
        // Commands for non-admins
        if (Self->CheatManager() == nullptr)
        {
            if (FParse::Command(&Stream, TEXT("ADMINLOGIN")))
            {
                moderation_component::HandleAdminLoginCommand(Stream, Self);
            }
        }
        
        // Only allow admins to execute commands on server
        if (Self->CheatManager())
        {
            if (FParse::Command(&Stream, TEXT("KICKPLAYER")))
            {
                moderation_component::HandleKickPlayerCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("HELP")))
            {
                moderation_component::HandleHelpCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("STARTMATCH")))
            {
                moderation_component::HandleStartMatchCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("RESTARTSERVER")))
            {
                moderation_component::HandleRestartServerCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("RESETSERVER")))
            {
                moderation_component::HandleResetServerCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("SAFERESETSERVER")))
            {
                moderation_component::HandleSafeResetServerCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("SETSERVERPASSWORD")))
            {
                moderation_component::HandleSetServerPasswordCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("OP")))
            {
                moderation_component::HandleOpCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("DEOP")))
            {
                moderation_component::HandleDeopCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("FLY")))
            {
                moderation_component::HandleFlyCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("SETSPEED")))
            {
                moderation_component::HandleSetSpeedCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("GIVEITEM")))
            {
                moderation_component::HandleGiveItemCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("SETPLAYLIST")))
            {
                moderation_component::HandleSetPlaylistCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("TOGGLESERVERCHOOSEPART")))
            {
                moderation_component::HandleToggleSCPCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("BAN")))
            {
                moderation_component::HandleBanCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("SETSERVERNAME")))
            {
                moderation_component::HandleSetServerNameCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("UNBAN")))
            {
                moderation_component::HandleUnBanCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("LISTPLAYERS")))
            {
                moderation_component::HandleListPlayersCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("BANLIST")))
            {
                moderation_component::HandleBanListCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("SETWARMUPDURATION")))
            {
                moderation_component::HandleSetWarmupDurationCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("WIPEINVENTORY")))
            {
                moderation_component::HandleWipeInventoryCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("KILL")))
            {
                moderation_component::HandleKillPlayerCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("TOGGLEKEEPINVENTORY")))
            {
                moderation_component::HandleToggleKeepInventoryCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("REVIVE")))
            {
                moderation_component::HandleReviveCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("INFINITEAMMO")))
            {
                moderation_component::HandleToggleInfiniteAmmoCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("VANISH")))
            {
                moderation_component::HandleVanishCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("GOD")))
            {
                moderation_component::HandleGodCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("LAUNCHPLAYER")))
            {
                moderation_component::HandleLaunchPlayerCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("GIVEWOOD")))
            {
                moderation_component::HandleGiveWoodCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("GIVERESOURCES")))
            {
                moderation_component::HandleGiveResourcesCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("BADASS")))
            {
                moderation_component::HandleBadassCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("GIVESTONE")))
            {
                moderation_component::HandleGiveStoneCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("GIVEMETAL")))
            {
                moderation_component::HandleGiveMetalCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("GIVEAMMO")))
            {
                moderation_component::HandleGiveAmmoCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("SIMULATEBUSJUMP")))
            {
                moderation_component::HandleSimulateBusJumpCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("SPAWNPICKUP")))
            {
                moderation_component::HandleSpawnPickupCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("SPAWNCHEST")))
            {
                moderation_component::HandleSpawnChestCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("CHANGESCALE")))
            {
                moderation_component::HandleChangeScaleCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("LOADSUMMON")))
            {
                moderation_component::HandleSummonCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("SPAWNDEFAULTBOT")))
            {
                moderation_component::HandleSpawnDefaultBotCommand(Stream, Self);
            }
            else
            {
                UKismetSystemLibrary::ExecuteConsoleCommand(Msg, Self);
            }
        }
    }
}


