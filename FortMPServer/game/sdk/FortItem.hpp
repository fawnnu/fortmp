#pragma once
#include "../../utils/Containers.hpp"

struct FFortItemEntry;
class UFortItemDefinition;

class UFortItem : public UObject
{
public:
    UFortItemDefinition* GetItemDefinitionBP()
    {
        return Call<UFortItemDefinition*>(L"GetItemDefinitionBP");
    }

    FGuid GetItemGuid()
    {
        return Call<FGuid>(L"GetItemGuid");
    }

    int GetMaxStackSize(class UAbilitySystemComponent* AbilityComponent)
    {
        return Call<int>(L"GetMaxStackSize", AbilityComponent);
    }

    int GetRemainingAmmo()
    {
        return Call<int>(L"GetRemainingAmmo");
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
    
    UFortItem* CreateTemporaryItemInstanceBP(int Count, int Level)
    {
        return Call<UFortItem*>(L"CreateTemporaryItemInstanceBP", Count, Level);
    }
};

class UFortGadgetItemDefinition : public UFortItemDefinition
{
public:

    DEFINE_PROPERTY(TArray<class UCustomCharacterPart*>, CharacterParts)
    DEFINE_PROPERTY(TSoftObjectPtr<class UObject>, AbilitySet)
    DEFINE_PROPERTY(TSoftObjectPtr<class UObject>, AttributeSet)
    DEFINE_PROPERTY(TArray<struct FGameplayAttribute>, TrackedAttributes);
    
    UFortItemDefinition* GetDecoItemDefinition()
    {
        if(!HasFunction(L"GetWeaponItemDefinition"))
        {
            // Used on early versions
            return Call<UFortItemDefinition*>(L"GetDecoItemDefinition");   
        }
        else
        {
            // Used on later builds
            return Call<UFortItemDefinition*>(L"GetWeaponItemDefinition");
        }
    }
};

class UFortWorldItemDefinition : public UFortItemDefinition
{
public:

    DEFINE_BIT_PROPERTY(bool, bForceFocusWhenAdded);

    UFortWorldItemDefinition* GetAmmoWorldItemDefinition_BP()
    {
        return Call<UFortWorldItemDefinition*>(L"GetAmmoWorldItemDefinition_BP");
    }
};

class UFortWeaponItemDefinition : public UFortWorldItemDefinition
{
public:
    DEFINE_PROPERTY(struct FTableRowHandle, WeaponStatHandle);
};

class UAthenaCharacterItemDefinition : public UFortItemDefinition
{
public:
    DEFINE_PROPERTY(class UFortHeroType*, HeroDefinition);
};

class UBGAConsumableWrapperItemDefinition : public UFortItemDefinition
{
public:
    static UObject* StaticClass()
    {
        static UObject* static_class = FindObject(L"/Script/FortniteGame.BGAConsumableWrapperItemDefinition");

        return static_class;
    }
    
    DEFINE_PROPERTY(TSoftObjectPtr<UObject>, ConsumableClass);
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
        Call(L"HandleInventoryLocalUpdate");
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

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortItemEntry");
};

struct FFortBaseWeaponStats
{
    DEFINE_STRUCT_PROPERTY(int, ClipSize);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortBaseWeaponStats");
};

struct FItemAndCount
{
    int Count;
    char pad2[4];
    UFortItemDefinition* Item;
};