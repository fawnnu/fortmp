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
    
    REGISTER_CUSTOM_UFUNCTION(OnSetPlaylist, AFortGameModeAthena::StaticClass());
    INIT_CUSTOM_FUNC_RET_OFFSET(OnSetPlaylist, AFortGameModeAthena::StaticClass(), 0x0);

    REGISTER_CUSTOM_UFUNCTION(SetSafeZoneStartDelay_Value, AFortGameModeAthena::StaticClass());
    INIT_CUSTOM_FUNC_PARAMS(SetSafeZoneStartDelay_Value, AFortGameModeAthena::StaticClass(), 0x4);
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

void gamemodes_component::OnSetPlaylist(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame, void* ReturnValue)
{
    INIT_CUSTOM_FUNCTION();

    // Unless a Blueprint implements this function, we will return false
    *reinterpret_cast<UObject**>(ReturnValue) = nullptr;
}

void gamemodes_component::SetSafeZoneStartDelay_Value(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame)
{
    GET_CUSTOM_FUNCTION_BP_PARAM(float, InValue);
    
    INIT_CUSTOM_FUNCTION();

    (*GWorld)->GameState()->MapInfo()->SafeZoneStartDelay().Value() = InValue;
}

