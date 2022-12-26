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

    // Only allow admins to execute commands on server
    if (Self->CheatManager())
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(Msg);
    }
}


