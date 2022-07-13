#include "include.hpp"
#include "storm_component.hpp"

storm_component::storm_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(storm_component_hooks::SetSafezoneValues, memory_manager::find_function_by_string(L"FortGameModeAthena: No MegaStorm on SafeZone[%d]."), storm_component_hooks::SetSafezoneValues_Hook);
    
    DetourTransactionCommit();
}

// TODO: don't hardcode
void storm_component_hooks::SetSafezoneValues_Hook(AFortGameModeAthena* FortGameModeAthena, int unk)
{
    // call function at first
    SetSafezoneValues(FortGameModeAthena, unk);

    float WorldTimeSeconds = (*GWorld)->GameState()->ReplicatedWorldTimeSeconds();
    FortGameModeAthena->SafeZoneIndicator()->SafeZoneStartShrinkTime() = WorldTimeSeconds + 60.f;
    FortGameModeAthena->SafeZoneIndicator()->SafeZoneFinishShrinkTime() = FortGameModeAthena->SafeZoneIndicator()->SafeZoneStartShrinkTime() + 40.f;
}


