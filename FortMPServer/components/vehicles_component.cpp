#include "include.hpp"
#include "vehicles_component.hpp"

#include "inventory_component.hpp"

vehicles_component::vehicles_component()
{
    // Hook the last call in EquipAbilityWeapon
    REGISTER_CALL_DETOUR(L"/Script/FortniteGame.FortGameplayAbility.EquipAbilityWeapon", vehicles_component_hooks::EquipAbilityWeapon_Hook);
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerAttemptExitVehicle, L"/Script/FortniteGame.FortPlayerControllerZone.ServerAttemptExitVehicle", vehicles_component_hooks::ServerAttemptExitVehicle_Hook);

    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerUpdatePhysicsParams, L"/Script/FortniteGame.FortAthenaVehicle.ServerUpdatePhysicsParams", vehicles_component_hooks::ServerUpdatePhysicsParams_Hook);
    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerUpdatePhysicsParams, L"/Script/FortniteGame.FortPhysicsPawn.ServerUpdatePhysicsParams", vehicles_component_hooks::ServerUpdatePhysicsParams_Hook);
    REGISTER_UFUNCTION_DETOUR(vehicles_component_hooks::ServerUpdatePhysicsParams, L"/Script/FortniteGame.FortPhysicsPawn.ServerMove", vehicles_component_hooks::ServerUpdatePhysicsParams_Hook);
    
    DetourTransactionCommit();
}

// re-add code for this function; it's stripped
void vehicles_component_hooks::EquipAbilityWeapon_Hook(UFortGameplayAbility* FortGameplayAbility, UFortWeaponItemDefinition* WeaponDefinition)
{
    // make sure the weapon definition is valid
    if (WeaponDefinition)
    {
        auto replicated_entries = TArrayManager<FFortItemEntry>(&FortGameplayAbility->GetActivatingPawn()->Controller()->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());

        // remove any existing instance of this vehicle weapon
        for (int i = 0; i < replicated_entries.Num(); i++)
        {
            FFortItemEntry& replicated_entry = replicated_entries[i];

            if (replicated_entry.ItemDefinition() == WeaponDefinition)
            {
                inventory_component::remove_item(FortGameplayAbility->GetActivatingPawn()->Controller(), replicated_entry.ItemGuid(), 1, false);
            }
        }

        // add it to the inventory
        auto new_item = inventory_component::add_item(FortGameplayAbility->GetActivatingPawn()->Controller(), WeaponDefinition, 1, INT_MAX);

        // equip the ability weapon
        FortGameplayAbility->GetActivatingPawn()->EquipWeaponDefinition(WeaponDefinition, new_item->ItemEntry().ItemGuid());
    }
}

void vehicles_component_hooks::ServerAttemptExitVehicle_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    auto replicated_entries = TArrayManager<FFortItemEntry>(&FortPlayerController->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());

    //First entry is always pickaxe
    FortPlayerController->ServerExecuteInventoryItem(replicated_entries[0].ItemGuid());
        
    return ServerAttemptExitVehicle(FortPlayerController, StackFrame);
}

// re-add vehicle sync
void vehicles_component_hooks::ServerUpdatePhysicsParams_Hook(AActor* Vehicle, FFrame& StackFrame)
{
    GET_PARAMETER(FReplicatedPhysicsPawnState, InState);

    if (const auto RootComponent = Vehicle->RootComponent())
    {
        RootComponent->K2_SetWorldLocation(InState.Translation, false, true, nullptr);

        RootComponent->SetPhysicsLinearVelocity(InState.LinearVelocity, false, L"None");
        
        RootComponent->SetPhysicsAngularVelocityInRadians(InState.AngularVelocity, false, L"None");
    }
    
    return ServerUpdatePhysicsParams(Vehicle, StackFrame);   
}
