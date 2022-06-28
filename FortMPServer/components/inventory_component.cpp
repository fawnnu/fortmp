#include "include.hpp"
#include "inventory_component.hpp"

#include "pickup_component.hpp"

inventory_component::inventory_component()
{
}

UFortWorldItem* inventory_component::add_item(AFortPlayerController* player_controller, UFortItemDefinition* item_definition, int item_count, int loaded_ammo)
{
    if (player_controller->WorldInventory() == nullptr)
    {
        return nullptr;
    }
    
    TArrayManager<FFortItemEntry> replicated_entries = TArrayManager(player_controller->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());
    TArrayManager<UFortWorldItem*> item_instances = TArrayManager(player_controller->WorldInventory()->Inventory().ItemInstances(), sizeof(UFortWorldItem*));

    // Create a world item from the item definition
    UFortWorldItem* world_item = reinterpret_cast<UFortWorldItem*>(item_definition->CreateTemporaryItemInstanceBP(item_count, 1));

    for (int i = 0; i < replicated_entries.Num(); i++)
    {
        // Get both replicated entry and item instance at the index
        FFortItemEntry& replicated_entry = replicated_entries[i];
        UFortWorldItem*& item_instance = item_instances[i];
        
        if (item_definition == replicated_entry.ItemDefinition())
        {
            const int last_count = replicated_entry.Count();

            if (item_definition->MaxStackSize() - replicated_entry.Count() > 0)
            {
                // Update entry count
                replicated_entry.Count() = item_count + item_instance->ItemEntry().Count() <= item_definition->MaxStackSize() ?
                    item_count + item_instance->ItemEntry().Count() : item_definition->MaxStackSize();

                // Update instance count
                item_instance->ItemEntry().Count() = item_count + item_instance->ItemEntry().Count() <= item_definition->MaxStackSize() ?
                    item_count + item_instance->ItemEntry().Count() : item_definition->MaxStackSize();

                if (item_count + last_count > item_definition->MaxStackSize())
                {
                    if (is_inventory_full(player_controller) || !has_item_type(player_controller, item_definition))
                    {
                        // Spawn item as pickup
                        pickup_component::spawn_pickup(item_definition, player_controller->Pawn()->K2_GetActorLocation(), last_count + item_count - item_definition->MaxStackSize(), replicated_entry.LoadedAmmo());

                        // Mark for replication
                        player_controller->WorldInventory()->Inventory().MarkItemDirty(replicated_entry);

                        return nullptr;
                    }

                    // Spawn a new item
                    world_item = reinterpret_cast<UFortWorldItem*>(item_definition->CreateTemporaryItemInstanceBP(item_definition->MaxStackSize() - item_count, 1));
                }
                else
                {
                    // Mark for replication
                    player_controller->WorldInventory()->Inventory().MarkItemDirty(replicated_entry);
                    
                    return nullptr;
                }
            }
            else
            {
                if (!has_item_type(player_controller, item_definition))
                {
                    // Spawn item as pickup
                    pickup_component::spawn_pickup(item_definition, player_controller->Pawn()->K2_GetActorLocation(), item_count);

                    return nullptr;
                }
            }
        }
    }

    if (world_item == nullptr)
    {
        return nullptr;
    }

    if (loaded_ammo)
    {
        // Update loaded ammo in item entry
        world_item->ItemEntry().LoadedAmmo() = loaded_ammo;
    }

    // Add to inventory
    replicated_entries.Add(&world_item->ItemEntry());
    item_instances.Add(&world_item);

    // Update inventory locally
    player_controller->WorldInventory()->HandleInventoryLocalUpdate();
    
    // Mark for replication
    player_controller->WorldInventory()->Inventory().MarkItemDirty(world_item->ItemEntry());
    
    return world_item;
}

bool inventory_component::is_inventory_full(AFortPlayerController* player_controller)
{
    int item_count = 0;
    int maximum_item_count = 5;
    
    // List of all replicated entries in inventory
    TArrayManager<FFortItemEntry> replicated_entries = TArrayManager(player_controller->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());
    
    for (FFortItemEntry& replicated_entry : replicated_entries)
    {
        if (has_item_type(player_controller, replicated_entry.ItemDefinition()))
        {
            item_count++;
        }
    }

    return item_count == maximum_item_count;
}

bool inventory_component::has_item_type(AFortPlayerController* player_controller, UFortItemDefinition* item_definition)
{
    AFortQuickBars* player_quickbars = player_controller->HasProperty("QuickBars") ?
        player_controller->QuickBars() : player_controller->ClientQuickBars();

    // Slot for all normal inventory items (skip the harvesting tool slot)
    constexpr int quickbar_slot = 1;
    
    if (player_quickbars == nullptr)
    {
        // No quickbars exist
        return false;
    }

    // Get all slots in the primary quick bar
    TArrayManager<FQuickBarSlotData> quickbar_slots = TArrayManager(player_quickbars->PrimaryQuickBar().DataDefinition().QuickbarSlots(), FQuickBarSlotData::GetStructSize());

    //return quickbar_slots[quickbar_slot].AcceptedItemTypes().Find(item_definition->ItemType());
    return true;
}
