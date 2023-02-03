#pragma once

class ability_component
{
public:
    ability_component();

    // Directly activate an ability
    static void activate_ability(UAbilitySystemComponent* ability_system_component, FGameplayAbilitySpecHandle ability_handle, bool input_pressed, const FPredictionKey& prediction_key, FGameplayEventData* event_data = nullptr);

    // Activate abilities from ability set
    static void apply_ability_set(UObject* player_pawn, UObject* ability_set);
};

namespace ability_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, ServerTryActivateAbility, (UAbilitySystemComponent* AbilitySystemComponent, FFrame& StackFrame));
    DEFINE_FUNCTION_HOOK(void, ServerTryActivateAbilityWithEventData, (UAbilitySystemComponent* AbilitySystemComponent, FFrame& StackFrame));
    DEFINE_FUNCTION_HOOK(void, ServerAbilityRPCBatch, (UAbilitySystemComponent* AbilitySystemComponent, FFrame& StackFrame));
    DEFINE_FUNCTION_HOOK(void, AbilityCost, (AFortWeapon* FortWeapon, int Amount))
}

// GHandle for specs
inline int* GHandle;

// Activate ability internally
inline bool (*InternalTryActivateAbility)(UObject* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, FPredictionKey InPredictionKey, UObject** OutInstancedAbility, void* OnGameplayAbilityEndedDelegate, void* TriggerEventData);

// Mark ability for replication
inline void (*MarkAbilitySpecDirty)(UObject* AbilitySystemComponent, FGameplayAbilitySpec& Spec);

// Gives an ability and activates it once
inline FGameplayAbilitySpecHandle (*GiveAbilityAndActivateOnce)(UAbilitySystemComponent* AbilitySystemComponent, DWORD*, FGameplayAbilitySpec& InSpec);
