#include "include.hpp"
#include "moderation_component.hpp"

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

    return NewPlayerController;
}

