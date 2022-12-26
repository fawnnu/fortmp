#pragma once

class moderation_component
{
public:
    moderation_component();

    // get player by username
    static AFortPlayerController* get_player_by_name(FString InPlayerName);

    // Commands
    static void HandleKickPlayerCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleHelpCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);

    // Password that admins will use to log-in 
    inline static std::vector<FString> passwords;
};

namespace moderation_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, ServerCheat, (AFortPlayerController* Self, FFrame& StackFrame));
    DEFINE_FUNCTION_HOOK(AFortPlayerController*, Login, (AFortGameModeAthena* Self, UPlayer* NewPlayer, int InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueID, FString& ErrorMessage));
}
