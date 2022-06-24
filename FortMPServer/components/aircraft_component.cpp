#include "include.hpp"
#include "aircraft_component.hpp"

#include "player_component.hpp"

aircraft_component::aircraft_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(aircraft_component_hooks::ServerAttemptAircraftJump, L"/Script/FortniteGame.FortPlayerControllerAthena.ServerAttemptAircraftJump", aircraft_component_hooks::ServerAttemptAircraftJump_Hook);
    REGISTER_UFUNCTION_DETOUR(aircraft_component_hooks::OnAircraftExitedDropZone, L"/Script/FortniteGame.FortGameModeAthena.OnAircraftExitedDropZone", aircraft_component_hooks::OnAircraftExitedDropZone_Hook);

    DetourTransactionCommit();
}

void aircraft_component_hooks::ServerAttemptAircraftJump_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    if ((*GWorld)->GameState()->Aircrafts().Num() < 0)
    {
        spdlog::error(L"Unable to exit the aircraft for player '{}', as no aircraft exists",
            FortPlayerController->GetName().c_str());
        
        return;
    }
    
    // TODO: Find a way to get the player's current aircraft
    AActor* aircraft = (*GWorld)->GameState()->Aircrafts()[0];
    
    if (aircraft)
    {
        AFortPlayerPawn* player_pawn = player_component::create_player(FortPlayerController, aircraft->GetTransform());

        if (player_pawn == nullptr)
        {
            spdlog::error(L"Failed to spawn a pawn for player '{}' when exiting from aircraft '{}'",
                player_pawn->GetName().c_str(),
                aircraft->GetName().c_str());
        }
    }
    
    return ServerAttemptAircraftJump(FortPlayerController, StackFrame);
}

void aircraft_component_hooks::OnAircraftExitedDropZone_Hook(AFortGameModeAthena* GameMode, FFrame& StackFrame)
{
    for (UNetConnection* NetConnection : (*GWorld)->NetDriver()->ClientConnections())
    {
        if (NetConnection->PlayerController()->IsInAircraft())
        {
            // Remove the player from the aircraft
            NetConnection->PlayerController()->ServerAttemptAircraftJump(FRotator { 0, 0, 0 });
        }
    }
    
    return OnAircraftExitedDropZone(GameMode, StackFrame);
}

