#include "include.hpp"
#include "moderation_component.hpp"

#include "config_manager.hpp"
#include "inventory_component.hpp"
#include "looting_component.hpp"
#include "net_component.hpp"
#include "pickup_component.hpp"
#include "team_component.hpp"
#include "../utils/webhook_manager.hpp"

moderation_component::moderation_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(moderation_component_hooks::Login, memory_manager::find_function_by_string(L"Login: Couldn't spawn player controller of class %s"), moderation_component_hooks::Login_Hook);    
    REGISTER_UFUNCTION_DETOUR(moderation_component_hooks::ServerCheat, L"/Script/FortniteGame.FortPlayerController.ServerCheat", moderation_component_hooks::ServerCheat_Hook);
    
    DetourTransactionCommit();

    // initialize password
    passwords = {
        TEXT("k9aJt6a6g2w3gz1l"),
        TEXT("y8seRL4Ji63fh7H6"),
        TEXT("84H9LZI8vAU4OwLl"),
        TEXT("eVd2Nv66ZIiljlfY"),
        TEXT("oEiU2JYp8zNL46Xg")
    };
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
        if (AFortPlayerController* TargetPlayerController = get_player_by_name(PlayerName))
        {
            if (OptionalKickReason.IsValid())
            {
                TargetPlayerController->ClientReturnToMainMenu(OptionalKickReason);
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
    ExecutingPlayerController->ClientMessage(TEXT("--- COMMAND HELP LIST ---\ncheat kickplayer <player name> <optional kick reason> (Kicks a player with an optional reason)\ncheat startmatch (Starts the bus countdown)\ncheat restartserver (Restarts the server)\ncheat setserverpassword <serverpassword> (Sets a server password)\ncheat togglewhitelist (Toggles a whitelist, if on, players will have to join with ?serverpassword=<serverpassword>)\ncheat op <player> (Gives access to admin commands to a given player)\ncheat deop <deop> (Revokes access to admin commands, doesn't work on players who join with ?adminpassword)\ncheat fly (Toggles fly)\ncheat giveitem <playername> <item wid> <item count> (Gives an item to a player, if the inventory is full, spawns a pickup)\n\nNOTE: command arguments have to be passed with \" \" if they have a space!\nExample: cheat kickplayer Owner \"Hacker, DM for appeal\""));
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

                    // Close connection anyways to prevent bypasses
                    NetConnection__Close(ClientConnection);
                }
            }
        }
    }
}

void moderation_component::HandleSetServerPasswordCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    FString InServerPassword;

    if (!FParse::Token(InStream, InServerPassword, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing InServerPassword parameter."));
        return;
    }

    if (InServerPassword.IsValid())
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        
        std::string sInServerPassword = converter.to_bytes(InServerPassword.c_str());
        
        // Set a password
        auto config = config_manager::get_server_config();
        config["server_key"] = sInServerPassword;
        config_manager::save_server_config(config);

        ExecutingPlayerController->ClientMessage(TEXT("ServerPassword successfully set!"));
    }
}

void moderation_component::HandleToggleWhitelistCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    auto config = config_manager::get_server_config();
    config["bIsWhitelistEnabled"] = !bool(config["bIsWhitelistEnabled"]);
    config_manager::save_server_config(config);

    ExecutingPlayerController->ClientMessage(bool(config["bIsWhitelistEnabled"]) ? TEXT("Whitelist Enabled!") : TEXT("Whitelist Disabled!"));
}

void moderation_component::HandleOpCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    // OP a player (Give them cheatmanager, without registering them as admin)
    FString TargetPlayerName;

    if (!FParse::Token(InStream, TargetPlayerName, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing Player parameter."));
        return;
    }

    if (TargetPlayerName.IsValid())
    {
        if (AFortPlayerController* TargetPlayer = get_player_by_name(TargetPlayerName))
        {
            if (TargetPlayer == nullptr)
            {
                ExecutingPlayerController->ClientMessage(TEXT("Couldn't find the specified player."));
                return;
            }

            if (TargetPlayer->CheatManager() != nullptr)
            {
                ExecutingPlayerController->ClientMessage(TEXT("The specified player is already an admin!"));
                return;
            }

            // Op!
            TargetPlayer->CheatManager() = static_cast<UCheatManager*>(UGameplayStatics::SpawnObject(UCheatManager::StaticClass(), TargetPlayer));

            ExecutingPlayerController->ClientMessage(TEXT("Successfully opped player!"));
        }
    }
}

void moderation_component::HandleDeopCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    // OP a player (Give them cheatmanager, without registering them as admin)
    FString TargetPlayerName;

    if (!FParse::Token(InStream, TargetPlayerName, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing Player parameter."));
        return;
    }

    if (TargetPlayerName.IsValid())
    {
        if (AFortPlayerController* TargetPlayer = get_player_by_name(TargetPlayerName))
        {
            if (TargetPlayer == nullptr)
            {
                ExecutingPlayerController->ClientMessage(TEXT("Couldn't find the specified player."));
                return;
            }

            if (TargetPlayer->CheatManager() == nullptr)
            {
                ExecutingPlayerController->ClientMessage(TEXT("The specified player is not opped!"));
                return;
            }

            // Get the IP as std::string
            std::string TargetPlayerIP =static_cast<UIpConnection*>(TargetPlayer->NetConnection())->RemoteAddr().Get()->ToString();

            // Make sure the player is not an admin
            for (auto& elem : config_manager::get_server_config()["server_admins"].items())
            {
                if (std::string(elem.value()) == TargetPlayerIP)
                {
                    ExecutingPlayerController->ClientMessage(TEXT("You can't deop a server admin!"));
                    return;
                }
            }

            // If the player is not a registered admin, de-op it
            TargetPlayer->CheatManager() = nullptr;

            ExecutingPlayerController->ClientMessage(TEXT("Successfully deopped player!"));
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
        static float DefaultRunValue = MyFortPawn->MovementSet()->RunSpeed().CurrentValue();

        // Set the movement mode
        MyFortPawn->CharacterMovement()->MovementMode() = MyFortPawn->CharacterMovement()->MovementMode() == MOVE_Flying ? MOVE_Walking : MOVE_Flying;

        // Toggle bCheatFlying
        UKismetSystemLibrary::SetBoolPropertyByName(MyFortPawn->CharacterMovement(), TEXT("bCheatFlying"), !MyFortPawn->CharacterMovement()->bCheatFlying());

        // Change speed
        if (MyFortPawn->CharacterMovement()->MovementMode() == MOVE_Flying)
        {
            MyFortPawn->MovementSet()->RunSpeed().CurrentValue() = 1600.f;
        }
        else if (MyFortPawn->CharacterMovement()->MovementMode() == MOVE_Walking)
        {
            MyFortPawn->MovementSet()->RunSpeed().CurrentValue() = DefaultRunValue;
        }

        ExecutingPlayerController->ClientMessage(TEXT("Toggled fly!"));
    }
}

void moderation_component::HandleGiveItemCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    FString InPlayerName;

    if (!FParse::Token(InStream, InPlayerName, 0))
    {
        ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing Player parameter."));
        return;
    }

    if (AFortPlayerController* TargetPlayer = get_player_by_name(InPlayerName))
    {
        FString InWID;

        if (!FParse::Token(InStream, InWID, 0))
        {
            ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing Item parameter."));
            return;
        }

        UFortItemDefinition* TargetItemDefinition = FindObject<UFortItemDefinition>(InWID.c_str(), false, true);

        if (TargetItemDefinition == nullptr)
        {
            ExecutingPlayerController->ClientMessage(TEXT("Couldn't find the specified item."));
            return;
        }

        FString InCount;

        if (!FParse::Token(InStream, InCount, 0))
        {
            ExecutingPlayerController->ClientMessage(TEXT("Syntax error: Missing Count parameter."));
            return;
        }

        std::wstring wInCount = InCount.c_str();
        int Count = std::stoi(wInCount);

        // Make sure the count is positive
        if (Count > 0)
        {
            // If the given player has space in their inventory, give the item, else spawn it as a pickup
            if (TargetPlayer->WorldInventory() && TargetPlayer->MyFortPawn())
            {
                if (inventory_component::is_inventory_full(TargetPlayer))
                {
                    // Spawn a pickup
                    pickup_component::spawn_pickup(TargetItemDefinition, TargetPlayer->MyFortPawn()->K2_GetActorLocation(), Count, looting_component::get_default_loaded_ammo(TargetItemDefinition));
                }
                else
                {
                    inventory_component::add_item(TargetPlayer, TargetItemDefinition, Count, looting_component::get_default_loaded_ammo(TargetItemDefinition));
                }

                ExecutingPlayerController->ClientMessage(TEXT("Successfully granted item!"));
            }
        }
        else
        {
            ExecutingPlayerController->ClientMessage(TEXT("Invalid count."));
        }
    }
    else
    {
        ExecutingPlayerController->ClientMessage(TEXT("Couldn't find the specified player."));
    }
}

AFortPlayerController* moderation_component_hooks::Login_Hook(AFortGameModeAthena* Self, UPlayer* NewPlayer, int InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueID, FString& ErrorMessage)
{
    AFortPlayerController* NewPlayerController = Login(Self, NewPlayer, InRemoteRole, Portal, Options, UniqueID, ErrorMessage);

    if (NewPlayerController)
    {
        if (UGameplayStatics::HasOption(Options, TEXT("TeamCode")))
        {
            // Get the team code value
            FString TeamCode = UGameplayStatics::ParseOption(Options, TEXT("TeamCode"));

            // If the team code is valid
            if (TeamCode.IsValid())
            {
                // Add every player controller to the given team code key
                team_component::team_codes_map[TeamCode.c_str()].push_back(NewPlayerController);
            }
        }

        // If whitelist is enabled, only let allowed users to join
        if (bool(config_manager::get_server_config()["bIsWhitelistEnabled"]))
        {
            if (UGameplayStatics::HasOption(Options, TEXT("ServerPassword")))
            {
                FString UserPassword = UGameplayStatics::ParseOption(Options, TEXT("ServerPassword"));

                std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        
                std::string sUserPassword = converter.to_bytes(UserPassword.c_str());

                if (sUserPassword != std::string(config_manager::get_server_config()["server_key"]))
                {
                    // Don't let the user login
                    return nullptr;
                }
            }
            else
            {
                return nullptr;
            }
        }

        if (UGameplayStatics::HasOption(Options, TEXT("AdminPassword")) && UGameplayStatics::HasOption(Options, TEXT("Name")))
        {
            FString UserPassword = UGameplayStatics::ParseOption(Options, TEXT("AdminPassword"));
            FString UserName = UGameplayStatics::ParseOption(Options, TEXT("Name"));

            if (UserPassword.IsValid() && UserName.IsValid())
            {
                for (FString& password_to_check : moderation_component::passwords)
                {
                    if (password_to_check == UserPassword)
                    {
                        // user is now logged in as administrator, give them cheatmanager
                        NewPlayerController->CheatManager() = static_cast<UCheatManager*>(UGameplayStatics::SpawnObject(UCheatManager::StaticClass(), NewPlayerController));

                        // Get admin player info
                        const std::wstring AdminPlayerName = UserName.c_str();
                        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
                        std::string sAdminPlayerName = converter.to_bytes(AdminPlayerName);
                        std::string sAdminIPAddress = static_cast<UIpConnection*>(NewPlayer)->RemoteAddr().Get()->ToString();

                        // Cache IP
                        auto config = config_manager::get_server_config();
                        config["server_admins"].push_back(sAdminIPAddress);
                        config_manager::save_server_config(config);
                        
                        // Format message
                        char Body[1024];
                        sprintf_s(Body, "Player '%s' [%s] logged in as administrator.", sAdminPlayerName.c_str(), sAdminIPAddress.c_str());
                        std::string sBody(Body);

                        // Get webhook
                        nlohmann::json payload = webhook_manager::get_webhook_by_name("webhook-adminlogin");
                        payload["body"] = Body;

                        // Send webhook
                        webhook_manager::send_webhook(payload);
                        
                        break;
                    }
                }
            }
        }
    }

    return NewPlayerController;
}

void moderation_component_hooks::ServerCheat_Hook(AFortPlayerController* Self, FFrame& StackFrame)
{
    GET_PARAMETER(FString, Msg);

    const TCHAR* Stream = Msg.c_str();

    if (Msg.IsValid())
    {
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
            else if (FParse::Command(&Stream, TEXT("SETSERVERPASSWORD")))
            {
                moderation_component::HandleSetServerPasswordCommand(Stream, Self);
            }
            else if (FParse::Command(&Stream, TEXT("TOGGLEWHITELIST")))
            {
                moderation_component::HandleToggleWhitelistCommand(Stream, Self);
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
            else if (FParse::Command(&Stream, TEXT("GIVEITEM")))
            {
                moderation_component::HandleGiveItemCommand(Stream, Self);
            }
            else
            {
                UKismetSystemLibrary::ExecuteConsoleCommand(Msg, Self);
            }
        }
    }
}


