#include "include.hpp"
#include "trap_component.hpp"

#include "ability_component.hpp"
#include "building_component.hpp"
#include "challenges_component.hpp"
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

void trap_component_hooks::ServerSpawnDeco_Hook(AFortDecoTool* FortDecoTool, FVector* Location, FRotator* Rotation, ABuildingSMActor* AttachedActor, uint8_t InBuildingAttachmentType)
{
    ServerSpawnDeco(FortDecoTool, Location, Rotation, AttachedActor, InBuildingAttachmentType);
    
    AFortPlayerControllerAthena* player_controller = static_cast<AFortPlayerControllerAthena*>(static_cast<AFortPlayerPawn*>(FortDecoTool->GetOwner())->Controller());

    challenges_component::SendCustomEvent(player_controller, "interaction:placed-trap");
}

ABuildingActor* trap_component::CreateBuilding(AFortPlayerController* BuildingOwner, UObject** Class, FVector* Location, FRotator* Rotation)
{
    ABuildingActor* NewBuildingActor = nullptr;

    if (BuildingOwner)
    {
        building_component::spawn_building_actor(*Class, *Location, *Rotation, false, BuildingOwner, EFortBuildPreviewMarkerOptionalAdjustment::None, false, &NewBuildingActor);
    }
    
    return NewBuildingActor;
}


