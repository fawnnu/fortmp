#pragma once

class gamemodes_component
{
public:
    gamemodes_component();
    
    static void OnServerReady(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame);

    static void OnPlayerJoined(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame);

    static void OnUpdateStorm(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame, void* ReturnValue);

    static void OnSetPlaylist(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame, void* ReturnValue);

    static void OnEnterAircraft(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame);

    static void OnExitAircraft(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame);
    
    static void SetSafeZoneStartDelay_Value(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame);

    static void GiveItemToPlayer(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame);

    static void OnPawnFinishedSpawning(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame);

    static void OnServerMapInfoReady(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame);
    
    static void GetRandomLootPackageData(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame, void* ReturnValue);

    static void SetShield(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame);
};  
