#pragma once

class fort_inventory_component : public component
{
public:
    fort_inventory_component();

    static void add_item_to_inventory();
    static void remove_item_from_inventory();
    static void clear_inventory();
};