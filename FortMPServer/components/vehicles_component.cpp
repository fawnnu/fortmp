#include "include.hpp"
#include "vehicles_component.hpp"

#include "inventory_component.hpp"

vehicles_component::vehicles_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::EquipAbilityWeapon, L"/Script/FortniteGame.FortGameplayAbility.EquipAbilityWeapon", vehicles_component_hooks::EquipAbilityWeapon_Hook);
    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerAttemptExitVehicle, L"/Script/FortniteGame.FortPlayerControllerZone.ServerAttemptExitVehicle", vehicles_component_hooks::ServerAttemptExitVehicle_Hook);
    
    DetourTransactionCommit();
}

// re-add code for this function; it's stripped
void vehicles_component_hooks::EquipAbilityWeapon_Hook(UFortGameplayAbility* FortGameplayAbility, FFrame& StackFrame)
{
    // params unpacking from StackFrame.Code
    UFortWeaponItemDefinition* WeaponDefinition = nullptr;

    StackFrame.Step(FortGameplayAbility, &WeaponDefinition);

    // make sure the weapon definition is valid
    if (WeaponDefinition)
    {
        auto replicated_entries = TArrayManager<FFortItemEntry>(&FortGameplayAbility->GetActivatingPawn()->Controller()->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());
        
        for (int i = 0; i < replicated_entries.Num(); i++)
        {
            FFortItemEntry& replicated_entry = replicated_entries[i];

            if (replicated_entry.ItemDefinition() == WeaponDefinition)
            {
                FortGameplayAbility->GetActivatingPawn()->EquipWeaponDefinition(WeaponDefinition, replicated_entry.ItemGuid());

                // return if the weapon gets equipped
                return EquipAbilityWeapon(FortGameplayAbility, StackFrame);
            }
        }

        // if the item isn't found, add it to the inventory
        auto new_item = inventory_component::add_item(FortGameplayAbility->GetActivatingPawn()->Controller(), WeaponDefinition, 1, INT_MAX);

        // equip the ability weapon
        FortGameplayAbility->GetActivatingPawn()->EquipWeaponDefinition(WeaponDefinition, new_item->ItemEntry().ItemGuid());
    }

    return EquipAbilityWeapon(FortGameplayAbility, StackFrame);
}

void vehicles_component_hooks::ServerAttemptExitVehicle_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    auto replicated_entries = TArrayManager<FFortItemEntry>(&FortPlayerController->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());

    //First entry is always pickaxe
    FortPlayerController->ServerExecuteInventoryItem(replicated_entries[0].ItemGuid());
        
    return ServerAttemptExitVehicle(FortPlayerController, StackFrame);
}

