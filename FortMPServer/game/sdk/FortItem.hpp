#pragma once

struct FFortItemEntry;
class UFortItemDefinition;

class UFortItem : public UObject
{
public:
    DEFINE_PROPERTY(int, Count);

    UFortItemDefinition* GetItemDefinitionBP()
    {
        return Call<UFortItemDefinition*>(L"GetItemDefinitionBP");
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
    
    UFortItem* CreateTemporaryItemInstanceBP(int Count, int Level)
    {
        return Call<UFortItem*>(L"CreateTemporaryItemInstanceBP", Count, Level);
    }
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

struct FFortItemEntry : public FFastArraySerializerItem
{
    DEFINE_STRUCT_PROPERTY(int, Count);
    DEFINE_STRUCT_PROPERTY(int, PreviousCount);
    DEFINE_STRUCT_PROPERTY(UFortItemDefinition*, ItemDefinition);
    DEFINE_STRUCT_PROPERTY(int, LoadedAmmo);
    DEFINE_STRUCT_PROPERTY(struct FGuid, ItemGuid);

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortItemEntry");
};

struct FItemAndCount
{
    int Count;
    char pad2[4];
    UFortItemDefinition* Item;
};