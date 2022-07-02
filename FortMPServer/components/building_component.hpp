#pragma once

class building_component
{
public:
    building_component();
    
    static bool spawn_building_actor(UObject* building_class, FVector building_location, FRotator building_rotation, bool is_mirrored, AFortPlayerController* building_owner = nullptr);
    static bool edit_building_actor(ABuildingActor* building_actor, UObject* building_class, bool is_mirrored, int rotation_iterations);

    static bool is_building_class_valid(UObject* building_class);
};

namespace building_component_hooks
{
    // Called when a player tries to place a building actor
    DEFINE_FUNCTION_HOOK(void, ServerCreateBuildingActor, (AFortPlayerController* FortPlayerController, FFrame& StackFrame));

    // Called when a player tries to edit a building actor
    DEFINE_FUNCTION_HOOK(void, ServerEditBuildingActor, (AFortPlayerController* FortPlayerController, FFrame& StackFrame));

    // Called when a player requests to edit a building actor
    DEFINE_FUNCTION_HOOK(void, ServerBeginEditingBuildingActor, (AFortPlayerController* FortPlayerController, FFrame& StackFrame));
    
    // Called when a player tries to repair a building actor
    DEFINE_FUNCTION_HOOK(void, ServerRepairBuildingActor, (AFortPlayerController* FortPlayerController, FFrame& StackFrame));
};

// Used to replace an existing building actor with a new building class
inline void* (*ReplaceBuildingActor)(UObject* InBuilding, uint8_t ReplacementDestructionReason, UObject* NewBuildingClass, uint8_t BuildingLevel, int RotationIterations, bool bIsMirrored, void* OutFortBuildingEditedParams);