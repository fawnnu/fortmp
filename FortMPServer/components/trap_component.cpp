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
    GET_PARAMETER(ABuildingSMActor*, AttachedActor);

    // Call native function to properly spawn the trap
    ServerSpawnDeco(FortDecoTool, StackFrame);

    if (AttachedActor == nullptr || AttachedActor->AttachedBuildingActors().Num() < 1)
    {
        return;
    }

    AFortPlayerControllerAthena* player_controller = static_cast<AFortPlayerControllerAthena*>(static_cast<AFortPlayerPawn*>(FortDecoTool->GetOwner())->Controller());

    // Take the most recent attached actor (new trap)
    ABuildingSMActor* trap_actor = AttachedActor->AttachedBuildingActors()[AttachedActor->AttachedBuildingActors().Num() - 1];

    uint8_t new_team = player_controller->PlayerState()->TeamIndex();
    
    // Set trap team
    trap_actor->Team() = new_team;

    // Set trap team on newer builds
    trap_actor->SetTeam(new_team);

    // Pay trap cost
    if (UFortItem* item_to_remove = inventory_component::get_inventory_item_by_definition(player_controller, FortDecoTool->ItemDefinition()))
    {
        inventory_component::remove_item(player_controller, static_cast<UFortWorldItem*>(item_to_remove)->ItemEntry().ItemGuid(), 1, false);
    }
}

