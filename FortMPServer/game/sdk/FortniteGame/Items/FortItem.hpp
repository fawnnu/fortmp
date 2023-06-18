#pragma once
#include "utils/Containers.hpp"

struct FFortItemEntry;
class UFortItemDefinition;

class UFortItem : public UObject
{
public:
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

class UFortWorldItem : public UFortItem
{
public:
    DEFINE_PROPERTY(FFortItemEntry, ItemEntry);
};

class UFortItemDefinition : public UObject
{
public:
    DEFINE_PROPERTY(int, MaxStackSize);
    DEFINE_PROPERTY(uint8_t, ItemType);
    DEFINE_PROPERTY(uint8_t, FilterOverride);
    DEFINE_PROPERTY(uint8_t, Rarity);
    DEFINE_PROPERTY(struct FGameplayTagContainer, GameplayTags);
    
    UFortItem* CreateTemporaryItemInstanceBP(int Count, int Level)
    {
        STATIC_FINDFUNCTION(CreateTemporaryItemInstanceBP)
        
        return Call<UFortItem*>(Function_CreateTemporaryItemInstanceBP, Count, Level);
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

class UFortWeaponItemDefinition : public UFortWorldItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortWeaponItemDefinition")
    
    DEFINE_PROPERTY(struct FTableRowHandle, WeaponStatHandle);
    DEFINE_PROPERTY(TSoftObjectPtr<UObject>, WeaponActorClass);
};

class UFortWeaponMeleeItemDefinition : public UFortWeaponItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortWeaponMeleeItemDefinition");
};

class UAthenaCosmeticItemDefinition : public UFortItemDefinition
{
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

class UAthenaSprayItemDefinition : public UAthenaDanceItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.AthenaSprayItemDefinition");
};

class UAthenaCharacterItemDefinition : public UAthenaCosmeticItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.AthenaCharacterItemDefinition");
    DEFINE_PROPERTY(class UFortHeroType*, HeroDefinition);
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
    DEFINE_PROPERTY(FFortItemList, Inventory);

    void HandleInventoryLocalUpdate()
    {
        STATIC_FINDFUNCTION(HandleInventoryLocalUpdate)
        
        Call(Function_HandleInventoryLocalUpdate);
    }
};

struct FFortItemEntryStateValue
{
    DEFINE_STRUCT_PROPERTY(int, IntValue);
    DEFINE_STRUCT_PROPERTY(FName, NameValue);
    DEFINE_STRUCT_PROPERTY(uint8_t, StateType);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortItemEntryStateValue");
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
        TArrayManager<FFortItemEntryStateValue> StateValuesManager = TArrayManager<FFortItemEntryStateValue>(&StateValues(), FFortItemEntryStateValue::GetStructSize());

        // Make a new state value
        FFortItemEntryStateValue* NewStateValue = reinterpret_cast<FFortItemEntryStateValue*>(alloca(FFortItemEntryStateValue::GetStructSize()));

        // Initialize the newly allocated memory
        ZeroMemory(NewStateValue, FFortItemEntryStateValue::GetStructSize());

        // Initialize the properties
        NewStateValue->IntValue() = IntValue;
        NewStateValue->NameValue() = NameValue;
        NewStateValue->StateType() = StateType;

        // Add it to the StateValues
        StateValuesManager.Add(NewStateValue);
    }

    FFortItemEntryStateValue* GetStateValueByName(FName InNameValue)
    {
        TArrayManager<FFortItemEntryStateValue> StateValuesManager = TArrayManager<FFortItemEntryStateValue>(&StateValues(), FFortItemEntryStateValue::GetStructSize());

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

struct FItemAndCount
{
    int Count;
    char pad2[4];
    UFortItemDefinition* Item;
};