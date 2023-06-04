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
}

ABuildingActor* trap_component::CreateBuilding(AFortPlayerController* BuildingOwner, UObject** Class, FVector* Location, FRotator* Rotation)
{
    ABuildingActor* NewBuildingActor;
    building_component::spawn_building_actor(*Class, *Location, *Rotation, false, BuildingOwner, 0, false, &NewBuildingActor);

    return NewBuildingActor;
}


