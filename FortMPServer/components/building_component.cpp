#include "include.hpp"
#include "building_component.hpp"

#include "inventory_component.hpp"

building_component::building_component()
{
    INIT_POINTER(ReplaceBuildingActor, memory_manager::find_pattern("4C 89 44 24 ? 55 56 57 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ?")); 
    INIT_POINTER(ReplaceBuildingActor, memory_manager::find_function_by_string(L"STAT_Fort_BuildingSMActorReplaceBuildingActor"));
    INIT_POINTER(CanPlaceBuildingActor, memory_manager::find_pattern("E8 ? ? ? ? 85 C0 0F 85 ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? C6", true, 1));

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    REGISTER_UFUNCTION_DETOUR(building_component_hooks::ServerCreateBuildingActor, L"/Script/FortniteGame.FortPlayerController.ServerCreateBuildingActor", building_component_hooks::ServerCreateBuildingActor_Hook);
    REGISTER_UFUNCTION_DETOUR(building_component_hooks::ServerEditBuildingActor, L"/Script/FortniteGame.FortPlayerController.ServerEditBuildingActor", building_component_hooks::ServerEditBuildingActor_Hook);
    REGISTER_UFUNCTION_DETOUR(building_component_hooks::ServerBeginEditingBuildingActor, L"/Script/FortniteGame.FortPlayerController.ServerBeginEditingBuildingActor", building_component_hooks::ServerBeginEditingBuildingActor_Hook);
    REGISTER_UFUNCTION_DETOUR(building_component_hooks::ServerRepairBuildingActor, L"/Script/FortniteGame.FortPlayerController.ServerRepairBuildingActor", building_component_hooks::ServerRepairBuildingActor_Hook);
    
    DetourTransactionCommit();
}

bool building_component::spawn_building_actor(UObject* building_class, FVector building_location, FRotator building_rotation, bool is_mirrored, AFortPlayerController* building_owner)
{
    if (is_building_class_valid(building_class))
    {
        // Check if the player has enough materials
        if (ABuildingSMActor* BuildingActorToTest = CreateDefaultObject<ABuildingSMActor>(building_class))
        {
            if (UFortItemDefinition* ResourceItemDefinition = get_building_resource(BuildingActorToTest); ResourceItemDefinition && building_owner)
            {
                if (UFortItem* FoundItem = inventory_component::get_inventory_item_by_definition(building_owner, ResourceItemDefinition))
                {
                    if (static_cast<UFortWorldItem*>(FoundItem)->ItemEntry().Count() < 10)
                    {
                        // Player is probably cheating with buildfree, don't spawn the building
                        return false;
                    }
                }
            }
        }
            
        // Spawn a building actor in the world
        ABuildingSMActor* building_actor = reinterpret_cast<ABuildingSMActor*>(SpawnActor(*GWorld, building_class, &building_location, &building_rotation, FActorSpawnParameters {}));
     
        if (building_actor)
        {
            // Set mirrored
            building_actor->SetMirrored(is_mirrored);
             
            if (building_owner)
            {
                uint8_t new_team = building_owner->PlayerState()->TeamIndex();
                 
                // Only allow members of the team to edit the building actor
                building_actor->Team() = new_team;

                building_actor->SetTeam(new_team);
     
                // Initialize building actor
                building_actor->InitializeKismetSpawnedBuildingActor(building_actor, building_owner);

                // Pay building cost
                UFortItemDefinition* building_resource_definition = get_building_resource(building_actor);
                 
                auto replicated_entries = TArrayManager<FFortItemEntry>(&building_owner->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());
                 
                for (int i = 0; i < replicated_entries.Num(); i++)
                {
                    FFortItemEntry& replicated_entry = replicated_entries[i];

                    if (replicated_entry.ItemDefinition() == building_resource_definition)
                    {
                        inventory_component::remove_item(building_owner, replicated_entry.ItemGuid(), 10, false);

                        break;
                    }
                }
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

UFortItemDefinition* building_component::get_building_resource(ABuildingSMActor* building_actor)
{
    static UFortItemDefinition* WoodItemData = FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
    static UFortItemDefinition* StoneItemData = FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
    static UFortItemDefinition* MetalItemData = FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

    switch (building_actor->ResourceType())
    {
    case 0:
        return WoodItemData;
    case 1:
        return StoneItemData;
    case 2:
        return MetalItemData;
    default:
        return nullptr;
    }
}

void building_component_hooks::ServerCreateBuildingActor_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    // Support both legacy and new building
    static bool use_new_building = FindObject(L"/Script/FortniteGame.CreateBuildingActorData");
    
    UObject* building_class;
    FVector build_location;
    FRotator build_rotation;
    bool is_mirrored;
    
    if (use_new_building)
    {
        GET_PARAMETER(FCreateBuildingActorData, CreateBuildingData);

        // Used to get building class from a building class handle
        TArray<UObject*> building_actor_classes = (*GWorld)->GameState()->AllPlayerBuildableClasses();
        
        // Update existing properties
        building_class = building_actor_classes[CreateBuildingData.BuildingClassHandle()];
        build_location = CreateBuildingData.BuildLoc();
        build_rotation = CreateBuildingData.BuildRot();
        is_mirrored = CreateBuildingData.bMirrored();
    }
    else
    {
        GET_PARAMETER(FBuildingClassData, BuildingClassData);
        GET_PARAMETER(FVector, BuildLoc);
        GET_PARAMETER(FRotator, BuildRot);
        GET_PARAMETER(bool, bMirrored);

        // Update existing properties
        building_class = BuildingClassData.BuildingClass();
        build_location = BuildLoc;
        build_rotation = BuildRot;
        is_mirrored = bMirrored;
    }

    // Setup parameters for double building check
    TArray<AActor*> HoveredActors;
    __int64 second_parameter;
    
    if (!CanPlaceBuildingActor(*GWorld, building_class, &build_location, &build_rotation, is_mirrored, &HoveredActors, reinterpret_cast<__int64>(&second_parameter))) // We check if it returns 0 as it actually returns an enum: 0 means we can place the build
    {
        if (building_component::spawn_building_actor(building_class, build_location, build_rotation, is_mirrored, FortPlayerController))
        {
            // If the build was placed, destroy all Hovered Actors
            for (AActor* HoveredActor : HoveredActors)
            {
                HoveredActor->K2_DestroyActor();
            }
        }
    } 
    
    return ServerCreateBuildingActor(FortPlayerController, StackFrame);
}

void building_component_hooks::ServerEditBuildingActor_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(ABuildingActor*, BuildingActorToEdit);
    GET_PARAMETER(UObject*, NewBuildingClass);
    GET_PARAMETER(bool, bMirrored);
    GET_PARAMETER(int, RotationIterations);

    if (BuildingActorToEdit && NewBuildingClass)
    {
        // Edit building actor
        building_component::edit_building_actor(BuildingActorToEdit, NewBuildingClass, bMirrored, RotationIterations);
    }
    
    return ServerEditBuildingActor(FortPlayerController, StackFrame);
}

void building_component_hooks::ServerBeginEditingBuildingActor_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(ABuildingSMActor*, BuildingActorToEdit);

    if (!FortPlayerController)
    {
        spdlog::error(L"Invalid PlayerController tried to edit building actor.");
        
        return;
    }

    if (!FortPlayerController->Pawn())
    {
        spdlog::error(L"Player '{}' tried to edit building actor with invalid pawn.", FortPlayerController->MyPlayerInfo()->PlayerName().c_str());

        return;
    }

    if (BuildingActorToEdit)
    {
        // Edit tool item definition
        static UFortItemDefinition* edit_tool_definition = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/EditTool.EditTool");

        // Get edit tool item
        UFortItem* edit_tool = FortPlayerController->HasFunction("K2_FindExistingItemForDefinition") ?
            FortPlayerController->K2_FindExistingItemForDefinition(edit_tool_definition) : FortPlayerController->BP_FindExistingItemForDefinition(edit_tool_definition);

        if (!memory_manager::is_bad_read_pointer(edit_tool))
        {
            // Equip edit tool
            AFortWeap_EditingTool* edit_tool_weapon = reinterpret_cast<AFortWeap_EditingTool*>(FortPlayerController->Pawn()->EquipWeaponDefinition(edit_tool_definition, edit_tool->GetItemGuid()));

            if (!memory_manager::is_bad_read_pointer(edit_tool_weapon))
            {
                // Update build in edit tool
                edit_tool_weapon->EditActor() = BuildingActorToEdit;
            }
        }
    }
    
    return ServerBeginEditingBuildingActor(FortPlayerController, StackFrame);
}

void building_component_hooks::ServerRepairBuildingActor_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(ABuildingSMActor*, BuildingActorToRepair);

    // TODO: Get proper repair cost, most likely to do with BuildingRepairCostMultiplierHandles
    constexpr int repair_cost = 3;

    if (BuildingActorToRepair)
    {
        // Repair building
        BuildingActorToRepair->RepairBuilding(FortPlayerController, repair_cost);

        // TODO: Repair cost
    }
    
    return ServerRepairBuildingActor(FortPlayerController, StackFrame);
}
