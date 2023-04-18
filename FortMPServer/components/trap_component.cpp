#include "include.hpp"
#include "trap_component.hpp"

#include "ability_component.hpp"
#include "building_component.hpp"
#include "inventory_component.hpp"

trap_component::trap_component()
{
    // Make ServerCreateBuildingAndSpawnDeco work
    if (uintptr_t target_function = memory_manager::find_function_by_string(L"ServerCreateBuildingAndSpawnDeco called without a valid DecoItemDef"))
    {
        if (uintptr_t target_call_instruction = memory_manager::scan_for(target_function, {0xE8, WILDCARD, WILDCARD, WILDCARD, 0xFE}, memory_manager::get_function_end(target_function)))
        {
            memory_manager::call_hook(target_call_instruction, reinterpret_cast<uintptr_t>(CreateBuilding));

            for (int i = 0; ; i++)
            {
                if (CreateDefaultObject<AFortDecoTool>()->VTable[i] == reinterpret_cast<PVOID>(target_function))
                {
                    uintptr_t target_function = reinterpret_cast<uintptr_t>(CreateDefaultObject<AFortDecoTool_ContextTrap>()->VTable[i]);
                    if (uintptr_t target_call_instruction = memory_manager::scan_for(target_function, {0xE8, WILDCARD, WILDCARD, WILDCARD, 0xFE}, memory_manager::get_function_end(target_function)))
                    {
                        memory_manager::call_hook(target_call_instruction, reinterpret_cast<uintptr_t>(CreateBuilding));
                    }
                    break;
                }
            }
        }
    }

    // C++ hook ServerSpawnDeco (Makes hook work with C++ virtual calls too)
    if (uintptr_t target_instruction = memory_manager::scan_for(memory_manager::get_function_end(memory_manager::find_function_by_string(L"ServerSpawnDeco_Validate")), {0xFF, WILDCARD, WILDCARD, WILDCARD, 0x0}, memory_manager::find_function_by_string(L"ServerSpawnDeco_Validate"), true))
    {
        int index = *reinterpret_cast<int*>(target_instruction + 2) / 8;

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        REGISTER_FUNCTION_DETOUR(trap_component_hooks::ServerSpawnDeco, CreateDefaultObject<AFortTrapTool>()->VTable[index], trap_component_hooks::ServerSpawnDeco_Hook);
    
        DetourTransactionCommit();
    }
}

ABuildingActor* trap_component::CreateBuilding(AFortPlayerController* BuildingOwner, UObject** Class, FVector* Location, FRotator* Rotation)
{
    ABuildingActor* NewBuildingActor;
    building_component::spawn_building_actor(*Class, *Location, *Rotation, false, BuildingOwner, 0, false, &NewBuildingActor);

    return NewBuildingActor;
}

void trap_component_hooks::ServerCreateBuildingAndSpawnDeco_Hook(AFortDecoTool* FortDecoTool, FFrame& StackFrame)
{
    ServerCreateBuildingAndSpawnDeco(FortDecoTool, StackFrame);
}

void trap_component_hooks::ServerSpawnDeco_Hook(AFortDecoTool* FortDecoTool, FVector* Location, FRotator* Rotation, ABuildingSMActor* AttachedActor, uint8_t InBuildingAttachmentType)
{
    // Call native function to properly spawn the trap
    ServerSpawnDeco(FortDecoTool, Location, Rotation, AttachedActor, InBuildingAttachmentType);

    if (AttachedActor == nullptr || AttachedActor->AttachedBuildingActors().Num() < 1)
    {
        return;
    }

    AFortPlayerControllerAthena* player_controller = static_cast<AFortPlayerControllerAthena*>(static_cast<AFortPlayerPawn*>(FortDecoTool->GetOwner())->Controller());

    // Take the most recent attached actor (new trap)
    ABuildingSMActor* trap_actor = AttachedActor->AttachedBuildingActors()[AttachedActor->AttachedBuildingActors().Num() - 1];

    /* DEPRECATED
     * This is no more used, we have moved to WorldPlayerId, this sets the team automatically. However there were reports from Jake that it MIGHT cause some issues, so i will keep this chunk of code just in case.

    // Get the new team for this trap
    uint8_t new_team = player_controller->PlayerState()->TeamIndex();
    
    // Set trap team
    trap_actor->Team() = new_team;

    // Set trap team on newer builds
    trap_actor->SetTeam(new_team); */

    // Pay trap cost
    if (UFortItem* item_to_remove = inventory_component::get_inventory_item_by_guid(player_controller, FortDecoTool->ItemEntryGuid()))
    {
        inventory_component::remove_item(player_controller, static_cast<UFortWorldItem*>(item_to_remove)->ItemEntry().ItemGuid(), 1, false);
    }
    else
    {
        spdlog::error(L"trap_component_hooks::ServerSpawnDeco_Hook failed to pay trap cost, invalid item_to_remove");
    }
}

