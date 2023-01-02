#pragma once

namespace FortMpGlobals
{
    // Override playlist specified with console commands by Server Owners
    inline UFortPlaylistAthena* OverridePlaylist;

    // Server key required to join a match, if specified by Server Owners
    inline FString ServerKey;

    // Specifies if whitelist is enabled
    inline bool bIsWhitelistEnabled;

    // List of registered server admins
    inline std::vector<AFortPlayerController*> ServerAdmins;
}