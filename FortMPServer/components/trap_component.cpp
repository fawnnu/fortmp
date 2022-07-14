#include "include.hpp"
#include "trap_component.hpp"

#include "ability_component.hpp"
#include "inventory_component.hpp"

trap_component::trap_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(trap_component_hooks::ServerSpawnDeco, L"/Script/FortniteGame.FortDecoTool.ServerSpawnDeco", trap_component_hooks::ServerSpawnDeco_Hook);
    
    DetourTransactionCommit();
}

void trap_component_hooks::ServerSpawnDeco_Hook(AFortDecoTool* FortDecoTool, FFrame& StackFrame)
{
    GET_PARAMETER(FVector, Location);
    GET_PARAMETER(FRotator, Rotation);
    GET_PARAMETER(ABuildingSMActor*, AttachedActor);

    UObject* actor_class = FortDecoTool->ItemDefinition()->BlueprintClass().Get();

    if (actor_class == nullptr)
    {
        // No class to spawn trap actor
        return;
    }

    // Spawn trap actor in world
    ABuildingTrap* trap_actor = reinterpret_cast<ABuildingTrap*>(SpawnActor(*GWorld, actor_class, &Location, &Rotation, FActorSpawnParameters {}));

    if (trap_actor == nullptr)
    {
        // No trap actor spawned
        return;
    }

    AFortPlayerController* player_controller = reinterpret_cast<AFortPlayerPawn*>(FortDecoTool->GetOwner())->Controller();

    if (player_controller == nullptr)
    {
        // Invalid owner of deco tool
        return;
    }
    
    // Setup trap
    trap_actor->InitializeKismetSpawnedBuildingActor(trap_actor, player_controller);
    
    // Attach trap to given building actor
    trap_actor->AttachedTo() = AttachedActor;

    // Update attached actor on server
    trap_actor->OnRep_AttachedTo();

    // Set trap team
    trap_actor->Team() = player_controller->PlayerState()->TeamIndex();

    if (trap_actor->AbilitySet())
    {
        // Apply any ability sets
        ability_component::apply_ability_set(FortDecoTool->GetOwner(), trap_actor->AbilitySet());
    }

    // Remove used trap
    auto replicated_entries = TArrayManager<FFortItemEntry>(&player_controller->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());
                 
    for (int i = 0; i < replicated_entries.Num(); i++)
    {
        FFortItemEntry& replicated_entry = replicated_entries[i];

        if (replicated_entry.ItemDefinition() == FortDecoTool->ItemDefinition())
        {
            inventory_component::remove_item(player_controller, replicated_entry.ItemGuid(), 1, false);

            break;
        }
    }
}

