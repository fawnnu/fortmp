#pragma once

class UKismetSystemLibrary : public UObject
{
public:
    template<typename T = UObject>
    static T* Conv_SoftObjectReferenceToObject(TSoftObjectPtr SoftObject)
    {
        // Static function so use default object
        static UObject* Default__KismetSystemLibrary = FindObject(L"/Script/Engine.Default__KismetSystemLibrary");
        
        return Default__KismetSystemLibrary->Call<T*>(L"Conv_SoftObjectReferenceToObject", SoftObject);
    }
};

class UKismetMathLibrary : public UObject
{
public:
    static float RandomFloatInRange(float Min, float Max)
    {
        // Static function so use default object
        static UObject* Default__KismetMathLibrary = FindObject(L"/Script/Engine.Default__KismetMathLibrary");
        
        return Default__KismetMathLibrary->Call<float>(L"RandomFloatInRange", Min, Max);
    }
};