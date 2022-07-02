#include "include.hpp"
#include "aircraft_component.hpp"

#include "inventory_component.hpp"
#include "player_component.hpp"

aircraft_component::aircraft_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(aircraft_component_hooks::ServerAttemptAircraftJump, L"/Script/FortniteGame.FortPlayerControllerAthena.ServerAttemptAircraftJump", aircraft_component_hooks::ServerAttemptAircraftJump_Hook);
    REGISTER_UFUNCTION_DETOUR(aircraft_component_hooks::OnAircraftExitedDropZone, L"/Script/FortniteGame.FortGameModeAthena.OnAircraftExitedDropZone", aircraft_component_hooks::OnAircraftExitedDropZone_Hook);

    REGISTER_FUNCTION_DETOUR(aircraft_component_hooks::EnterAircraft, memory_manager::find_function_by_string(L"EnterAircraft: %s"), aircraft_component_hooks::EnterAircraft_Hook);
    
    DetourTransactionCommit();
}

void aircraft_component_hooks::ServerAttemptAircraftJump_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(FRotator, ClientRotation);
    
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
        
        // Update pawn rotation
        player_pawn->K2_SetActorRotation(ClientRotation, true);
        
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

void aircraft_component_hooks::EnterAircraft_Hook(AFortPlayerController* PlayerController, void* unknown_data)
{
    // Get inventory
    auto replicated_entries = TArrayManager<FFortItemEntry>(&PlayerController->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());
    
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
        inventory_component::remove_item(PlayerController, item_to_remove, -1, false);
    }
    
    return EnterAircraft(PlayerController, unknown_data);
}
