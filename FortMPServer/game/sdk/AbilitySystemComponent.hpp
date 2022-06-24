#pragma once

struct FGameplayAbilitySpecHandle
{
    int Handle;
};

struct FPredictionKey
{
    int16_t Current;
    int16_t Base;
    char Unk00[0x4];
    UObject* PredictiveConnection;
    bool bIsStale;
    bool bIsServerInitiated;
    char Unk01[0x6];
};

struct FGameplayAbilitySpec : FFastArraySerializerItem
{
    int Handle;
    UObject* Ability;
    int Level;
    int InputID;
    UObject* SourceObject;
    char ActiveCount;
    bool InputPressed : 1;
    bool RemoveAfterActivation : 1;
    bool PendingRemove : 1;
    char UnknownData_29_3 : 5;
    char UnknownData_2A[0x6];
    char pad[0x20];
    TArray<UObject*> NonReplicatedInstances;
    TArray<UObject*> ReplicatedInstances;
    char pad2[0x8];
    char UnknownData_78[0x50];
};

struct FGameplayAbilitySpecContainer : public FFastArraySerializer
{
    TArray<struct FGameplayAbilitySpec> Items;
    UObject*                            Owner;
};

class UAbilitySystemComponent : public UObject
{
public:
    DEFINE_PROPERTY(FGameplayAbilitySpecContainer, ActivatableAbilities);

    void ClientActivateAbilityFailed(FGameplayAbilitySpecHandle AbilityToActivate, int16_t PredictionKey)
    {
        static UFunction* function = FindObject<UFunction>(L"/Script/GameplayAbilities.AbilitySystemComponent.ClientActivateAbilityFailed");
        
        struct params
        {
            // Ability to activate by handle
            FGameplayAbilitySpecHandle AbilityToActivate;

            // Prediction key
            int16_t PredictionKey;
        } function_params;
        
        function_params.AbilityToActivate = AbilityToActivate;
        function_params.PredictionKey = PredictionKey;

        // TODO: use Call
        ProcessEvent(this, function, &function_params);
    }
};
