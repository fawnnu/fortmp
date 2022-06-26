#pragma once

struct FTableRowHandle
{
    // Instance of current table
    UObject* Table;

    // Current row name
    FName RowName; 
};

class UDataTable : public UObject
{
public:
    DEFINE_PROPERTY(UObject*, RowStruct);
};
