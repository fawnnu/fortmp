#pragma once

class UGameplayAbility;

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
    FGameplayAbilitySpec() = default;
    
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
    TArray<UGameplayAbility*> NonReplicatedInstances;
    TArray<UGameplayAbility*> ReplicatedInstances;
    char pad2[0x8];
    char UnknownData_78[0x50];

    DEFINE_STRUCT_CLASS(L"/Script/GameplayAbilities.GameplayAbilitySpec");
};

struct FGameplayAbilitySpecContainer : public FFastArraySerializer
{
    DEFINE_STRUCT_PROPERTY(TArray<struct FGameplayAbilitySpec>, Items);

    DEFINE_STRUCT_CLASS(L"/Script/GameplayAbilities.GameplayAbilitySpecContainer");
};

struct FServerAbilityRPCBatch
{
    DEFINE_STRUCT_PROPERTY(FGameplayAbilitySpecHandle, AbilitySpecHandle);
    DEFINE_STRUCT_PROPERTY(FPredictionKey, PredictionKey);
    DEFINE_STRUCT_PROPERTY(bool, InputPressed);
    
    DEFINE_STRUCT_CLASS(L"/Script/GameplayAbilities.ServerAbilityRPCBatch");
};

struct FGameplayAttribute
{
    DEFINE_STRUCT_PROPERTY(FString, AttributeName);
    DEFINE_STRUCT_PROPERTY(UObject*, AttributeOwner);
    
    DEFINE_STRUCT_CLASS(L"/Script/GameplayAbilities.GameplayAttribute");
};

struct FGameplayAttributeData
{
    DEFINE_STRUCT_PROPERTY(float, BaseValue);
    DEFINE_STRUCT_PROPERTY(float, CurrentValue);
    
    DEFINE_STRUCT_CLASS(L"/Script/GameplayAbilities.GameplayAttributeData");
};

struct FScalableFloat
{
    DEFINE_STRUCT_PROPERTY(float, Value);
    DEFINE_STRUCT_PROPERTY(FCurveTableRowHandle, Curve);
    
    DEFINE_STRUCT_CLASS(L"/Script/GameplayAbilities.ScalableFloat");

    float GetValueAtLevel(float Level, FString* ContextString)
    {
        static uintptr_t GetValueAtLevel_Address = memory_manager::find_function_by_string(L"FScalableFloat::GetValueAtLevel");

        if (GetValueAtLevel_Address)
        {
            return reinterpret_cast<float (*)(FScalableFloat*, float, FString*)>(GetValueAtLevel_Address)(this, Level, ContextString);
        }
        
        spdlog::error(L"FScalableFloat::GetValueAtLevel failed: invalid address.");

        return 0.f;
    }

    FScalableFloat& operator=(FScalableFloat& InScalableFloat)
    {
        Value() = InScalableFloat.Value();
        Curve() = InScalableFloat.Curve();

        return *this;
    }
};

class UAbilitySystemComponent : public UObject
{
public:
    DEFINE_PROPERTY(FGameplayAbilitySpecContainer, ActivatableAbilities);

    DEFINE_PROPERTY(TArray<UObject*>, SpawnedAttributes);

    void ClientActivateAbilityFailed(FGameplayAbilitySpecHandle AbilityToActivate, int16_t PredictionKey)
    {
        STATIC_FINDFUNCTION(ClientActivateAbilityFailed)
        
        Call(Function_ClientActivateAbilityFailed, AbilityToActivate, PredictionKey);
    }
};