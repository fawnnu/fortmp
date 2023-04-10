#pragma once

class aircraft_component
{
public:
    aircraft_component();

    static void exit_aircraft(AFortPlayerController* FortPlayerController, FRotator ClientRotation);
};

namespace aircraft_component_hooks
{
    // Called when a player tries to exit the aircraft
    DEFINE_FUNCTION_HOOK(void, UFortControllerComponent_Aircraft_ServerAttemptAircraftJump, (UFortControllerComponent_Aircraft* AircraftComponent, FFrame& StackFrame));

    // Native function called when a player enters the aircraft. Exists on any build
    DEFINE_FUNCTION_HOOK(void, EnterAircraft, (UObject*, AFortAircraft* FortAircraft));
    
    // Called when the aircraft exits the drop zone
    DEFINE_FUNCTION_HOOK(void, OnAircraftExitedDropZone, (AFortGameModeAthena* GameMode, FFrame& StackFrame));
};