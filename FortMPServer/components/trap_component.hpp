#pragma once

class trap_component
{
public:
    trap_component();

    static ABuildingActor* CreateBuilding(AFortPlayerController* BuildingOwner, UObject** Class, FVector* Location, FRotator* Rotation);
};

namespace trap_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, ServerSpawnDeco, (AFortDecoTool* FortDecoTool, FVector* Location, FRotator* Rotation, ABuildingSMActor* AttachedActor, uint8_t InBuildingAttachmentType));
}