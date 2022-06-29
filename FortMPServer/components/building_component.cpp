#include "include.hpp"
#include "building_component.hpp"

building_component::building_component()
{
    INIT_POINTER(ReplaceBuildingActor, memory_manager::find_function_by_string(L"STAT_Fort_BuildingSMActorReplaceBuildingActor"));

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    REGISTER_UFUNCTION_DETOUR(building_component_hooks::ServerCreateBuildingActor, L"ServerCreateBuildingActor", building_component_hooks::ServerCreateBuildingActor_Hook);
    REGISTER_UFUNCTION_DETOUR(building_component_hooks::ServerEditBuildingActor, L"ServerEditBuildingActor", building_component_hooks::ServerEditBuildingActor_Hook);
    REGISTER_UFUNCTION_DETOUR(building_component_hooks::ServerRepairBuildingActor, L"ServerRepairBuildingActor", building_component_hooks::ServerRepairBuildingActor_Hook);

    DetourTransactionCommit();
}

bool building_component::spawn_building_actor(UObject* building_class, FVector building_location, FRotator building_rotation, AFortPlayerController* building_owner)
{
    if (is_building_class_valid(building_class))
    {
         // Spawn in a building actor to the world
         ABuildingActor* building_actor = reinterpret_cast<ABuildingActor*>(SpawnActor(*GWorld, building_class, &building_location, &building_rotation, FActorSpawnParameters {}));
     
         if (building_actor)
         {
             if (building_owner)
             {
                 // Only allow members of the team to edit the building actor
                 building_actor->Team() = building_owner->PlayerState()->TeamIndex();
     
                 // Initialize building actor
                 building_actor->Call(L"InitializeKismetSpawnedBuildingActor", building_actor, building_owner);
             }
     
             return true;
         }   
    }
    
    return false;
}

bool building_component::edit_building_actor(ABuildingActor* building_actor, UObject* building_class, bool is_mirrored, int rotation_iterations)
{
    if (is_building_class_valid(building_class))
    {
        if (building_actor)
        {
            constexpr int replacement_destruction_reason = 1;
            constexpr int building_level = 0;
        
            // Edit building actor
            ReplaceBuildingActor(building_actor, replacement_destruction_reason, building_class, building_level, rotation_iterations, is_mirrored, nullptr);

            return true;
        }
    }

    return false;
}

bool building_component::is_building_class_valid(UObject* building_class)
{
    for (UObject* building_actor_class : (*GWorld)->GameState()->BuildingActorClasses())
    {
        if (building_actor_class == building_class)
        {
            // Only allow valid building classes
            return true;
        }
    }
    
    return false;
}

void building_component_hooks::ServerCreateBuildingActor_Hook(AFortPlayerController* FortPlayerController,
    FFrame& StackFrame)
{
    return ServerCreateBuildingActor(FortPlayerController, StackFrame);
}

void building_component_hooks::ServerEditBuildingActor_Hook(AFortPlayerController* FortPlayerController,
    FFrame& StackFrame)
{
    return ServerEditBuildingActor(FortPlayerController, StackFrame);
}

void building_component_hooks::ServerRepairBuildingActor_Hook(AFortPlayerController* FortPlayerController,
    FFrame& StackFrame)
{
    return ServerRepairBuildingActor(FortPlayerController, StackFrame);
}
