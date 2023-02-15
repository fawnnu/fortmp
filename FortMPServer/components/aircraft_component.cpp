#include "include.hpp"
#include "aircraft_component.hpp"

#include "inventory_component.hpp"
#include "player_component.hpp"

aircraft_component::aircraft_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(aircraft_component_hooks::UFortControllerComponent_Aircraft_ServerAttemptAircraftJump, L"/Script/FortniteGame.FortControllerComponent_Aircraft.ServerAttemptAircraftJump", aircraft_component_hooks::UFortControllerComponent_Aircraft_ServerAttemptAircraftJump_Hook);
    
    REGISTER_FUNCTION_DETOUR(aircraft_component_hooks::EnterAircraft, memory_manager::find_function_by_string(L"EnterAircraft: %s"), aircraft_component_hooks::EnterAircraft_Hook);
    
    DetourTransactionCommit();
}

void aircraft_component::exit_aircraft(AFortPlayerController* FortPlayerController, FRotator ClientRotation)
{
    (*GWorld)->AuthorityGameMode()->RestartPlayer(FortPlayerController);
        
    // Update rotation
    FortPlayerController->SetControlRotation(ClientRotation);
}

void aircraft_component_hooks::UFortControllerComponent_Aircraft_ServerAttemptAircraftJump_Hook(UFortControllerComponent_Aircraft* AircraftComponent, FFrame& StackFrame)
{
    GET_PARAMETER(FRotator, ClientRotation);
    
    // Exit aircraft for player
    aircraft_component::exit_aircraft(static_cast<AFortPlayerController*>(AircraftComponent->GetOwner()), ClientRotation);
    
    return UFortControllerComponent_Aircraft_ServerAttemptAircraftJump(AircraftComponent, StackFrame);
}

void aircraft_component_hooks::EnterAircraft_Hook(UObject* _this, AFortAircraft* Aircraft)
{
    STATIC_HASPROPERTY(_this, CurrentAircraft);
    
    AFortPlayerControllerAthena* FortPlayerControllerAthena = bExists_CurrentAircraft ?
        static_cast<AFortPlayerControllerAthena*>(static_cast<UFortControllerComponent_Aircraft*>(_this)->GetOwner()) :
        static_cast<AFortPlayerControllerAthena*>(_this);

    if (FortPlayerControllerAthena && FortPlayerControllerAthena->WorldInventory())
    {
        // Clear inventory
        inventory_component::clear_primary_inventory(FortPlayerControllerAthena, false);

        (*GWorld)->AuthorityGameMode()->OnEnterAircraft(FortPlayerControllerAthena);
    }
    
    return EnterAircraft(_this, Aircraft);
}