#pragma once

struct FTableRowHandle
{
    // Instance of current table
    class UDataTable* Table;

    // Current row name
    FName RowName;

    template <typename T>
    T* Eval()
    {
        auto Map = *reinterpret_cast<TMap<FName, T*>*>(__int64(Table) + 0x30);

        return &Map[RowName].Value;
    }
};

class UDataTable : public UObject
{
public:
    DEFINE_PROPERTY(UObject*, RowStruct);
};
