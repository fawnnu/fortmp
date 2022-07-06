#include "include.hpp"
#include "inventory_component.hpp"

#include "pickup_component.hpp"

inventory_component::inventory_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(inventory_component_hooks::ServerExecuteInventoryItem, L"/Script/FortniteGame.FortPlayerController.ServerExecuteInventoryItem", inventory_component_hooks::ServerExecuteInventoryItem_Hook);
    REGISTER_UFUNCTION_DETOUR(inventory_component_hooks::ServerExecuteInventoryWeapon, L"/Script/FortniteGame.FortPlayerController.ServerExecuteInventoryWeapon", inventory_component_hooks::ServerExecuteInventoryWeapon_Hook);
    REGISTER_UFUNCTION_DETOUR(inventory_component_hooks::ServerAttemptInventoryDrop, L"/Script/FortniteGame.FortPlayerController.ServerAttemptInventoryDrop", inventory_component_hooks::ServerAttemptInventoryDrop_Hook);
    
    DetourTransactionCommit();
}

UFortWorldItem* inventory_component::add_item(AFortPlayerController* player_controller, UFortItemDefinition* item_definition, int item_count, int loaded_ammo)
{
    if (player_controller->WorldInventory() == nullptr)
    {
        return nullptr;
    }
    
    auto replicated_entries = TArrayManager<FFortItemEntry>(&player_controller->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());
    auto item_instances = TArrayManager<UFortWorldItem*>(&player_controller->WorldInventory()->Inventory().ItemInstances(), sizeof(UFortWorldItem*));

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
            int MaxStackSize;

            // On chapter 2 MaxStackSize is a FScalableFloat. To avoid confusion within the UFortItemDefinition class, use this function instead
            if (static UFunction* GetMaxStackSize = FindObject<UFunction>(L"/Script/FortniteGame.FortItem.GetMaxStackSize"))
            {
                MaxStackSize = item_instance->GetMaxStackSize(player_controller->Pawn()->AbilitySystemComponent());
            }
            else
            {
                MaxStackSize = item_definition->MaxStackSize();
            }

            if (MaxStackSize - replicated_entry.Count() > 0)
            {
                // Update entry count
                replicated_entry.Count() = item_count + item_instance->ItemEntry().Count() <= MaxStackSize ?
                    item_count + item_instance->ItemEntry().Count() : MaxStackSize;

                // Update instance count
                item_instance->ItemEntry().Count() = item_count + item_instance->ItemEntry().Count() <= MaxStackSize ?
                    item_count + item_instance->ItemEntry().Count() : MaxStackSize;

                if (item_count + last_count > MaxStackSize)
                {
                    if (is_inventory_full(player_controller) || !has_item_type(player_controller, item_definition))
                    {
                        // Spawn item as pickup
                        pickup_component::spawn_pickup(item_definition, player_controller->Pawn()->K2_GetActorLocation(), last_count + item_count - MaxStackSize, replicated_entry.LoadedAmmo());

                        // Mark for replication
                        player_controller->WorldInventory()->Inventory().MarkItemDirty(replicated_entry);

                        return nullptr;
                    }

                    // Spawn a new item
                    world_item = reinterpret_cast<UFortWorldItem*>(item_definition->CreateTemporaryItemInstanceBP(MaxStackSize - item_count, 1));
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

void inventory_component::remove_item(AFortPlayerController* player_controller, FGuid item_guid, int item_count, bool should_drop_item)
{
    if (!player_controller->Pawn() && should_drop_item)
    {
        // No pawn to spawn item at
        return;
    }

    TArrayManager replicated_entries = TArrayManager<FFortItemEntry>(&player_controller->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());
    TArrayManager item_instances = TArrayManager<UFortWorldItem*>(&player_controller->WorldInventory()->Inventory().ItemInstances(), sizeof(UFortWorldItem*));

    for (int i = 0; i < replicated_entries.Num(); i++)
    {
        // Get both replicated entry and item instance at the index
        FFortItemEntry& replicated_entry = replicated_entries[i];
        UFortWorldItem*& item_instance = item_instances[i];

        if (replicated_entry.ItemGuid() == item_guid)
        {
            if (should_drop_item)
            {
                // Spawn dropped item as pickup
                pickup_component::spawn_pickup(replicated_entry.ItemDefinition(), player_controller->Pawn()->K2_GetActorLocation(), item_count > 0 ? item_count : replicated_entry.Count(), replicated_entry.LoadedAmmo());
            }

            if (item_count > 0 && replicated_entry.Count() - item_count > 0)
            {
                // Update item entry count
                replicated_entry.Count() -= item_count;
                item_instance->ItemEntry().Count() -= item_count;

                // Mark for replication
                player_controller->WorldInventory()->Inventory().MarkItemDirty(replicated_entry);
                
                return;
            }

            // Cache array classes
            static void* replicated_entries_property = FindPropertyByName(FFortItemList::GetClass(), L"ReplicatedEntries");
            static void* item_instances_property = FindPropertyByName(FFortItemList::GetClass(), L"ItemInstances");

            replicated_entries.Remove(i, replicated_entries_property);
            item_instances.Remove(i, item_instances_property);

            // Mark for replication
            player_controller->WorldInventory()->Inventory().MarkArrayDirty();

            break;
        }
    }
}

bool inventory_component::is_inventory_full(AFortPlayerController* player_controller)
{
    int item_count = 0;
    constexpr int maximum_item_count = 6;
    
    // List of all replicated entries in inventory
    TArrayManager replicated_entries = TArrayManager<FFortItemEntry>(&player_controller->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());
    
    for (FFortItemEntry& replicated_entry : replicated_entries)
    {
        if (has_item_type(player_controller, replicated_entry.ItemDefinition()))
        {
            item_count++;
        }
    }

    return item_count >= maximum_item_count;
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
    TArrayManager quickbar_slots = TArrayManager<FQuickBarSlotData>(&player_quickbars->PrimaryQuickBar().DataDefinition().QuickbarSlots(), FQuickBarSlotData::GetStructSize());

    return quickbar_slots[quickbar_slot].AcceptedItemTypes().Find(item_definition->ItemType());
}

void inventory_component_hooks::ServerExecuteInventoryItem_Hook(AFortPlayerController* PlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(FGuid, ItemGuid);

    // Get inventory item by GUID
    UFortItem* fort_item = PlayerController->HasFunction("K2_GetInventoryItemWithGuid") ?
        PlayerController->K2_GetInventoryItemWithGuid(ItemGuid) : PlayerController->BP_GetInventoryItemWithGuid(ItemGuid);

    if (fort_item == nullptr)
    {
        return;
    }

    // Get item definition from inventory item
    UFortItemDefinition* item_definition = fort_item->GetItemDefinitionBP();

    if (item_definition == nullptr)
    {
        return;
    }

    // Support trap logic
    static UObject* trap_definition_class = FindObject(L"/Script/FortniteGame.FortTrapItemDefinition");
    static UObject* context_trap_class = FindObject(L"/Script/FortniteGame.FortDecoTool_ContextTrap");

    if (UKismetMathLibrary::ClassIsChildOf(item_definition->Class, trap_definition_class))
    {
        // Equip trap
        PlayerController->Pawn()->PickUpActor(nullptr, reinterpret_cast<UFortDecoItemDefinition*>(item_definition));

        if (PlayerController->Pawn()->CurrentWeapon())
        {
            // Update entry GUID in current weapon
            PlayerController->Pawn()->CurrentWeapon()->ItemEntryGuid() = ItemGuid;
            
            if (UKismetMathLibrary::ClassIsChildOf(PlayerController->Pawn()->CurrentWeapon()->Class, context_trap_class))
            {
                // Update item definition if we have a context trap
                reinterpret_cast<AFortDecoTool_ContextTrap*>(PlayerController->Pawn()->CurrentWeapon())->ContextTrapItemDefinition() = item_definition;
            }
        }
    }
    
    // Equip weapon
    PlayerController->Pawn()->EquipWeaponDefinition(item_definition, ItemGuid);
    
    return ServerExecuteInventoryItem(PlayerController, StackFrame);
}

void inventory_component_hooks::ServerExecuteInventoryWeapon_Hook(AFortPlayerController* PlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(AFortWeapon*, Weapon);

    // Equip weapon
    PlayerController->Pawn()->EquipWeaponDefinition(Weapon->WeaponData(), Weapon->ItemEntryGuid());
    
    return ServerExecuteInventoryWeapon(PlayerController, StackFrame);
}

void inventory_component_hooks::ServerAttemptInventoryDrop_Hook(AFortPlayerController* PlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(FGuid, ItemGuid);
    GET_PARAMETER(int, Count);

    // Remove item
    inventory_component::remove_item(PlayerController, ItemGuid, Count);
    
    return ServerAttemptInventoryDrop(PlayerController, StackFrame);
}
