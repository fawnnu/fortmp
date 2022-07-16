#include "include.hpp"
#include "aircraft_component.hpp"

#include "inventory_component.hpp"
#include "player_component.hpp"

aircraft_component::aircraft_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(aircraft_component_hooks::ServerAttemptAircraftJump, L"/Script/FortniteGame.FortPlayerControllerAthena.ServerAttemptAircraftJump", aircraft_component_hooks::ServerAttemptAircraftJump_Hook);
    REGISTER_UFUNCTION_DETOUR(aircraft_component_hooks::UFortControllerComponent_Aircraft_ServerAttemptAircraftJump, L"/Script/FortniteGame.FortControllerComponent_Aircraft.ServerAttemptAircraftJump", aircraft_component_hooks::UFortControllerComponent_Aircraft_ServerAttemptAircraftJump_Hook);
    REGISTER_UFUNCTION_DETOUR(aircraft_component_hooks::OnAircraftExitedDropZone, L"/Script/FortniteGame.FortGameModeAthena.OnAircraftExitedDropZone", aircraft_component_hooks::OnAircraftExitedDropZone_Hook);

    REGISTER_UFUNCTION_DETOUR(aircraft_component_hooks::ServerSetInAircraft, L"/Script/FortniteGame.FortPlayerStateAthena.ServerSetInAircraft", aircraft_component_hooks::ServerSetInAircraft_Hook);
    REGISTER_UFUNCTION_DETOUR(aircraft_component_hooks::ServerSetInAircraft, L"/Script/FortniteGame.FortPlayerStateZone.ServerSetInAircraft", aircraft_component_hooks::ServerSetInAircraft_Hook);
    
    DetourTransactionCommit();
}

void aircraft_component::exit_aircraft(AFortPlayerController* FortPlayerController, AFortAircraft* Aircraft, FRotator ClientRotation)
{
    if (Aircraft)
    {
        AFortPlayerPawn* player_pawn = player_component::create_player(FortPlayerController, Aircraft->GetTransform());
        
        // Update rotation
        FortPlayerController->SetControlRotation(ClientRotation);
        
        if (player_pawn == nullptr)
        {
            spdlog::error(L"Failed to spawn a pawn for player '{}' when exiting from aircraft '{}'",
                player_pawn->GetName().c_str(),
                Aircraft->GetName().c_str());
        }
    }    
}

void aircraft_component_hooks::ServerAttemptAircraftJump_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(FRotator, ClientRotation);

    // Get aircraft that player has exited from
    AActor* FortAircraft = (*GWorld)->GameState()->HasProperty(L"Aircraft") ?
        (*GWorld)->GameState()->Aircraft() : (*GWorld)->GameState()->Aircrafts()[0];

    // Exit aircraft for player
    aircraft_component::exit_aircraft(FortPlayerController, static_cast<AFortAircraft*>(FortAircraft), ClientRotation);
    
    return ServerAttemptAircraftJump(FortPlayerController, StackFrame);
}

void aircraft_component_hooks::OnAircraftExitedDropZone_Hook(AFortGameModeAthena* GameMode, FFrame& StackFrame)
{
    // Fix as function does not contain the exiting aircraft on older builds
    AFortAircraft* FortAircraft = nullptr;
    
    if (!StackFrame.Node->HasProperty(L"FortAthenaAircraft"))
    {
        // Update aircraft with one from GameState
        FortAircraft = reinterpret_cast<AFortAircraft*>((*GWorld)->GameState()->Aircraft());
    }
    else
    {
        GET_PARAMETER(AFortAircraft*, FortAthenaAircraft);

        // Update aircraft with one from function
        FortAircraft = FortAthenaAircraft;
    }

    for (UNetConnection* NetConnection : (*GWorld)->NetDriver()->ClientConnections())
    {
        if (NetConnection->PlayerController()->IsInAircraft())
        {
            // Remove the player from the aircraft
            aircraft_component::exit_aircraft(NetConnection->PlayerController(), FortAircraft, FRotator{ 0, 0, 0 });
        }
    }
    
    return OnAircraftExitedDropZone(GameMode, StackFrame);
}

void aircraft_component_hooks::UFortControllerComponent_Aircraft_ServerAttemptAircraftJump_Hook(UFortControllerComponent_Aircraft* AircraftComponent, FFrame& StackFrame)
{
    GET_PARAMETER(FRotator, ClientRotation);

    // Exit aircraft for player
    aircraft_component::exit_aircraft(static_cast<AFortPlayerController*>(AircraftComponent->GetOwner()), AircraftComponent->CurrentAircraft(), ClientRotation);
    
    return UFortControllerComponent_Aircraft_ServerAttemptAircraftJump(AircraftComponent, StackFrame);
}


void aircraft_component_hooks::ServerSetInAircraft_Hook(AFortPlayerState* PlayerState, FFrame& StackFrame)
{
    if (PlayerState == nullptr)
    {
        // No player state to use
        return;
    }
    
    GET_PARAMETER(bool, bNewInAircraft);
    
    if (bNewInAircraft)
    {
        AFortPlayerController* player_controller = reinterpret_cast<AFortPlayerController*>(PlayerState->GetOwner());
    
        // Get inventory
        auto replicated_entries = TArrayManager<FFortItemEntry>(&player_controller->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());
    
        // List of items to remove
        std::vector<FGuid> items_to_remove;

        // Item definitions to filter by
        static UObject* building_item_definition = FindObject(L"/Script/FortniteGame.FortBuildingItemDefinition");
        static UObject* melee_item_definition = FindObject(L"/Script/FortniteGame.FortWeaponMeleeItemDefinition");
        static UObject* edit_tool_definition = FindObject(L"/Script/FortniteGame.FortEditToolItemDefinition");

        for (FFortItemEntry& item_entry : replicated_entries)
        {
            if (!item_entry.ItemDefinition()->IsA(building_item_definition) &&
                !item_entry.ItemDefinition()->IsA(melee_item_definition) &&
                !item_entry.ItemDefinition()->IsA(edit_tool_definition))
            {
                // Remove any items from the inventory that do not match
                items_to_remove.push_back(item_entry.ItemGuid());
            }
        }

        for (FGuid item_to_remove : items_to_remove)
        {
            // Remove item
            inventory_component::remove_item(player_controller, item_to_remove, -1, false);
        }
    }
    
    return ServerSetInAircraft(PlayerState, StackFrame);
}