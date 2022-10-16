#include "include.hpp"
#include "ability_component.hpp"

#include "inventory_component.hpp"

ability_component::ability_component()
{
    INIT_POINTER(GHandle, memory_manager::find_pattern("FF 05 ? ? ? ? 8B 43 0C 41 89 86 ? ? ? ? 48 8B 84 24 ? ? ? ? 48 89 43 70 E8 ? ? ? ?", true, 2));
    
    INIT_POINTER(InternalTryActivateAbility, memory_manager::find_pattern("4C 89 4C 24 ? 4C 89 44 24 ? 89 54 24 10 55 53 56 57 41 54 41 57 48 8D AC 24 ? ? ? ?"));
    INIT_POINTER(InternalTryActivateAbility, memory_manager::find_function_by_string(L"InternalTryActivateAbility called with invalid"));
    INIT_POINTER(InternalTryActivateAbility, memory_manager::find_pattern("4C 89 4C 24 ? 4C 89 44 24 ? 89 54 24 10 55 56 41 54 41 55 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ?"));

    INIT_POINTER(MarkAbilitySpecDirty, memory_manager::find_pattern("48 89 5C 24 ? 48 89 7C 24 ? 41 56 48 83 EC 20 48 8B 01 41 0F B6 ? 4C 8B F2 48 8B ? FF 90 ? ? ? ? 84 C0 0F 84 ? ? ? ?"));
    INIT_POINTER(MarkAbilitySpecDirty, memory_manager::find_pattern("48 89 5C 24 ? 57 48 83 EC 20 80 B9 ? ? ? ? ? 48 8B FA 48 8B D9 75 4A C6 81 ? ? ? ? ? 48 81 C1 ? ? ? ? 83 3A FF 75 16 FF 41 50"));
    INIT_POINTER(MarkAbilitySpecDirty, memory_manager::find_pattern("E8 ? ? ? ? 48 8B 86 ? ? ? ? 48 85 C0 75 08 48 8B CE E8 ? ? ? ? 8B 80 ? ? ? ? 89 86 ? ? ? ? B0 01", true, 1));

    INIT_POINTER(GiveAbilityAndActivateOnce, memory_manager::find_function_by_string(L"GiveAbilityAndActivateOnce called on ability %s that"));
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(ability_component_hooks::ServerTryActivateAbility, L"/Script/GameplayAbilities.AbilitySystemComponent.ServerTryActivateAbility", ability_component_hooks::ServerTryActivateAbility_Hook);
    REGISTER_UFUNCTION_DETOUR(ability_component_hooks::ServerTryActivateAbilityWithEventData, L"/Script/GameplayAbilities.AbilitySystemComponent.ServerTryActivateAbilityWithEventData", ability_component_hooks::ServerTryActivateAbilityWithEventData_Hook);
    REGISTER_UFUNCTION_DETOUR(ability_component_hooks::ServerAbilityRPCBatch, L"/Script/GameplayAbilities.AbilitySystemComponent.ServerAbilityRPCBatch", ability_component_hooks::ServerAbilityRPCBatch_Hook);
    
    REGISTER_FUNCTION_DETOUR(ability_component_hooks::AbilityCost, memory_manager::find_pattern("89 54 24 10 55 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 80 B9 ? ? ? ? ? 4C 8B F1 0F 85 ? ? ? ? 48 8B 01 FF 90 ? ? ? ? 84 C0 0F 84 ? ? ? ? 48 89 9C 24 ? ? ? ? 49 8B CE 48 89 BC 24 ? ? ? ? E8 ? ? ? ? 48 89 45 7F"), ability_component_hooks::AbilityCost_Hook);
    REGISTER_FUNCTION_DETOUR(ability_component_hooks::AbilityCost, memory_manager::find_function_by_string(L"Failed to remove item %s during reload, item duplic"), ability_component_hooks::AbilityCost_Hook);
    REGISTER_FUNCTION_DETOUR(ability_component_hooks::AbilityCost, memory_manager::find_function_by_string(L"Weapon [%s] - Failed to find an Inventory Owner"), ability_component_hooks::AbilityCost_Hook);

    REGISTER_FUNCTION_DETOUR(ability_component_hooks::DoesAbilitySatisfyTagRequirements, memory_manager::find_pattern("E8 ? ? ? ? 84 C0 74 43 8B 55 5F", true, 1), ability_component_hooks::DoesAbilitySatisfyTagRequirements_Hook);
    
    DetourTransactionCommit();
}

void ability_component::activate_ability(UAbilitySystemComponent* ability_system_component, FGameplayAbilitySpecHandle ability_handle,
    bool input_pressed, const FPredictionKey& prediction_key, FGameplayEventData* event_data)
{
    FGameplayAbilitySpec* ability_spec = nullptr;

    auto items = TArrayManager<FGameplayAbilitySpec>(&ability_system_component->ActivatableAbilities().Items(), FGameplayAbilitySpec::GetStructSize());
    
    for (FGameplayAbilitySpec& item : items)
    {
        if (item.Handle == ability_handle.Handle)
        {
            ability_spec = &item;
        }
    }

    if (ability_spec == nullptr)
    {
        // Player does not have ability
        ability_system_component->ClientActivateAbilityFailed(ability_handle, prediction_key.Current);

        return;
    }

    UObject* instanced_ability = nullptr;

    // Mark ability as input pressed
    ability_spec->InputPressed = true;

    if (!InternalTryActivateAbility(ability_system_component, ability_handle, prediction_key, &instanced_ability, nullptr, event_data))
    {
        // Failed to activate ability
        ability_system_component->ClientActivateAbilityFailed(ability_handle, prediction_key.Current);

        // Reset input press
        ability_spec->InputPressed = false;
    }

    // Mark ability for replication
    MarkAbilitySpecDirty(ability_system_component, *ability_spec);
}

void ability_component::apply_ability_set(UObject* player_pawn, UObject* ability_set)
{
    if (ability_set == nullptr)
    {
        return;
    }

    // Apply ability set
    reinterpret_cast<AFortAIPawn*>(player_pawn)->AddFortAbilitySet(ability_set);
}

void ability_component_hooks::ServerTryActivateAbility_Hook(UAbilitySystemComponent* AbilitySystemComponent, FFrame& StackFrame)
{
    struct function_params
    {
        // Ability to activate
        FGameplayAbilitySpecHandle Handle;

        // Input pressed?
        bool InputPressed;

        // Prediction key
        FPredictionKey PredictionKey;
    };

    // TODO: move to direct function hooking or a macro
    function_params params = *reinterpret_cast<function_params*>(StackFrame.Locals);
    
    // Activate ability
    ability_component::activate_ability(AbilitySystemComponent, params.Handle, params.InputPressed, params.PredictionKey); 
    
    return ServerTryActivateAbility(AbilitySystemComponent, StackFrame);
}

void ability_component_hooks::ServerTryActivateAbilityWithEventData_Hook(UAbilitySystemComponent* AbilitySystemComponent, FFrame& StackFrame)
{
    GET_PARAMETER(FGameplayAbilitySpecHandle, AbilityToActivate);
    GET_PARAMETER(bool, InputPressed);
    GET_PARAMETER(FPredictionKey, PredictionKey);
    GET_PARAMETER(FGameplayEventData, TriggerEventData);
    
    ability_component::activate_ability(AbilitySystemComponent, AbilityToActivate, InputPressed, PredictionKey, &TriggerEventData); 
    
    return ServerTryActivateAbilityWithEventData(AbilitySystemComponent, StackFrame);
}

void ability_component_hooks::ServerAbilityRPCBatch_Hook(UAbilitySystemComponent* AbilitySystemComponent, FFrame& StackFrame)
{
    GET_PARAMETER(FServerAbilityRPCBatch, BatchInfo);
    
    ability_component::activate_ability(AbilitySystemComponent, BatchInfo.AbilitySpecHandle(), BatchInfo.InputPressed(), BatchInfo.PredictionKey()); 
    
    return ServerAbilityRPCBatch(AbilitySystemComponent, StackFrame);   
}

void ability_component_hooks::AbilityCost_Hook(AFortWeapon* FortWeapon, int Amount)
{
    if (FortWeapon)
    {
        if (const auto weapon_item_definition = static_cast<UFortWorldItemDefinition*>(FortWeapon->WeaponData()))
        {
            if (const auto ammo_item_definition = weapon_item_definition->GetAmmoWorldItemDefinition_BP())
            {   
                const auto player_controller = static_cast<AFortPawn*>(FortWeapon->GetOwner())->Controller();

                if (UFortWorldItem* ItemToRemove = static_cast<UFortWorldItem*>(inventory_component::get_inventory_item_by_definition(player_controller, ammo_item_definition)))
                {
                    inventory_component::remove_item(player_controller, ItemToRemove->ItemEntry().ItemGuid(), Amount, false);
                }
            }
        }
    }

    return AbilityCost(FortWeapon, Amount);
}

bool ability_component_hooks::DoesAbilitySatisfyTagRequirements_Hook(UGameplayAbility* Ability, UAbilitySystemComponent* UAbilitySystemComponent, void* SourceTags, void* TargetTags, void* OutTags)
{
    if (Ability == nullptr)
    {
        return false;
    }
    
    if (SourceTags || TargetTags)
    {
        return DoesAbilitySatisfyTagRequirements(Ability, UAbilitySystemComponent, SourceTags, TargetTags, OutTags);
    }

    return true;
}