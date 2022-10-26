#pragma once

class gamemodes_component
{
public:
    gamemodes_component();
    
    // OnServerReady implementation
    static void OnServerReady(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame);
};
