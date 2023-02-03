#include "include.hpp"
#include "inventory_component.hpp"

#include "pickup_component.hpp"

#include "ability_component.hpp"
#include "function_component.hpp"

inventory_component::inventory_component()
{
    // We use RemoveGadgetData to remove gadget data such as abilities, CPs, attributes.
    INIT_POINTER(RemoveGadgetData, memory_manager::find_function_by_string(L"UFortGadgetItemDefinition::RemoveGadgetData"));
    
    // We use TryAddToQuickBar to have quickbars working below UE 4.20 builds
    INIT_POINTER(TryAddToQuickBar, memory_manager::find_function_by_string(L"TryAddToQuickBar"));
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(inventory_component_hooks::ServerExecuteInventoryItem, L"/Script/FortniteGame.FortPlayerController.ServerExecuteInventoryItem", inventory_component_hooks::ServerExecuteInventoryItem_Hook);
    REGISTER_UFUNCTION_DETOUR(inventory_component_hooks::ServerExecuteInventoryWeapon, L"/Script/FortniteGame.FortPlayerController.ServerExecuteInventoryWeapon", inventory_component_hooks::ServerExecuteInventoryWeapon_Hook);
    REGISTER_UFUNCTION_DETOUR(inventory_component_hooks::ServerAttemptInventoryDrop, L"/Script/FortniteGame.FortPlayerController.ServerAttemptInventoryDrop", inventory_component_hooks::ServerAttemptInventoryDrop_Hook);
    REGISTER_UFUNCTION_DETOUR(inventory_component_hooks::ServerAttemptInventoryDrop, L"/Script/FortniteGame.FortPlayerController.ServerSpawnInventoryDrop", inventory_component_hooks::ServerAttemptInventoryDrop_Hook);
    
    DetourTransactionCommit();

    REGISTER_CALL_DETOUR(L"/Script/FortniteGame.FortKismetLibrary.K2_RemoveItemFromPlayer", inventory_component_hooks::K2_RemoveItemFromPlayer_Hook);

    REGISTER_CALL_DETOUR(L"/Script/FortniteGame.FortKismetLibrary.K2_RemoveItemFromPlayerByGuid", inventory_component_hooks::K2_RemoveItemFromPlayerByGuid_Hook);

    REGISTER_CALL_DETOUR(L"/Script/FortniteGame.FortKismetLibrary.GiveItemToInventoryOwner", inventory_component_hooks::GiveItemToInventoryOwner_Hook);

    // Create custom function
    REGISTER_CUSTOM_UFUNCTION(UpdateWeaponLoadedAmmo, AFortWeapon::StaticClass());
}

void inventory_component::clear_primary_inventory(AFortPlayerController* fort_player_controller, bool should_drop_item)
{
    auto replicated_entries = TArrayManager<FFortItemEntry>(&fort_player_controller->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());

    static UEnum* EFortInventoryFilter = FindObject<UEnum>(L"/Script/FortniteGame.EFortInventoryFilter");
    
    for (int i = replicated_entries.Num() - 1; i >= 0; i--)
    {
        FFortItemEntry& item_entry = replicated_entries[i];

        if (item_entry.ItemDefinition()->FilterOverride() == EFortInventoryFilter->GetEnumIndex(L"Invisible") || item_entry.Count() < 1)
        {
            continue;
        }

        // Remove item from player
        remove_item(fort_player_controller, item_entry.ItemGuid(), -1, should_drop_item);
    }   
}

UFortWorldItem* inventory_component::add_item(AFortPlayerController* player_controller, UFortItemDefinition* item_definition, int item_count, int loaded_ammo)
{
    if (player_controller->WorldInventory() == nullptr)
    {
        return nullptr;
    }

    if (item_definition->IsA(UFortGadgetItemDefinition::StaticClass()))
    {
        if (static_cast<UFortGadgetItemDefinition*>(item_definition)->bDropAllOnEquip())
        {
            // Drop all the items
            clear_primary_inventory(player_controller, true);
        }
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
                
                // Mark for replication
                player_controller->WorldInventory()->Inventory().MarkItemDirty(replicated_entry);

                if (item_count + last_count > MaxStackSize)
                {
                    if (is_inventory_full(player_controller, 5) || !has_item_type(player_controller, item_definition))
                    {
                        // Spawn item as pickup
                        pickup_component::spawn_pickup(item_definition, player_controller->Pawn()->K2_GetActorLocation(), last_count + item_count - MaxStackSize, replicated_entry.LoadedAmmo());

                        return nullptr;
                    }

                    // Spawn a new item
                    world_item = reinterpret_cast<UFortWorldItem*>(item_definition->CreateTemporaryItemInstanceBP(last_count + item_count - MaxStackSize, 1));
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
    
    // Update inventory locally
    player_controller->WorldInventory()->HandleInventoryLocalUpdate();

    // Add to state values
    TArrayManager<FFortItemEntryStateValue> StateValues = TArrayManager<FFortItemEntryStateValue>(&replicated_entries[replicated_entries.Num() - 1].StateValues(), FFortItemEntryStateValue::GetStructSize());

    // Make a new state value
    FFortItemEntryStateValue* NewStateValue = reinterpret_cast<FFortItemEntryStateValue*>(alloca(FFortItemEntryStateValue::GetStructSize()));

    // Initialize the newly allocated memory
    ZeroMemory(NewStateValue, FFortItemEntryStateValue::GetStructSize());

    // Initialize the properties
    NewStateValue->IntValue() = 1;
    NewStateValue->StateType() = 2;

    // Add it to the StateValues
    StateValues.Add(NewStateValue);
    
    // Support force quickbar focus when item gets added to inventory
    if (STATIC_HASPROPERTY(item_definition, bForceFocusWhenAdded))
    {
        if(static_cast<UFortWorldItemDefinition*>(item_definition)->bForceFocusWhenAdded() == true)
        {
            player_controller->ClientEquipItem(world_item->GetItemGuid(), true);
        }
    }
    
    // Mark for replication
    player_controller->WorldInventory()->Inventory().MarkItemDirty(world_item->ItemEntry());

    if (TryAddToQuickBar)
    {
        TryAddToQuickBar(player_controller, world_item);
    }
    
    return world_item;
}

void inventory_component::remove_item(AFortPlayerController* player_controller, FGuid item_guid, int item_count, bool should_drop_item, bool b_is_swapping)
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
            if (item_count > replicated_entry.Count())
            {
                spdlog::error(L"Player '{}' tried to drop more items than they have, aborting.", player_controller->MyPlayerInfo()->PlayerName().c_str());
                
                return;
            }
            
            UFortWorldItemDefinition* world_item_definition = static_cast<UFortWorldItemDefinition*>(replicated_entry.ItemDefinition());
            
            if (should_drop_item && world_item_definition->bCanBeDropped())
            {
                // Spawn dropped item as pickup
                pickup_component::spawn_pickup(replicated_entry.ItemDefinition(), player_controller->Pawn()->K2_GetActorLocation(), item_count > 0 ? item_count : replicated_entry.Count(), replicated_entry.LoadedAmmo());
            }

            if (STATIC_HASPROPERTY(world_item_definition, bPersistInInventoryWhenFinalStackEmpty))
            {
                if(world_item_definition->bPersistInInventoryWhenFinalStackEmpty() == true)
                {
                    if(replicated_entry.Count() == 1)
                    {
                        // Update item entry count
                        replicated_entry.Count() = 0;
                        item_instance->ItemEntry().Count() = 0;
                
                        // Mark for replication
                        player_controller->WorldInventory()->Inventory().MarkItemDirty(replicated_entry);

                        return;
                    }
                }
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

            // If it's an AGID, remove it accordingly
            if (item_instance->ItemEntry().ItemDefinition()->IsA(UFortGadgetItemDefinition::StaticClass()))
            {
                RemoveGadgetData(static_cast<UFortGadgetItemDefinition*>(item_instance->ItemEntry().ItemDefinition()), player_controller->GetInterfaceAddress(IFortInventoryOwnerInterface::StaticClass()), item_instance);
            }
            
            // Cache array classes
            static void* replicated_entries_property = FindPropertyByName(FFortItemList::GetClass(), L"ReplicatedEntries");
            static void* item_instances_property = FindPropertyByName(FFortItemList::GetClass(), L"ItemInstances");

            replicated_entries.Remove(i, replicated_entries_property);
            item_instances.Remove(i, item_instances_property);

            // Mark for replication
            player_controller->WorldInventory()->Inventory().MarkArrayDirty();

            if (item_instance->ItemEntry().ItemDefinition()->IsA(UFortGadgetItemDefinition::StaticClass()) && player_controller->Pawn() != nullptr)
            {
                // If it's an AGID, remove the character parts (Has to be done after removing from inventory)
                player_controller->PlayerState()->ApplyCharacterCustomization(player_controller->Pawn());
            }
            
            if (STATIC_HASPROPERTY(player_controller, QuickBars))
            {
                player_controller->QuickBars()->ServerRemoveItemInternal(item_guid, b_is_swapping);
            }

            break;
        }
    }
}

bool inventory_component::is_inventory_full(AFortPlayerController* player_controller, int override_max_inventory_size)
{
    int item_count = 0;
    const int maximum_item_count = override_max_inventory_size == -1 ? 6 : override_max_inventory_size;
    
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
    STATIC_HASPROPERTY(player_controller, QuickBars);
    
    AFortQuickBars* player_quickbars = bExists_QuickBars ?
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
    
    if (item_definition->IsA(trap_definition_class))
    {
        // Equip trap
        PlayerController->Pawn()->PickUpActor(nullptr, reinterpret_cast<UFortDecoItemDefinition*>(item_definition));

        if (PlayerController->Pawn()->CurrentWeapon())
        {
            // Update entry GUID in current weapon
            PlayerController->Pawn()->CurrentWeapon()->ItemEntryGuid() = ItemGuid;
            
            if (PlayerController->Pawn()->CurrentWeapon()->IsA(context_trap_class))
            {
                // Update item definition if we have a context trap
                reinterpret_cast<AFortDecoTool_ContextTrap*>(PlayerController->Pawn()->CurrentWeapon())->ContextTrapItemDefinition() = item_definition;
            }
        }
    }
    
    if (item_definition->IsA(UFortGadgetItemDefinition::StaticClass()))
    {
        PlayerController->Pawn()->PickUpActor(nullptr, reinterpret_cast<UFortDecoItemDefinition*>(static_cast<UFortGadgetItemDefinition*>(item_definition)->GetDecoItemDefinition()));

        if (PlayerController->Pawn()->CurrentWeapon())
        {
            // Update entry GUID in current weapon
            PlayerController->Pawn()->CurrentWeapon()->ItemEntryGuid() = ItemGuid;
        }
    }
    else
    {
        // Equip Weapon
        auto NewWeapon = PlayerController->Pawn()->EquipWeaponDefinition(item_definition, ItemGuid);

        // Bind a delegate to keep loadedammo updated
        if (NewWeapon != nullptr)
        {
            NewWeapon->OnLocalAmmoChanged().Bind(TScriptDelegate(NewWeapon, L"UpdateWeaponLoadedAmmo"));
        }
    }

    static bool b_set_metadata = GetEngineVersionEasy() < 421;

    if (b_set_metadata)
    {
        if (AFortWeapon* CurrentWeapon = PlayerController->Pawn()->CurrentWeapon())
        {
            if (CurrentWeapon->IsA(AFortWeap_BuildingTool::StaticClass()))
            {
                if (STATIC_HASPROPERTY(CurrentWeapon, DefaultMetadata))
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
    }
    
    return ServerExecuteInventoryItem(PlayerController, StackFrame);
}

UFortWorldItem* inventory_component::get_pickaxe(AFortPlayerController* player_controller)
{
    if (player_controller == nullptr || player_controller->WorldInventory() == nullptr)
    {
        return nullptr;
    }
    
    auto item_instances = TArrayManager<UFortWorldItem*>(&player_controller->WorldInventory()->Inventory().ItemInstances(), sizeof(UFortWorldItem*));

    for (int i = 0; i < item_instances.Num(); i++)
    {
        UFortWorldItem* CurrentItemInstance = item_instances[i];

        if (CurrentItemInstance && CurrentItemInstance->ItemEntry().ItemDefinition()->IsA(UFortWeaponMeleeItemDefinition::StaticClass()))
        {
            return CurrentItemInstance;
        }
    }
    
    return nullptr;   
}

void inventory_component::UpdateWeaponLoadedAmmo(AFortWeapon* FortWeapon, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    const auto player_controller = static_cast<AFortPawn*>(FortWeapon->GetOwner())->Controller();

    if (player_controller == nullptr)
    {
        return;
    }

    if (player_controller->WorldInventory() == nullptr)
    {
        return;
    }

    auto replicated_entries = TArrayManager<FFortItemEntry>(&player_controller->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());
    auto item_instances = TArrayManager<UFortWorldItem*>(&player_controller->WorldInventory()->Inventory().ItemInstances(), sizeof(UFortWorldItem*));

    for (int i = 0; i < replicated_entries.Num(); i++)
    {
        FFortItemEntry& replicated_entry = replicated_entries[i];
        UFortWorldItem*& item_instance = item_instances[i];

        if (replicated_entry.ItemGuid() == FortWeapon->ItemEntryGuid())
        {
            replicated_entry.LoadedAmmo() = FortWeapon->GetLocalMagazineAmmoCount();
            item_instance->ItemEntry().LoadedAmmo() = FortWeapon->GetLocalMagazineAmmoCount();

            player_controller->WorldInventory()->Inventory().MarkItemDirty(replicated_entry);

            break;
        }
    }
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

int inventory_component_hooks::K2_RemoveItemFromPlayer_Hook(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int AmountToRemove, bool bForceRemoval)
{
    if (UFortWorldItem* ItemToRemove = static_cast<UFortWorldItem*>(inventory_component::get_inventory_item_by_definition(PlayerController, ItemDefinition)))
    {
        inventory_component::remove_item(PlayerController, ItemToRemove->ItemEntry().ItemGuid(), AmountToRemove, false);
    }
    
    return 1;
}

int inventory_component_hooks::K2_RemoveItemFromPlayerByGuid_Hook(AFortPlayerController* PlayerController, FGuid ItemGuid, int AmountToRemove, bool bForceRemoval)
{
    inventory_component::remove_item(PlayerController, ItemGuid, AmountToRemove, false);
    
    return 1;
}

void inventory_component_hooks::GiveItemToInventoryOwner_Hook(TScriptInterface<UObject> InventoryOwner, UFortWorldItemDefinition* ItemDefinition, int NumberToGive, bool bNotifyPlayer, int ItemLevel)
{
    if (AFortPlayerController* PlayerController = static_cast<AFortPlayerController*>(InventoryOwner.GetObjectRef()))
    {
        inventory_component::add_item(PlayerController, ItemDefinition, NumberToGive);
    }
}