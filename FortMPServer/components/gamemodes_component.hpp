#pragma once

class gamemodes_component
{
public:
    gamemodes_component();
    
    static void OnServerReady(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame);

    static void OnPlayerJoined(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame);
};
