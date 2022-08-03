#include "include.hpp"
#include "inventory_component.hpp"

#include "pickup_component.hpp"

#include "ability_component.hpp"

inventory_component::inventory_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(inventory_component_hooks::ServerExecuteInventoryItem, L"/Script/FortniteGame.FortPlayerController.ServerExecuteInventoryItem", inventory_component_hooks::ServerExecuteInventoryItem_Hook);
    REGISTER_UFUNCTION_DETOUR(inventory_component_hooks::ServerExecuteInventoryWeapon, L"/Script/FortniteGame.FortPlayerController.ServerExecuteInventoryWeapon", inventory_component_hooks::ServerExecuteInventoryWeapon_Hook);
    REGISTER_UFUNCTION_DETOUR(inventory_component_hooks::ServerAttemptInventoryDrop, L"/Script/FortniteGame.FortPlayerController.ServerAttemptInventoryDrop", inventory_component_hooks::ServerAttemptInventoryDrop_Hook);
    REGISTER_UFUNCTION_DETOUR(inventory_component_hooks::ServerAttemptInventoryDrop, L"/Script/FortniteGame.FortPlayerController.ServerSpawnInventoryDrop", inventory_component_hooks::ServerAttemptInventoryDrop_Hook);
    
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
    
    // Gadgets implementation
    static UObject* gadget_definition_class = FindObject(L"/Script/FortniteGame.FortGadgetItemDefinition", true);
    if (UKismetMathLibrary::ClassIsChildOf(item_definition->Class, gadget_definition_class))
    {
        UFortGadgetItemDefinition* gadget_item_definition = static_cast<UFortGadgetItemDefinition*>(item_definition);
        
        // Apply characrer parts
        for (UCustomCharacterPart* character_part : gadget_item_definition->CharacterParts())
        {
            player_controller->Pawn()->ServerChoosePart(character_part->CharacterPartType(), character_part);
        }

        // Apply Gadget Ability set
        UObject* ability_set = gadget_item_definition->AbilitySet().Get();
        
        if(ability_set)
        {
            ability_component::apply_ability_set(player_controller->Pawn(), ability_set);
        }
    }
    
    // Add to inventory
    replicated_entries.Add(&world_item->ItemEntry());
    item_instances.Add(&world_item);

    // Support force quickbar focus when item gets added to inventory
    UFortWorldItemDefinition* world_item_definition = static_cast<UFortWorldItemDefinition*>(world_item->ItemEntry().ItemDefinition());

    if(world_item_definition != nullptr)
    {
        if (static bool b_has_property = world_item_definition->HasProperty(L"bForceFocusWhenAdded"))
        {
            if(world_item_definition->bForceFocusWhenAdded() == true)
            {
                player_controller->ClientEquipItem(world_item->GetItemGuid(), true);
            }
        }
    }
    
    // Update inventory locally
    player_controller->WorldInventory()->HandleInventoryLocalUpdate();
    
    // Mark for replication
    player_controller->WorldInventory()->Inventory().MarkItemDirty(world_item->ItemEntry());

    // has to be done on older builds (items aren't added to quickbars by default)
    static bool b_legacy_quickbars = player_controller->HasProperty(L"QuickBars");
    
    if (b_legacy_quickbars)
    {
        player_controller->QuickBars()->ServerAddItemInternal(item_definition, world_item->ItemEntry().ItemGuid());
    }
    
    return world_item;
}

void inventory_component::remove_item(AFortPlayerController* player_controller, FGuid item_guid, int item_count, bool should_drop_item, bool b_is_swapping)
{
    // TODO: Add logic for character parts reverting, if player is removing a gadget from inventory.
    
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

            // has to be done on older builds
            static bool b_legacy_quickbars = player_controller->HasProperty(L"QuickBars");
            
            if (b_legacy_quickbars)
            {
                player_controller->QuickBars()->ServerRemoveItemInternal(item_guid, b_is_swapping);
            }

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

UFortItem* inventory_component::get_inventory_item_by_guid(AFortPlayerController* player_controller, FGuid item_guid)
{
    for (UFortItem* ItemInstance : player_controller->WorldInventory()->Inventory().ItemInstances())
    {
        if (static_cast<UFortWorldItem*>(ItemInstance)->ItemEntry().ItemGuid() == item_guid)
        {
            return ItemInstance;
        }
    }

    return nullptr;
}

UFortItem* inventory_component::get_inventory_item_by_definition(AFortPlayerController* player_controller, UFortItemDefinition* item_definition)
{
    for (UFortItem* ItemInstance : player_controller->WorldInventory()->Inventory().ItemInstances())
    {
        if (static_cast<UFortWorldItem*>(ItemInstance)->ItemEntry().ItemDefinition() == item_definition)
        {
            return ItemInstance;
        }
    }
    
    return nullptr;    
}

void inventory_component_hooks::ServerExecuteInventoryItem_Hook(AFortPlayerController* PlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(FGuid, ItemGuid);

    if (!PlayerController->Pawn())
    {
        return;
    }

    // Get inventory item by GUID
    UFortItem* fort_item = PlayerController->HasFunction("K2_GetInventoryItemWithGuid") ?
        PlayerController->K2_GetInventoryItemWithGuid(ItemGuid) : PlayerController->HasFunction("BP_GetInventoryItemWithGuid") ?
        PlayerController->BP_GetInventoryItemWithGuid(ItemGuid) : inventory_component::get_inventory_item_by_guid(PlayerController, ItemGuid);

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

    // Support gadgets logic
    static UObject* gadget_definition_class = FindObject(L"/Script/FortniteGame.FortGadgetItemDefinition");
    
    if (UKismetMathLibrary::ClassIsChildOf(item_definition->Class, gadget_definition_class))
    {
        // If item is a gadget, equip it's deco item definition
        PlayerController->Pawn()->EquipWeaponDefinition(static_cast<UFortGadgetItemDefinition*>(item_definition)->GetDecoItemDefinition(), ItemGuid); // TODO: Add a check to use GetWeaponItemDefinition on later builds
    }
    else
    {
        // Equip Weapon
        PlayerController->Pawn()->EquipWeaponDefinition(item_definition, ItemGuid);
    }

    static bool b_set_metadata = GetEngineVersionEasy() < 420;

    if (b_set_metadata)
    {
        if (AFortWeapon* CurrentWeapon = PlayerController->Pawn()->CurrentWeapon())
        {
            if (CurrentWeapon->HasProperty(L"DefaultMetadata"))
            {
                UBuildingEditModeMetadata*& Metadata = reinterpret_cast<AFortWeap_BuildingTool*>(CurrentWeapon)->DefaultMetadata();

                static UFortItemDefinition* Roof = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_RoofS.BuildingItemData_RoofS");
                static UFortItemDefinition* Stairs = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_Stair_W.BuildingItemData_Stair_W");
                static UFortItemDefinition* Floor = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_Floor.BuildingItemData_Floor");
                static UFortItemDefinition* Wall = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_Wall.BuildingItemData_Wall");

                static UBuildingEditModeMetadata* RoofMetadata = FindObject<UBuildingEditModeMetadata>(L"/Game/Building/EditModePatterns/Roof/EMP_Roof_RoofC.EMP_Roof_RoofC");
                static UBuildingEditModeMetadata* StairsMetadata = FindObject<UBuildingEditModeMetadata>(L"/Game/Building/EditModePatterns/Stair/EMP_Stair_StairW.EMP_Stair_StairW");
                static UBuildingEditModeMetadata* FloorMetadata = FindObject<UBuildingEditModeMetadata>(L"/Game/Building/EditModePatterns/Floor/EMP_Floor_Floor.EMP_Floor_Floor");
                static UBuildingEditModeMetadata* WallMetadata = FindObject<UBuildingEditModeMetadata>(L"/Game/Building/EditModePatterns/Wall/EMP_Wall_Solid.EMP_Wall_Solid");

                if (fort_item->GetItemDefinitionBP() == Wall)
                {
                    Metadata = WallMetadata;
                }
                else if (fort_item->GetItemDefinitionBP() == Floor)
                {
                    Metadata = FloorMetadata;
                }
                else if (fort_item->GetItemDefinitionBP() == Stairs)
                {
                    Metadata = StairsMetadata;
                }
                else if (fort_item->GetItemDefinitionBP() == Roof)
                {
                    Metadata = RoofMetadata;
                }

                reinterpret_cast<AFortWeap_BuildingTool*>(CurrentWeapon)->OnRep_DefaultMetadata();
            }
        }
    }
    
    return ServerExecuteInventoryItem(PlayerController, StackFrame);
}

void inventory_component_hooks::ServerExecuteInventoryWeapon_Hook(AFortPlayerController* PlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(AFortWeapon*, Weapon);

    if (Weapon != nullptr && PlayerController->Pawn() != nullptr)
    {
        // Equip weapon
        PlayerController->Pawn()->EquipWeaponDefinition(Weapon->WeaponData(), Weapon->ItemEntryGuid());   
    }
    
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
