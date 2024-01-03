#include "include.hpp"
#include "ability_component.hpp"

#include "inventory_component.hpp"
#include "log_component.hpp"

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

    INIT_POINTER(UpdateTagMap_Internal, memory_manager::find_function_by_string(L"Attempted to remove tag: %s from tag count container, but it is not explicitly in the container!"));
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(ability_component_hooks::ServerTryActivateAbility, L"/Script/GameplayAbilities.AbilitySystemComponent.ServerTryActivateAbility", ability_component_hooks::ServerTryActivateAbility_Hook);
    REGISTER_UFUNCTION_DETOUR(ability_component_hooks::ServerTryActivateAbilityWithEventData, L"/Script/GameplayAbilities.AbilitySystemComponent.ServerTryActivateAbilityWithEventData", ability_component_hooks::ServerTryActivateAbilityWithEventData_Hook);
    REGISTER_UFUNCTION_DETOUR(ability_component_hooks::ServerAbilityRPCBatch, L"/Script/GameplayAbilities.AbilitySystemComponent.ServerAbilityRPCBatch", ability_component_hooks::ServerAbilityRPCBatch_Hook);

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

void ability_component::apply_playlist_modifiers(UObject* player_pawn)
{
    if (AFortPlayerPawnAthena* PlayerPawn = player_pawn->Cast<AFortPlayerPawnAthena>())
    {
        // Get playlist
        if (UFortPlaylistAthena* Playlist = GetPlaylistEasy())
        {
            if (Playlist->Cast<UFortPlaylistAthena>())
            {
                if (STATIC_HASPROPERTY(Playlist, ModifierList))
                {
                    for (TSoftObjectPtr<UFortGameplayModifierItemDefinition> Modifier : Playlist->ModifierList())
                    {
                        if (UFortGameplayModifierItemDefinition* ModifierItemDefinition = Modifier.Get())
                        {
                            if (ModifierItemDefinition->Cast<UFortGameplayModifierItemDefinition>())
                            {
                                TArrayManager<FFortGameplayEffectDeliveryInfo> PersistentGameplayEffects = TArrayManager<FFortGameplayEffectDeliveryInfo>(&ModifierItemDefinition->PersistentGameplayEffects(), FFortGameplayEffectDeliveryInfo::GetStructSize());

                                for (FFortGameplayEffectDeliveryInfo& PersistentGameplayEffect : PersistentGameplayEffects)
                                {
                                    // Loop through the gameplay effects
                                    TArrayManager<FGameplayEffectApplicationInfo> GameplayEffects = TArrayManager<FGameplayEffectApplicationInfo>(&PersistentGameplayEffect.GameplayEffects(), FGameplayEffectApplicationInfo::GetStructSize());

                                    for (FGameplayEffectApplicationInfo& GameplayEffect : GameplayEffects)
                                    {
                                        if (UAbilitySystemComponent* ASC = PlayerPawn->AbilitySystemComponent())
                                        {
                                            FGameplayEffectSpecHandle* Spec = UAbilitySystemBlueprintLibrary::MakeSpecHandle(CreateDefaultObject<UObject>(GameplayEffect.GameplayEffect().Get()), PlayerPawn, PlayerPawn, GameplayEffect.Level());

                                            if (Spec)
                                            {
                                                Spec->AddSourceObject(PlayerPawn);
                                                ASC->BP_ApplyGameplayEffectSpecToSelf(Spec);

                                                if (log_component::enable_debug_logs)
                                                {
                                                    spdlog::info(L"ability_component::apply_playlist_modifiers: Applied effect {} to player.", GameplayEffect.GameplayEffect()->GetName().c_str());
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void ability_component::UpdateTagMap(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTag& Tag, int CountDelta)
{
    if (AbilitySystemComponent)
    {
        if (static void* BlockedAbilityBindings_Property = FindPropertyByName(AbilitySystemComponent->Class, TEXT("BlockedAbilityBindings")))
        {
            static int BlockedAbilityBindings_Offset = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(BlockedAbilityBindings_Property) + CoreOffsets::Offset_Internal);

            const PVOID GameplayTagCountContainer = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(AbilitySystemComponent) + BlockedAbilityBindings_Offset + 0x10);

            if (UpdateTagMap_Internal)
            {
                UpdateTagMap_Internal(GameplayTagCountContainer, Tag, CountDelta);
            }
        }
    }
}

void ability_component::RemoveGameplayCue(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTag& CueTag)
{
    if (AbilitySystemComponent)
    {
        TArrayManager Items = TArrayManager<FGameplayAbilitySpec>(&AbilitySystemComponent->ActivatableAbilities().Items(), FGameplayAbilitySpec::GetStructSize());

        for (FGameplayAbilitySpec& Spec : Items)
        {
            if (Spec.Ability)
            {
                if (UEnum* EGameplayAbilityInstancingPolicy = FindObject<UEnum>(TEXT("/Script/GameplayAbilities.EGameplayAbilityInstancingPolicy")))
                {
                    if (static_cast<UGameplayAbility*>(Spec.Ability)->InstancingPolicy() != EGameplayAbilityInstancingPolicy->GetEnumIndex(TEXT("NonInstanced")))
                    {
                        // K2_RemoveGameplayCue for every instanced ability
                        for (UGameplayAbility* ReplicatedInstance : Spec.ReplicatedInstances)
                        {
                            ReplicatedInstance->K2_RemoveGameplayCue(CueTag);
                        }
                        for (UGameplayAbility* NonReplicatedInstance : Spec.NonReplicatedInstances)
                        {
                            NonReplicatedInstance->K2_RemoveGameplayCue(CueTag);
                        }
                    }
                }
            }
        }
    }
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
