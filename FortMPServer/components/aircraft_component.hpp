#pragma once

class aircraft_component
{
public:
    aircraft_component();

    static void exit_aircraft(AFortPlayerController* FortPlayerController, AFortAircraft* Aircraft, FRotator ClientRotation);
};

namespace aircraft_component_hooks
{
    // Called when a player tries to exit the aircraft
    DEFINE_FUNCTION_HOOK(void, ServerAttemptAircraftJump, (AFortPlayerController* FortPlayerController, FFrame& StackFrame));
    DEFINE_FUNCTION_HOOK(void, UFortControllerComponent_Aircraft_ServerAttemptAircraftJump, (UFortControllerComponent_Aircraft* AircraftComponent, FFrame& StackFrame));

    // Called when the aircraft exits the drop zone
    DEFINE_FUNCTION_HOOK(void, OnAircraftExitedDropZone, (AFortGameModeAthena* GameMode, FFrame& StackFrame));

    // Native function called when a player enters the aircraft. Exists on any build
    DEFINE_FUNCTION_HOOK(void, EnterAircraft, (AFortPlayerControllerAthena* FortPlayerControllerAthena, AFortAircraft* FortAircraft));
};

// Used to choose a player start for the player exiting the aircraft
inline UObject* (*ChoosePlayerStart_Aircraft)(AFortGameModeAthena* GameMode, AFortPlayerController* PlayerController);