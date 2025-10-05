#include "include.hpp"
#include "building_component.hpp"

#include "anticheat_component.hpp"
#include "creative_component.hpp"
#include "inventory_component.hpp"
#include "log_component.hpp"
#include "player_component.hpp"
#include "team_component.hpp"

building_component::building_component()
{
    INIT_POINTER(ReplaceBuildingActor, memory_manager::find_pattern("4C 89 44 24 ? 55 56 57 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ?")); 
    INIT_POINTER(ReplaceBuildingActor, memory_manager::find_function_by_string(L"STAT_Fort_BuildingSMActorReplaceBuildingActor"));
    INIT_POINTER(CanPlaceBuildingActor, memory_manager::find_pattern("E8 ? ? ? ? 85 C0 0F 85 ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? C6", true, 1));
    INIT_POINTER(ProcessRepairCost, memory_manager::find_function_by_string(L"Failed to remove item %s during repair, item duplicated!"));

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    REGISTER_UFUNCTION_DETOUR(building_component_hooks::ServerCreateBuildingActor, L"/Script/FortniteGame.FortPlayerController.ServerCreateBuildingActor", building_component_hooks::ServerCreateBuildingActor_Hook);
    REGISTER_UFUNCTION_DETOUR(building_component_hooks::ServerEditBuildingActor, L"/Script/FortniteGame.FortPlayerController.ServerEditBuildingActor", building_component_hooks::ServerEditBuildingActor_Hook);
    REGISTER_UFUNCTION_DETOUR(building_component_hooks::ServerEndEditingBuildingActor, L"/Script/FortniteGame.FortPlayerController.ServerEndEditingBuildingActor", building_component_hooks::ServerEndEditingBuildingActor_Hook);
    REGISTER_UFUNCTION_DETOUR(building_component_hooks::ServerBeginEditingBuildingActor, L"/Script/FortniteGame.FortPlayerController.ServerBeginEditingBuildingActor", building_component_hooks::ServerBeginEditingBuildingActor_Hook);
    REGISTER_UFUNCTION_DETOUR(building_component_hooks::ServerRepairBuildingActor, L"/Script/FortniteGame.FortPlayerController.ServerRepairBuildingActor", building_component_hooks::ServerRepairBuildingActor_Hook);
    
    DetourTransactionCommit();
}

void building_component_hooks::ServerEndEditingBuildingActor_Hook(AFortPlayerController* FortPlayerController,
    FFrame& StackFrame)
{
    GET_PARAMETER(ABuildingSMActor*, BuildingActorToStopEditing);
    if (BuildingActorToStopEditing)
    {
        BuildingActorToStopEditing->ForceNetUpdate();   
    }
    return ServerEndEditingBuildingActor(FortPlayerController, StackFrame);
}

bool building_component::spawn_building_actor(UObject* building_class, FVector building_location, FRotator building_rotation, bool is_mirrored, AFortPlayerController* building_owner, EFortBuildPreviewMarkerOptionalAdjustment MarkerOptionalAdjustment, bool is_building_free, ABuildingActor** OutBuildingActor)
{
    // Save the guid of the item to remove for building payment
    FGuid BuildingResource{};
    UFortItemDefinition* ResourceItemDefinition = nullptr;
    
    if (is_building_class_valid(building_class))
    {
        // Check if the player has enough materials (Only if not in creative)
        if (!creative_component::is_creative_enabled() && !is_building_free)
        {
            if (ABuildingSMActor* BuildingActorToTest = CreateDefaultObject<ABuildingSMActor>(building_class))
            {
                ResourceItemDefinition = get_building_resource(BuildingActorToTest);
                if (ResourceItemDefinition)
                {
                    if (UFortItem* FoundItem = inventory_component::get_inventory_item_by_definition(building_owner, ResourceItemDefinition))
                    {
                        BuildingResource = static_cast<UFortWorldItem*>(FoundItem)->ItemEntry().ItemGuid();
                            
                        if (static_cast<UFortWorldItem*>(FoundItem)->ItemEntry().Count() < 10)
                        {
                            // Player is probably cheating with buildfree, don't spawn the building
                            return false;
                        }
                    }
                }
            }
        }
        
        if (static UEnum* EFortBuildPreviewMarkerOptionalAdjustment = FindObject<UEnum>(L"/Script/FortniteGame.EFortBuildPreviewMarkerOptionalAdjustment"))
        {
            if (MarkerOptionalAdjustment == EFortBuildPreviewMarkerOptionalAdjustment::FreeWallPieceOnTop)
            {
                FVector NewBuildLocation = building_location;
                NewBuildLocation.Z += CreateDefaultObject<ABuildingSMActor>(building_class)->VertSnapGridSize();
                TArray<AActor*> Temp_HoveredActors;
                auto Temp_Adjustment = EFortBuildPreviewMarkerOptionalAdjustment::None;
                if (CreateDefaultObject<ABuildingSMActor>(building_class)->BuildingType() == EFortBuildingType::Wall)
                {
                    if (CanPlaceBuildingActor(*GWorld, building_class, &NewBuildLocation, &building_rotation, is_mirrored, &Temp_HoveredActors, &Temp_Adjustment) == 0)
                    {
                        spawn_building_actor(building_class, NewBuildLocation, building_rotation, is_mirrored, building_owner, EFortBuildPreviewMarkerOptionalAdjustment::None, true);
                    }
                }
            }
            else if (MarkerOptionalAdjustment == EFortBuildPreviewMarkerOptionalAdjustment::FreeWallPieceOnBottom)
            {
                FVector NewBuildLocation = building_location;
                NewBuildLocation.Z -= CreateDefaultObject<ABuildingSMActor>(building_class)->VertSnapGridSize();
                TArray<AActor*> Temp_HoveredActors;
                auto Temp_Adjustment = EFortBuildPreviewMarkerOptionalAdjustment::None;
                if (CreateDefaultObject<ABuildingSMActor>(building_class)->BuildingType() == EFortBuildingType::Wall)
                {
                    if (CanPlaceBuildingActor(*GWorld, building_class, &NewBuildLocation, &building_rotation, is_mirrored, &Temp_HoveredActors, &Temp_Adjustment) == 0)
                    {
                        spawn_building_actor(building_class, NewBuildLocation, building_rotation, is_mirrored, building_owner, EFortBuildPreviewMarkerOptionalAdjustment::None, true);
                    }
                }
            }
        }
            
        // Spawn a building actor in the world
        ABuildingSMActor* building_actor = reinterpret_cast<ABuildingSMActor*>(SpawnActor(*GWorld, building_class, &building_location, &building_rotation, FActorSpawnParameters {}));

        UBuildingStructuralSupportSystem* structural_support_system = (*GWorld)->GameState()->StructuralSupportSystem();

        if (structural_support_system)
        {
            // Check if the actor can be placed
            if (structural_support_system->IsBuildingActorStructurallySupported(building_actor) == false)
            {
                //spdlog::error(L"spawn_building_actor: Building not structurally supported");
                //building_actor->SilentDie();
                //return false;
            }   
        }
        
        // Set out param, if any
        if (OutBuildingActor)
        {
            *OutBuildingActor = building_actor;
        }
        
        if (building_actor)
        {
            building_actor->ForceNetUpdate();
            
            // Set mirrored
            building_actor->SetMirrored(is_mirrored);
            
            {
                uint8_t new_team = building_owner->PlayerState()->TeamIndex();
                 
                // Only allow members of the team to edit the building actor
                building_actor->OwnerPersistentID() = building_owner->PlayerState()->WorldPlayerId();
                
                building_actor->SetTeam(new_team);
                
                building_actor->Team() = new_team;

                building_actor->OnRep_Team();
     
                // Initialize building actor
                building_actor->InitializeKismetSpawnedBuildingActor(building_actor, building_owner);

                // Pay building cost
                inventory_component::safe_update_materials_used(building_owner, ResourceItemDefinition, 10);

                inventory_component::remove_item(building_owner, BuildingResource, 10, false);

                // Count buildings placement
                static_cast<AFortPlayerControllerAthena*>(building_owner)->EraPlayerInfo().PlayerStatsTracker.TemporaryPlacedBuilds++;
                if (UKismetSystemLibrary::K2_IsTimerActive(building_owner, L"BuildingPlacedCallback"))
                {
                    UKismetSystemLibrary::K2_ClearTimer(building_owner, L"BuildingPlacedCallback");
                }
                UKismetSystemLibrary::K2_SetTimer(building_owner, L"BuildingPlacedCallback", 5.0, false);
            }
     
            return true;
        }
    }
    
    return false;
}

bool building_component::edit_building_actor(ABuildingActor* building_actor, UObject* building_class, bool is_mirrored, int rotation_iterations, bool override_class_checks)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"building_component::edit_building_actor: Editing '{}' (Is valid class: {}) with '{}' (Is valid class: {})",
            building_actor->Class->GetName().c_str(),
            is_building_class_valid(building_actor->Class),
            building_class->GetName().c_str(),
            is_building_class_valid(building_class));
    }
    
    if ((is_building_class_valid(building_class) && is_building_class_valid(building_actor->Class)) ||
        override_class_checks)
    {
        if (building_actor)
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::info(L"building_component::edit_building_actor: Passed security checks, Valid building actor target, proceeding with editing.");
            }
            
            constexpr int replacement_destruction_reason = 1;
            constexpr int building_level = 0;
            
            // Edit building actor
            auto building_actor_new = ReplaceBuildingActor(building_actor, replacement_destruction_reason, building_class, building_level, rotation_iterations, is_mirrored, nullptr);
            if (building_actor_new)
            {
                if (building_actor_new->IsA(ABuildingActor::StaticClass()))
                {
                    building_actor_new->Cast<ABuildingActor>()->ForceNetUpdate();   
                }
            }
            
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
    auto Adjustment = EFortBuildPreviewMarkerOptionalAdjustment::None;
    
    if (CanPlaceBuildingActor(*GWorld, building_class, &build_location, &build_rotation, is_mirrored, &HoveredActors, &Adjustment) == 0) // We check if it returns 0 as it actually returns an enum (EFortStructuralGridQueryResults): 0 means we can place the build
    {
        if (anticheat_component::ValidateBuildingPlacement(static_cast<AFortPlayerControllerAthena*>(FortPlayerController), build_location))
        {
            if (building_component::spawn_building_actor(building_class, build_location, build_rotation, is_mirrored, FortPlayerController, Adjustment))
            {
                // If the build was placed, destroy all Hovered Actors
                for (AActor* HoveredActor : HoveredActors)
                {
                    HoveredActor->K2_DestroyActor();
                }
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

    if (!memory_manager::is_bad_read_pointer(BuildingActorToEdit) && BuildingActorToEdit && NewBuildingClass)
    {
        // Edit building actor
        if (anticheat_component::ValidateBuildingPlacement(static_cast<AFortPlayerControllerAthena*>(FortPlayerController), BuildingActorToEdit->K2_GetActorLocation()))
        {
            // Validate building ownership
            if (!memory_manager::is_bad_read_pointer(FortPlayerController) && FortPlayerController)
            {
                if (!memory_manager::is_bad_read_pointer(FortPlayerController->PlayerState()) && FortPlayerController->PlayerState())
                {
                    if (log_component::enable_debug_logs)
                    {
                        spdlog::info(L"building_component_hooks::ServerEditBuildingActor_Hook: PlayerState ('{}', Team: '{}') is attempting to edit BuildingActor ('{}', Team: '{}')",
                            FortPlayerController->PlayerState()->GetName().c_str(),
                            FortPlayerController->PlayerState()->TeamIndex(),
                            BuildingActorToEdit->GetName().c_str(),
                            BuildingActorToEdit->Team());
                    }
                    
                    if (FortPlayerController->PlayerState()->TeamIndex() == BuildingActorToEdit->Team() || !team_component::is_human_team(BuildingActorToEdit->Team()))
                    {
                        building_component::edit_building_actor(BuildingActorToEdit, NewBuildingClass, bMirrored, RotationIterations, FortPlayerController->PlayerState()->TeamIndex() == BuildingActorToEdit->Team());
                    }
                    else
                    {
                        spdlog::error(L"Player trying to edit another team's building actor.");
                    }
                }
            }
        }
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
        BuildingActorToEdit->ForceNetUpdate();
        
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

        BuildingActorToEdit->ForceNetUpdate();
    }
    
    return ServerBeginEditingBuildingActor(FortPlayerController, StackFrame);
}

void building_component_hooks::ServerRepairBuildingActor_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(ABuildingSMActor*, BuildingActorToRepair);

    if (ProcessRepairCost)
    {
        if (int RepairCost = ProcessRepairCost(FortPlayerController, BuildingActorToRepair))
        {
            spdlog::info(L"building_component_hooks::ServerRepairBuildingActor_Hook: paid RepairCost of {}. Repairing building actor.", RepairCost);

            BuildingActorToRepair->RepairBuilding(FortPlayerController, RepairCost);
        }
    }
        
    return ServerRepairBuildingActor(FortPlayerController, StackFrame);
}
