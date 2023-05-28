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

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(trap_component_hooks::ServerSpawnDeco, L"/Script/FortniteGame.FortDecoTool.ServerSpawnDeco", trap_component_hooks::ServerSpawnDeco_Hook);
    REGISTER_UFUNCTION_DETOUR(trap_component_hooks::ServerCreateBuildingAndSpawnDeco, L"/Script/FortniteGame.FortDecoTool.ServerCreateBuildingAndSpawnDeco", trap_component_hooks::ServerCreateBuildingAndSpawnDeco_Hook);
    
    DetourTransactionCommit();
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

    AFortPlayerControllerAthena* player_controller = static_cast<AFortPlayerControllerAthena*>(static_cast<AFortPlayerPawn*>(FortDecoTool->GetOwner())->Controller());
    
    // Pay trap cost
    if (UFortItem* item_to_remove = inventory_component::get_inventory_item_by_guid(player_controller, FortDecoTool->ItemEntryGuid()))
    {
        inventory_component::remove_item(player_controller, static_cast<UFortWorldItem*>(item_to_remove)->ItemEntry().ItemGuid(), 1, false);
    }
    else
    {
        spdlog::error(L"trap_component_hooks::ServerCreateBuildingAndSpawnDeco_Hook failed to pay trap cost, invalid item_to_remove");
    }
}

void trap_component_hooks::ServerSpawnDeco_Hook(AFortDecoTool* FortDecoTool, FFrame& StackFrame)
{
    // Call native function to properly spawn the trap
    ServerSpawnDeco(FortDecoTool, StackFrame);

    AFortPlayerControllerAthena* player_controller = static_cast<AFortPlayerControllerAthena*>(static_cast<AFortPlayerPawn*>(FortDecoTool->GetOwner())->Controller());
    
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

