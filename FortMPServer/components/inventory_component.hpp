#pragma once

class inventory_component
{
public:
    inventory_component();

    static UFortWorldItem* add_item(AFortPlayerController* player_controller, UFortItemDefinition* item_definition, int item_count, int loaded_ammo = 0);

    static bool is_inventory_full(AFortPlayerController* player_controller);
    static bool has_item_type(AFortPlayerController* player_controller, UFortItemDefinition* item_definition);
};