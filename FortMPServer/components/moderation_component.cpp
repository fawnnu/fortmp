#include "include.hpp"
#include "moderation_component.hpp"
#include "team_component.hpp"

moderation_component::moderation_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(moderation_component_hooks::Login, memory_manager::find_function_by_string(L"Login: Couldn't spawn player controller of class %s"), moderation_component_hooks::Login_Hook);    
    
    DetourTransactionCommit();
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
    }

    return NewPlayerController;
}

