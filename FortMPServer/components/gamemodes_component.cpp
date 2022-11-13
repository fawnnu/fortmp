#include "include.hpp"
#include "gamemodes_component.hpp"

#include "function_component.hpp"

gamemodes_component::gamemodes_component()
{
    // Register UFunctions
    REGISTER_CUSTOM_UFUNCTION(OnServerReady, AFortGameModeAthena::StaticClass());

    REGISTER_CUSTOM_UFUNCTION(OnPlayerJoined, AFortGameModeAthena::StaticClass());
    INIT_CUSTOM_FUNC_PARAMS(OnPlayerJoined, AFortGameModeAthena::StaticClass(), 0x8);

    REGISTER_CUSTOM_UFUNCTION(OnUpdateStorm, AFortGameModeAthena::StaticClass());
    INIT_CUSTOM_FUNC_RET_OFFSET(OnUpdateStorm, AFortGameModeAthena::StaticClass(), 0x0);
}

void gamemodes_component::OnServerReady(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();
}

void gamemodes_component::OnPlayerJoined(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();   
}

void gamemodes_component::OnUpdateStorm(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame, void* ReturnValue)
{
    INIT_CUSTOM_FUNCTION();

    // Unless a Blueprint implements this function, we will return false
    *reinterpret_cast<bool*>(ReturnValue) = false;
}
