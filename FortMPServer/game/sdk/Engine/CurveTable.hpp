#pragma once

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

struct FCurveTableRowHandle
{                                 
    UCurveTable* CurveTable;
    FName RowName;

    float Eval(int Index)
    {
        auto CurveTableMap = *reinterpret_cast<TMap<FName, Struct*>*>(__int64(CurveTable) + 0x30);

        return FSimpleCurve::GetClass() ?
            static_cast<FSimpleCurve*>(CurveTableMap[RowName].Value)->Keys()[Index].Value() :
            static_cast<FRichCurve*>(CurveTableMap[RowName].Value)->Keys()[Index].Value();
    }
};