#pragma once

class pickup_component
{
public:
    pickup_component();
    
    static AFortPickup* spawn_pickup(UFortItemDefinition* item_definition, FVector pickup_location, int count, int loaded_ammo = 0);
};