#pragma once

class trap_component
{
public:
    trap_component();

    static ABuildingActor* CreateBuilding(AFortPlayerController* BuildingOwner, UObject** Class, FVector* Location, FRotator* Rotation);
};