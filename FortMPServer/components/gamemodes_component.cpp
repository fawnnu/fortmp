#include "include.hpp"
#include "gamemodes_component.hpp"

#include "function_component.hpp"

gamemodes_component::gamemodes_component()
{
    // Register UFunctions
    REGISTER_CUSTOM_UFUNCTION(OnServerReady, AFortGameModeAthena::StaticClass());

    REGISTER_CUSTOM_UFUNCTION_WITH_PARAMS(OnPlayerJoined, AFortGameModeAthena::StaticClass(), 0x8);
}

void gamemodes_component::OnServerReady(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();
}

void gamemodes_component::OnPlayerJoined(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();   
}