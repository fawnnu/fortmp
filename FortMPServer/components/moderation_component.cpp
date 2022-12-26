#include "include.hpp"
#include "moderation_component.hpp"
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

    if (AFortPlayerController* TargetPlayerController = get_player_by_name(PlayerName))
    {
        TargetPlayerController->ClientReturnToMainMenu(OptionalKickReason);
    }
    else
    {
        ExecutingPlayerController->ClientMessage(TEXT("Specified player could not be found."));
    }
}

void moderation_component::HandleHelpCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController)
{
    // Log everything to the player's console
    ExecutingPlayerController->ClientMessage(TEXT("--- COMMAND HELP LIST ---\ncheat kickplayer <player name> <optional kick reason> (Kicks a player with an optional reason)\n\nNOTE: command arguments have to be passed with \" \" if they have a space!\nExample: cheat kickplayer Owner \"Hacker, DM for appeal\""));
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

        if (UGameplayStatics::HasOption(Options, TEXT("Password")) && UGameplayStatics::HasOption(Options, TEXT("Name")))
        {
            FString UserPassword = UGameplayStatics::ParseOption(Options, TEXT("Password"));
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
        else
        {
            UKismetSystemLibrary::ExecuteConsoleCommand(Msg);
        }
    }
}


