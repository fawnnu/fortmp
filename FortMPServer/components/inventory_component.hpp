#pragma once

class inventory_component
{
public:
    inventory_component();

    static UFortWorldItem* add_item(AFortPlayerController* player_controller, UFortItemDefinition* item_definition, int item_count, int loaded_ammo = 0, std::vector<float> GenericAttributeValues = {});

    static void remove_item(AFortPlayerController* player_controller, FGuid item_guid, int item_count, bool should_drop_item = true, bool b_is_swapping = false);
    
    static void clear_primary_inventory(AFortPlayerController* fort_player_controller, bool should_drop_item = true);
    
    static bool is_inventory_full(AFortPlayerController* player_controller, int override_max_inventory_size = -1);
    static bool has_item_type(AFortPlayerController* player_controller, UFortItemDefinition* item_definition);

    static UFortItem* get_inventory_item_by_guid(AFortPlayerController* player_controller, FGuid item_guid);
    static UFortItem* get_inventory_item_by_definition(AFortPlayerController* player_controller, UFortItemDefinition* item_definition);
    static UFortItem* get_inventory_item_by_deco_definition(AFortPlayerController* player_controller, UFortItemDefinition* item_definition);

    static UFortWorldItem* get_pickaxe(AFortPlayerController* player_controller);
    
    static void UpdateWeaponLoadedAmmo(AFortWeapon* FortWeapon, FFrame& StackFrame);
};

namespace inventory_component_hooks
{
    // Called when player tries to equip an inventory item
    DEFINE_FUNCTION_HOOK(void, ServerExecuteInventoryItem, (AFortPlayerController* PlayerController, FFrame& StackFrame));

    // Called when player tries to equip an inventory weapon
    DEFINE_FUNCTION_HOOK(void, ServerExecuteInventoryWeapon, (AFortPlayerController* PlayerController, FFrame& StackFrame));

    // Called when player tries to drop item from inventory
    DEFINE_FUNCTION_HOOK(void, ServerAttemptInventoryDrop, (AFortPlayerController* PlayerController, FFrame& StackFrame));

    DEFINE_FUNCTION_HOOK(int, K2_RemoveItemFromPlayer, (class AFortPlayerController* PlayerController, class UFortWorldItemDefinition* ItemDefinition, int AmountToRemove, bool bForceRemoval));
    
    DEFINE_FUNCTION_HOOK(int, K2_RemoveItemFromPlayerByGuid, (class AFortPlayerController* PlayerController, struct FGuid ItemGuid, int AmountToRemove, bool bForceRemoval));

    DEFINE_FUNCTION_HOOK(void, GiveItemToInventoryOwner, (TScriptInterface<class UObject> InventoryOwner, class UFortWorldItemDefinition* ItemDefinition, int NumberToGive, bool bNotifyPlayer, int ItemLevel));

    DEFINE_FUNCTION_HOOK(void, ServerRemoveInventoryItem, (AFortPlayerController* PlayerController, FFrame& StackFrame));
    DEFINE_FUNCTION_HOOK(bool, RemoveInventoryItem, (void* Interface, FGuid Guid, int Count))
    inline int InterfaceToOwnerOffset;
}

inline void (*TryAddToQuickBar)(AFortPlayerController*, UFortWorldItem*);

inline void (*RemoveGadgetData)(UFortGadgetItemDefinition* ItemDefinition, void* Interface, UFortWorldItem* ItemInstance);