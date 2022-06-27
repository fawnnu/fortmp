#include "include.hpp"
#include "inventory_component.hpp"

inventory_component::inventory_component()
{
}

UFortWorldItem* inventory_component::add_item(AFortPlayerController* player_controller, UFortItemDefinition* item_definition, int Count,
                                   int LoadedAmmo)
{
    if (player_controller->WorldInventory() == nullptr)
    {
        return nullptr;
    }

    UFortWorldItem* world_item = reinterpret_cast<UFortWorldItem*>(item_definition->CreateTemporaryItemInstanceBP(Count, 1));
    
    return world_item;
}
