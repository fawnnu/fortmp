#pragma once

class aircraft_component
{
public:
    aircraft_component();
};

namespace aircraft_component_hooks
{
    // Called when a player tries to exit the aircraft
    DEFINE_FUNCTION_HOOK(void, ServerAttemptAircraftJump, (AFortPlayerController* FortPlayerController, FFrame& StackFrame));

    // Called when the aircraft exits the drop zone
    DEFINE_FUNCTION_HOOK(void, OnAircraftExitedDropZone, (AFortGameModeAthena* GameMode, FFrame& StackFrame));

    // Called when a player enters the aircraft
    DEFINE_FUNCTION_HOOK(void, EnterAircraft, (AFortPlayerController* PlayerController, void* unknown_data));
};

// Used to choose a player start for the player exiting the aircraft
inline UObject* (*ChoosePlayerStart_Aircraft)(AFortGameModeAthena* GameMode, AFortPlayerController* PlayerController);