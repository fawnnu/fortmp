#pragma once

class UKismetSystemLibrary : public UObject
{
public:
};

class UKismetMathLibrary : public UObject
{
public:
};

class UKismetStringLibrary : public UObject
{
public:

    static bool Equals(FString A, FString B)
    {
        static UKismetStringLibrary* Default__KismetStringLibrary = FindObject<UKismetStringLibrary>(XOR(L"/Script/Engine.Default__KismetStringLibrary"));
        
        return Default__KismetStringLibrary->Call<bool>(XOR(L"EqualEqual_StrStr"), A, B);
    }
};

class UFortKismetLibrary : public UObject
{
public:
};