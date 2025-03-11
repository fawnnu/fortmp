#pragma once
#include "utils/Containers.hpp"

struct FFortItemEntry;
class UFortItemDefinition;

class UFortItem : public UObject
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortItem");
    
    UFortItemDefinition* GetItemDefinitionBP()
    {
        STATIC_FINDFUNCTION(GetItemDefinitionBP)
        
        return Call<UFortItemDefinition*>(Function_GetItemDefinitionBP);
    }

    FGuid GetItemGuid()
    {
        STATIC_FINDFUNCTION(GetItemGuid)
        
        return Call<FGuid>(Function_GetItemGuid);
    }

    int GetMaxStackSize(class UAbilitySystemComponent* AbilityComponent)
    {
        STATIC_FINDFUNCTION(GetMaxStackSize)
        
        return Call<int>(Function_GetMaxStackSize, AbilityComponent);
    }

    int GetRemainingAmmo()
    {
        STATIC_FINDFUNCTION(GetRemainingAmmo)
        
        return Call<int>(Function_GetRemainingAmmo);
    }
};

class UFortHero : public UFortItem
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortHero");
    DEFINE_PROPERTY(TArray<class UFortHeroSpecialization*>, Specializations);
    DEFINE_PROPERTY(TArray<class UCustomCharacterPart*>, CharacterParts);
    DEFINE_PROPERTY(FString, Hero_name);

    class UFortHeroType* GetHeroTypeBP()
    {
        STATIC_FINDFUNCTION(GetHeroTypeBP)
                
        return Call<UFortHeroType*>(Function_GetHeroTypeBP);
    }
};

class UFortWorldItem : public UFortItem
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortWorldItem");
    
    DEFINE_PROPERTY(FFortItemEntry, ItemEntry);
};

class UFortItemDefinition : public UObject
{
public:
    DEFINE_PROPERTY(int, MaxStackSize);
    DEFINE_PROPERTY(uint8_t, ItemType);
    DEFINE_PROPERTY(uint8_t, FilterOverride);
    DEFINE_PROPERTY(uint8_t, Rarity);
    DEFINE_PROPERTY(uint8_t, Tier);
    DEFINE_PROPERTY(struct FGameplayTagContainer, GameplayTags);
    
    UFortItem* CreateTemporaryItemInstanceBP(int Count, int Level)
    {
        STATIC_FINDFUNCTION(CreateTemporaryItemInstanceBP)
        
        return Call<UFortItem*>(Function_CreateTemporaryItemInstanceBP, Count, Level);
    }

    std::string GetRarity()
    {
        std::string rarity;

        if (STATIC_HASPROPERTY(this, Tier))
        {
            static UEnum* EFortItemTier = FindObject<UEnum>(L"/Script/FortniteGame.EFortItemTier");
            static uint8_t I = EFortItemTier->GetEnumIndex(TEXT("I"));
            static uint8_t II = EFortItemTier->GetEnumIndex(TEXT("II"));
            static uint8_t III = EFortItemTier->GetEnumIndex(TEXT("III"));
            static uint8_t IV = EFortItemTier->GetEnumIndex(TEXT("IV"));
            static uint8_t V = EFortItemTier->GetEnumIndex(TEXT("V"));
            if (Tier() == I)
            {
                rarity = "common";
            }
            else if (Tier() == II)
            {
                rarity = "uncommon";
            }
            else if (Tier() == III)
            {
                rarity = "rare";
            }
            else if (Tier() == IV)
            {
                rarity = "epic";
            }
            else if (Tier() == V)
            {
                rarity = "legendary";
            }
        }

        return rarity;
    }

    int GetMaxStackSize();
};

class UFortGadgetItemDefinition : public UFortItemDefinition
{
public:

    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortGadgetItemDefinition")
    
    DEFINE_PROPERTY(TArray<class UCustomCharacterPart*>, CharacterParts)
    DEFINE_PROPERTY(TSoftObjectPtr<class UObject>, AbilitySet)
    DEFINE_PROPERTY(TSoftObjectPtr<class UObject>, AttributeSet)
    DEFINE_PROPERTY(TArray<struct FGameplayAttribute>, TrackedAttributes);
    DEFINE_PROPERTY(bool, bDropAllOnEquip);
    
    UFortItemDefinition* GetDecoItemDefinition()
    {
        STATIC_FINDFUNCTION(GetWeaponItemDefinition)
        
        if(!Function_GetWeaponItemDefinition)
        {
            STATIC_FINDFUNCTION(GetDecoItemDefinition)
            
            // Used on early versions
            return Call<UFortItemDefinition*>(Function_GetDecoItemDefinition);   
        }
        else
        {
            // Used on later builds
            return Call<UFortItemDefinition*>(Function_GetWeaponItemDefinition);
        }
    }

    bool HasTrackedAttributes()
    {
        STATIC_FINDFUNCTION(HasTrackedAttributes)

        return Call<bool>(Function_HasTrackedAttributes);
    }
};

class UFortCreativeRealEstatePlotItemDefinition : public UFortItemDefinition
{
public:
    DEFINE_PROPERTY(TSoftObjectPtr<class UFortPlaysetItemDefinition>, BasePlayset);
};

class UFortWorldItemDefinition : public UFortItemDefinition
{
public:

    DEFINE_BIT_PROPERTY(bool, bForceFocusWhenAdded);
    DEFINE_BIT_PROPERTY(bool, bCanBeDropped);
    DEFINE_BIT_PROPERTY(bool, bPersistInInventoryWhenFinalStackEmpty);
    DEFINE_BIT_PROPERTY(bool, bAllowedToBeLockedInInventory);
    
    UFortWorldItemDefinition* GetAmmoWorldItemDefinition_BP()
    {
        STATIC_FINDFUNCTION(GetAmmoWorldItemDefinition_BP)
        
        return Call<UFortWorldItemDefinition*>(Function_GetAmmoWorldItemDefinition_BP);
    }
};

class UFortResourceItemDefinition : public UFortWorldItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortResourceItemDefinition")
};

class UFortAmmoItemDefinition : public UFortWorldItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAmmoItemDefinition")
};

class UFortTrapItemDefinition : public UFortWorldItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortTrapItemDefinition")
};


class UFortWeaponItemDefinition : public UFortWorldItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortWeaponItemDefinition")
    
    DEFINE_PROPERTY(struct FTableRowHandle, WeaponStatHandle);
    DEFINE_PROPERTY(TSoftObjectPtr<UObject>, WeaponActorClass);
};

class UFortVehicleItemDefinition : public UFortWorldItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortVehicleItemDefinition")
    
    DEFINE_PROPERTY(struct FScalableFloat, VehicleMinSpawnPercent);
    DEFINE_PROPERTY(FScalableFloat, VehicleMaxSpawnPercent);
    DEFINE_PROPERTY(FScalableFloat, MaxNumberCanSpawn);
    DEFINE_PROPERTY(FScalableFloat, MinPercentWithGas);
    DEFINE_PROPERTY(FScalableFloat, MaxPercentWithGas);
    DEFINE_PROPERTY(FScalableFloat, MinPercentInoperable);
    DEFINE_PROPERTY(FScalableFloat, MaxPercentInoperable);
    DEFINE_PROPERTY(TSoftObjectPtr<class UObject>, VehicleActorClass);
};

class UFortWeaponRangedItemDefinition : public UFortWeaponItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortWeaponRangedItemDefinition");
};

class UFortWeaponMeleeItemDefinition : public UFortWeaponItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortWeaponMeleeItemDefinition");
};

class UAthenaCosmeticItemDefinition : public UFortItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.AthenaCosmeticItemDefinition");

    DEFINE_PROPERTY(TArray<class UObject*>, ItemVariants);
};

class UFortMontageItemDefinitionBase : public UAthenaCosmeticItemDefinition
{
    
};

class UAthenaDanceItemDefinition : public UFortMontageItemDefinitionBase
{
public:
    DEFINE_PROPERTY(bool, bMovingEmote);
    DEFINE_PROPERTY(bool, bMoveForwardOnly);
    DEFINE_PROPERTY(float, WalkForwardSpeed);
};

class UAthenaItemWrapDefinition : public UFortItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.AthenaItemWrapDefinition")
};

class UAthenaSprayItemDefinition : public UAthenaDanceItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.AthenaSprayItemDefinition");
};

class UAthenaToyItemDefinition : public UAthenaDanceItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.AthenaToyItemDefinition");

    DEFINE_PROPERTY(TSoftObjectPtr<class UObject>, ToySpawnAbility);
    
    TSoftObjectPtr<class UObject> GetToyActorClass()
    {
        STATIC_FINDFUNCTION(GetToyActorClass)
        
        return Call<TSoftObjectPtr<class UObject>>(Function_GetToyActorClass);
    }
};

class UAthenaCharacterItemDefinition : public UAthenaCosmeticItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.AthenaCharacterItemDefinition");
    DEFINE_PROPERTY(class UFortHeroType*, HeroDefinition);

    void ApplyVariants(AActor* InActor, TArray<struct FMcpVariantChannelInfo>& VariantChannels)
    {
        STATIC_FINDFUNCTION(ApplyVariants)
        
        Call(Function_ApplyVariants, InActor, VariantChannels);
    }
};

class UAthenaCharacterPartItemDefinition : public UAthenaCosmeticItemDefinition
{
public:
    DEFINE_PROPERTY(TArray<UCustomCharacterPart*>, CharacterParts);
};

class UAthenaBackpackItemDefinition : public UAthenaCharacterPartItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.AthenaBackpackItemDefinition");
};

class UAthenaPickaxeItemDefinition : public UAthenaCosmeticItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.AthenaPickaxeItemDefinition");
    // Pickaxe Weapon Item Data
    DEFINE_PROPERTY(UFortWeaponItemDefinition*, WeaponDefinition);
};

class UBGAConsumableWrapperItemDefinition : public UFortItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.BGAConsumableWrapperItemDefinition")
    
    DEFINE_PROPERTY(TSoftObjectPtr<UObject>, ConsumableClass);
};

class UAthenaGliderItemDefinition : public UAthenaCosmeticItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.AthenaGliderItemDefinition")
};

class UAthenaSkyDiveContrailItemDefinition : public UAthenaCosmeticItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.AthenaSkyDiveContrailItemDefinition")
};

struct FMcpVariantChannelInfo
{
    DEFINE_STRUCT_PROPERTY(struct FGameplayTag, VariantChannelTag);
    DEFINE_STRUCT_PROPERTY(struct FGameplayTag, ActiveVariantTag);
    DEFINE_STRUCT_PROPERTY(struct FGameplayTagContainer, OwnedVariantTags);
    DEFINE_STRUCT_PROPERTY(UObject*, ItemVariantIsUsedFor);

    OVERLOAD_NEW_OPERATOR();
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.McpVariantChannelInfo");
};

struct FFortAthenaLoadout
{
    DEFINE_STRUCT_PROPERTY(FString, BannerIconId);
    DEFINE_STRUCT_PROPERTY(FString, BannerColorId);
    DEFINE_STRUCT_PROPERTY(UAthenaCosmeticItemDefinition*, SkyDiveContrail);
    DEFINE_STRUCT_PROPERTY(UAthenaCosmeticItemDefinition*, Glider);
    DEFINE_STRUCT_PROPERTY(UAthenaPickaxeItemDefinition*, Pickaxe);
    DEFINE_STRUCT_PROPERTY(UAthenaCharacterItemDefinition*, Character);
    DEFINE_STRUCT_PROPERTY(UAthenaCharacterPartItemDefinition*, Backpack);
    DEFINE_STRUCT_PROPERTY(UAthenaCosmeticItemDefinition*, ItemWrapOverride);
    DEFINE_STRUCT_PROPERTY(TArray<FMcpVariantChannelInfo>, CharacterVariantChannels);
    DEFINE_STRUCT_PROPERTY(TArray<UAthenaItemWrapDefinition*>, ItemWraps);
    DEFINE_STRUCT_PROPERTY(TArray<UAthenaDanceItemDefinition*>, Dances);

    OVERLOAD_NEW_OPERATOR();
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortAthenaLoadout");
};


struct FFortItemList : public FFastArraySerializer
{
    DEFINE_STRUCT_PROPERTY(TArray<FFortItemEntry>, ReplicatedEntries);
    DEFINE_STRUCT_PROPERTY(TArray<UFortWorldItem*>, ItemInstances);

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortItemList");
};
 
class AFortInventory : public UObject
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortInventory")
    
    DEFINE_PROPERTY(FFortItemList, Inventory);

    void HandleInventoryLocalUpdate()
    {
        STATIC_FINDFUNCTION(HandleInventoryLocalUpdate)
        
        Call(Function_HandleInventoryLocalUpdate);
    }
};

struct FFortItemEntryStateValue
{
public:
    DEFINE_STRUCT_PROPERTY(int, IntValue);
    DEFINE_STRUCT_PROPERTY(FName, NameValue);
    DEFINE_STRUCT_PROPERTY(uint8_t, StateType);

    UE_OVERLOAD_NEW_OPERATOR();
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortItemEntryStateValue")
};

struct FFortItemEntry : public FFastArraySerializerItem
{
    DEFINE_STRUCT_PROPERTY(int, Count);
    DEFINE_STRUCT_PROPERTY(int, PreviousCount);
    DEFINE_STRUCT_PROPERTY(UFortItemDefinition*, ItemDefinition);
    DEFINE_STRUCT_PROPERTY(int, LoadedAmmo);
    DEFINE_STRUCT_PROPERTY(struct FGuid, ItemGuid);
    DEFINE_STRUCT_PROPERTY(TArray<FFortItemEntryStateValue>, StateValues);
    DEFINE_STRUCT_PROPERTY(TArray<float>, GenericAttributeValues);
    DEFINE_STRUCT_PROPERTY(bool, bIsDirty);

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortItemEntry");

    void AddStateValue(int IntValue, FName NameValue, uint8_t StateType)
    {
        TArrayManager StateValuesManager = StateValues();

        // Make a new state value
        FFortItemEntryStateValue* NewStateValue = new FFortItemEntryStateValue;

        // Initialize the properties
        NewStateValue->IntValue() = IntValue;
        NewStateValue->NameValue() = NameValue;
        NewStateValue->StateType() = StateType;

        // Add it to the StateValues
        StateValuesManager.Add(NewStateValue);

        // Free
        FMemory_Free(NewStateValue);
    }

    FFortItemEntryStateValue* GetStateValueByName(FName InNameValue)
    {
        TArrayManager<FFortItemEntryStateValue> StateValuesManager = TArrayManager<FFortItemEntryStateValue>(&StateValues(), sizeof(FFortItemEntryStateValue));

        for (FFortItemEntryStateValue& StateValue : StateValuesManager)
        {
            if (StateValue.NameValue() == InNameValue)
            {
                return &StateValue;
            }
        }
        
        return nullptr;
    }
};

struct FFortBaseWeaponStats
{
    DEFINE_STRUCT_PROPERTY(int, ClipSize);
    DEFINE_STRUCT_PROPERTY(float, DamageZone_Light);
    DEFINE_STRUCT_PROPERTY(float, DamageZone_Normal);
    DEFINE_STRUCT_PROPERTY(float, DamageZone_Critical);
    DEFINE_STRUCT_PROPERTY(float, DamageZone_Vulnerability);
    DEFINE_STRUCT_PROPERTY(float, DmgPB);
    DEFINE_STRUCT_PROPERTY(int, CartridgePerFire);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortBaseWeaponStats");
};

struct FFortRangedWeaponStats
{
    DEFINE_STRUCT_PROPERTY(float, SweepRadius);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortRangedWeaponStats");
};

struct FItemAndCount
{
    int Count;
    char pad2[4];
    UFortItemDefinition* Item;
};

struct FActiveItemGrantInfo
{
    DEFINE_STRUCT_PROPERTY(UFortWorldItemDefinition*, Item);
    DEFINE_STRUCT_PROPERTY(FScalableFloat, AmountToGive);
    DEFINE_STRUCT_PROPERTY(FScalableFloat, MaxAmount);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.ActiveItemGrantInfo");
};