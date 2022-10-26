#include "include.hpp"
#include "gamemodes_component.hpp"

#include "function_component.hpp"

gamemodes_component::gamemodes_component()
{
    // Register UFunction
    REGISTER_CUSTOM_UFUNCTION(OnServerReady, AFortGameModeAthena::StaticClass());
}

void gamemodes_component::OnServerReady(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame)
{
    
}