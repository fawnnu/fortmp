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
        static UFunction* function = FindObject<UFunction>(L"/Script/FortniteGame.FortItemDefinition.CreateTemporaryItemInstanceBP");
        
        struct params
        {
            // Item count
            int Count;

            // Level of item
            int Level;

            // Temporary item created
            UFortItem* TemporaryItemInstance;
        } function_params;
        
        function_params.Count = Count;
        function_params.Level = Level;
        
        // TODO: use Call
        ProcessEvent(this, function, &function_params);

        return function_params.TemporaryItemInstance;
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