#pragma once

class inventory_component
{
public:
    inventory_component();

    static UFortWorldItem* add_item(AFortPlayerController* player_controller, UFortItemDefinition* item_definition, int Count, int LoadedAmmo = 0);
};
