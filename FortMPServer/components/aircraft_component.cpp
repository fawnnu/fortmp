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

void aircraft_component::exit_aircraft(AFortPlayerController* FortPlayerController, AFortAircraft* Aircraft, FRotator ClientRotation)
{
    if (Aircraft)
    {
        auto ban_list = dll_utils::get_banlist();

        for (auto& elem : ban_list["Bans"].items())
        {
            std::string IP = elem.value();

            if (std::wstring(IP.begin(), IP.end()) ==
                std::wstring(FortPlayerController->PlayerState()->SavedNetworkAddress().c_str()))
            {
                FortPlayerController->ClientReturnToMainMenu(L"");

                return;
            }
        }
        
        AFortPlayerPawn* player_pawn = player_component::create_player(FortPlayerController, Aircraft->GetTransform());
        
        // Update rotation
        FortPlayerController->SetControlRotation(ClientRotation);
        
        if (player_pawn == nullptr)
        {
            spdlog::error(L"Failed to spawn a pawn for player '{}' when exiting from aircraft '{}'",
                FortPlayerController->GetName().c_str(),
                Aircraft->GetName().c_str());

            return;
        }

        if (FortPlayerController != nullptr && FortPlayerController->WorldInventory() != nullptr)
        {
            // Auto equip pickaxe, missing on old builds
            auto replicated_entries = TArrayManager<FFortItemEntry>(&FortPlayerController->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());

            if (replicated_entries.Num() > 0)
            {
                FortPlayerController->ServerExecuteInventoryItem(replicated_entries[0].ItemGuid());
            }
        }
    }
}

void aircraft_component_hooks::UFortControllerComponent_Aircraft_ServerAttemptAircraftJump_Hook(UFortControllerComponent_Aircraft* AircraftComponent, FFrame& StackFrame)
{
    GET_PARAMETER(FRotator, ClientRotation);

    // Exit aircraft for player
    aircraft_component::exit_aircraft(static_cast<AFortPlayerController*>(AircraftComponent->GetOwner()), AircraftComponent->CurrentAircraft(), ClientRotation);
    
    return UFortControllerComponent_Aircraft_ServerAttemptAircraftJump(AircraftComponent, StackFrame);
}

void aircraft_component_hooks::EnterAircraft_Hook(UObject* _this, AFortAircraft* Aircraft)
{
    STATIC_HASPROPERTY(_this, CurrentAircraft);
    
    AFortPlayerControllerAthena* FortPlayerControllerAthena = bExists_CurrentAircraft ?
        static_cast<AFortPlayerControllerAthena*>(static_cast<UFortControllerComponent_Aircraft*>(_this)->GetOwner()) :
        static_cast<AFortPlayerControllerAthena*>(_this);
    
    // Get inventory
    auto replicated_entries = TArrayManager<FFortItemEntry>(&FortPlayerControllerAthena->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());
    
    // Item definitions to filter by
    static UObject* building_item_definition = FindObject(L"/Script/FortniteGame.FortBuildingItemDefinition");
    static UObject* melee_item_definition = FindObject(L"/Script/FortniteGame.FortWeaponMeleeItemDefinition");
    static UObject* edit_tool_definition = FindObject(L"/Script/FortniteGame.FortEditToolItemDefinition");
    
    // Backwards loop to remove items from inventory
    for (int i = replicated_entries.Num() - 1; i >= 0; i--)
    {
        FFortItemEntry& item_entry = replicated_entries[i];
        
        if (!item_entry.ItemDefinition()->IsA(building_item_definition) &&
            !item_entry.ItemDefinition()->IsA(melee_item_definition) &&
            !item_entry.ItemDefinition()->IsA(edit_tool_definition))
        {
            // Remove any items from the inventory that do not match
            inventory_component::remove_item(FortPlayerControllerAthena, item_entry.ItemGuid(), -1, false);
        }
    }

    (*GWorld)->AuthorityGameMode()->OnEnterAircraft(FortPlayerControllerAthena);
    
    return EnterAircraft(_this, Aircraft);
}