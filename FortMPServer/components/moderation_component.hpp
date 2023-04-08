#pragma once

class moderation_component
{
public:
    moderation_component();

    // get player by username
    static AFortPlayerController* get_player_by_name(FString InPlayerName);

    // Check if the player's IP is marked as Administrator, if so, grant admin permission
    static void grant_admin_permission_safe(AFortPlayerController* JoiningPlayer);

    // Finds whether or not the given player is a registered server admin
    static bool is_registered_admin(AFortPlayerController* Target);

    // Perform a ban
    static void ban_player(AFortPlayerController* Target, FString BanReason);
    static void unban_player(std::string InPlayerName);
    static bool is_player_banned(AFortPlayerController* Target);

    // Commands
    static void HandleAdminLoginCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleKickPlayerCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleHelpCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleStartMatchCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleRestartServerCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleResetServerCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleSafeResetServerCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleSetServerPasswordCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleOpCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleDeopCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleFlyCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleSetSpeedCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleGiveItemCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleSetPlaylistCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleToggleSCPCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleBanCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleSetServerNameCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleUnBanCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleListPlayersCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleBanListCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);
    static void HandleSetWarmupDurationCommand(const TCHAR*& InStream, AFortPlayerController* ExecutingPlayerController);

    // Password that admins will use to log-in 
    inline static std::vector<std::wstring> passwords;
};

namespace moderation_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, ServerCheat, (AFortPlayerController* Self, FFrame& StackFrame));
    DEFINE_FUNCTION_HOOK(AFortPlayerController*, Login, (AFortGameModeAthena* Self, UPlayer* NewPlayer, int InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueID, FString& ErrorMessage));
}
