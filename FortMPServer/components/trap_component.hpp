#pragma once

class trap_component
{
public:
    trap_component();

    static ABuildingActor* CreateBuilding(AFortPlayerController* BuildingOwner, UObject** Class, FVector* Location, FRotator* Rotation);
};

namespace trap_component_hooks
{
    // Called when a player tries to place a trap
    DEFINE_FUNCTION_HOOK(void, ServerSpawnDeco, (AFortDecoTool* FortDecoTool, FFrame& StackFrame));
    
    DEFINE_FUNCTION_HOOK(void, ServerCreateBuildingAndSpawnDeco, (AFortDecoTool* FortDecoTool, FFrame& StackFrame));
}