#include "include.hpp"
#include "ability_component.hpp"

ability_component::ability_component()
{
    INIT_POINTER(InternalTryActivateAbility, memory_manager::find_pattern("4C 89 4C 24 ? 4C 89 44 24 ? 89 54 24 10 55 53 56 57 41 54 41 57 48 8D AC 24 ? ? ? ?"));
    INIT_POINTER(InternalTryActivateAbility, memory_manager::find_function_by_string(L"InternalTryActivateAbility called with invalid"));

    INIT_POINTER(MarkAbilitySpecDirty, memory_manager::find_pattern("48 89 5C 24 ? 48 89 7C 24 ? 41 56 48 83 EC 20 48 8B 01 41 0F B6 D8 4C 8B F2 48 8B F9 FF 90 ? ? ? ? 84 C0 0F 84 ? ? ? ?"));
    INIT_POINTER(MarkAbilitySpecDirty, memory_manager::find_pattern("48 89 5C 24 ? 57 48 83 EC 20 80 B9 ? ? ? ? ? 48 8B FA 48 8B D9 75 4A C6 81 ? ? ? ? ? 48 81 C1 ? ? ? ? 83 3A FF 75 16 FF 41 50"));

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(ability_component_hooks::ServerTryActivateAbility, L"/Script/GameplayAbilities.AbilitySystemComponent.ServerTryActivateAbility", ability_component_hooks::ServerTryActivateAbility_Hook);
    REGISTER_UFUNCTION_DETOUR(ability_component_hooks::ServerTryActivateAbilityWithEventData, L"/Script/GameplayAbilities.AbilitySystemComponent.ServerTryActivateAbilityWithEventData", ability_component_hooks::ServerTryActivateAbilityWithEventData_Hook);
    REGISTER_UFUNCTION_DETOUR(ability_component_hooks::ServerAbilityRPCBatch, L"/Script/GameplayAbilities.AbilitySystemComponent.ServerAbilityRPCBatch", ability_component_hooks::ServerAbilityRPCBatch_Hook);
    
    DetourTransactionCommit();
}

void ability_component::activate_ability(UAbilitySystemComponent* ability_system_component, FGameplayAbilitySpecHandle ability_handle,
    bool input_pressed, const FPredictionKey& prediction_key)
{
    FGameplayAbilitySpec* ability_spec = nullptr;
    
    for (FGameplayAbilitySpec& item : ability_system_component->ActivatableAbilities().Items)
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

    if (!InternalTryActivateAbility(ability_system_component, ability_handle, prediction_key, &instanced_ability, nullptr, nullptr))
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
    return ServerTryActivateAbilityWithEventData(AbilitySystemComponent, StackFrame);
}

void ability_component_hooks::ServerAbilityRPCBatch_Hook(UAbilitySystemComponent* AbilitySystemComponent, FFrame& StackFrame)
{
    return ServerAbilityRPCBatch(AbilitySystemComponent, StackFrame);   
}