#pragma once

struct FFortItemEntry;

class UFortItem : public UObject
{
public:
    DEFINE_PROPERTY(int, Count);
};

class UFortWorldItem : public UFortItem
{
public:
    DEFINE_PROPERTY(FFortItemEntry, ItemEntry);
};

class UFortItemDefinition : public UObject
{
public:
    UFortItem* CreateTemporaryItemInstanceBP(int Count, int Level)
    {
        return Call<UFortItem*>(L"CreateTemporaryItemInstanceBP", Count, Level);
    }
};

struct FFortItemEntry
{
    DEFINE_STRUCT_PROPERTY(int, Count);
    DEFINE_STRUCT_PROPERTY(int, PreviousCount);
    DEFINE_STRUCT_PROPERTY(UFortItemDefinition*, ItemDefinition);
    DEFINE_STRUCT_PROPERTY(int, LoadedAmmo);
    DEFINE_STRUCT_PROPERTY(struct FGuid, ItemGuid);

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortItemEntry");
};