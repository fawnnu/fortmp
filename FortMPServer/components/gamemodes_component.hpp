#pragma once

class gamemodes_component
{
public:
    gamemodes_component();
    
    static void OnServerReady(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame);

    static void OnPlayerJoined(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame);

    static void OnUpdateStorm(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame, void* ReturnValue);

    static void OnSetPlaylist(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame, void* ReturnValue);
    
    static void SetSafeZoneStartDelay_Value(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame);
};
