#pragma once

struct FCurveTableRowHandle
{                                 
    class UCurveTable* CurveTable;
    FName RowName;
};

class UCurveTable : public UObject
{
public:
    
};

struct FSimpleCurveKey
{
    DEFINE_STRUCT_PROPERTY(float, Value);
    
    DEFINE_STRUCT_CLASS(L"/Script/Engine.SimpleCurveKey");
};

struct FRichCurveKey
{
    DEFINE_STRUCT_PROPERTY(float, Value);
    
    DEFINE_STRUCT_CLASS(L"/Script/Engine.RichCurveKey");
};

//Make both structs have a common parent struct in order to later cast them. (Dynamic farming, zonewars)
struct FSimpleCurve : Struct
{
    DEFINE_STRUCT_PROPERTY(TArray<FSimpleCurveKey>, Keys);
    
    DEFINE_STRUCT_CLASS(L"/Script/Engine.SimpleCurve");
};

struct FRichCurve : Struct
{
    DEFINE_STRUCT_PROPERTY(TArray<FRichCurveKey>, Keys);
    
    DEFINE_STRUCT_CLASS(L"/Script/Engine.RichCurve");
};