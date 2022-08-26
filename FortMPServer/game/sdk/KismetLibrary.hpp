#pragma once

struct FTimerHandle
{
    uint64_t Handle;
};

class UKismetSystemLibrary : public UObject
{
public:
    static void SetBoolPropertyByName(UObject* Target, FName PropertyName, bool Value)
    {
        // Static function so use default object
        static UObject* Default__KismetSystemLibrary = FindObject(L"/Script/Engine.Default__KismetSystemLibrary");

        Default__KismetSystemLibrary->Call(L"SetBoolPropertyByName", Target, PropertyName, Value);
    }

    static void ExecuteConsoleCommand(FString Command)
    {
        static UObject* Default__KismetSystemLibrary = FindObject(L"/Script/Engine.Default__KismetSystemLibrary");
        
        Default__KismetSystemLibrary->Call(L"ExecuteConsoleCommand", *GWorld, Command, nullptr);
    }

    static FTimerHandle K2_SetTimer(UObject* Target, FString FunctionName, float Time, bool bLooping)
    {
        static UObject* Default__KismetSystemLibrary = FindObject(L"/Script/Engine.Default__KismetSystemLibrary");
        static UFunction* SetTimer = FindObject<UFunction>(L"/Script/Engine.KismetSystemLibrary.K2_SetTimer");
        
        struct
        {
            UObject* Target;
            FString FunctionName;
            float Time;
            bool bLooping;
            float InitialStartDelay;
            float InitialStartDelayVariance;
            FTimerHandle ReturnValue;
        } params;

        params.Target = Target;
        params.FunctionName = FunctionName;
        params.Time = Time;
        params.bLooping = bLooping;
        params.InitialStartDelay = 0;
        params.InitialStartDelayVariance = 0;

        ProcessEvent(Default__KismetSystemLibrary, SetTimer, &params);

        return params.ReturnValue;
    }
};

class UKismetMathLibrary : public UObject
{
public:
    static float RandomFloatInRange(float Min, float Max)
    {
        // Static function to use default object
        static UObject* Default__KismetMathLibrary = FindObject(L"/Script/Engine.Default__KismetMathLibrary");
        
        return Default__KismetMathLibrary->Call<float>(L"RandomFloatInRange", Min, Max);
    }
    
    static int RandomIntegerInRange(int Min, int Max)
    {
        // Static function to use default object
        static UObject* Default__KismetMathLibrary = FindObject(L"/Script/Engine.Default__KismetMathLibrary");
        
        return Default__KismetMathLibrary->Call<int>(L"RandomIntegerInRange", Min, Max);
    }
    
    static bool RandomBool()
    {
        // Static function to use default object
        static UObject* Default__KismetMathLibrary = FindObject(L"/Script/Engine.Default__KismetMathLibrary");
        
        return Default__KismetMathLibrary->Call<bool>(L"RandomBool");
    }

    //better than reimplementing IsA!
    static bool ClassIsChildOf(UObject* in_test_class, UObject* parent_class)
    {
        // Static function to use default object
        static UObject* Default__KismetMathLibrary = FindObject(L"/Script/Engine.Default__KismetMathLibrary");
        
        return Default__KismetMathLibrary->Call<bool>(L"ClassIsChildOf", in_test_class, parent_class);
    }
};

class UKismetStringLibrary : public UObject
{
public:
    static bool Split(FString source_string, FString in_str, FString* left_str, FString* right_str, uint8_t search_case, uint8_t search_dir)
    {
        static UKismetStringLibrary* Default__KismetStringLibrary = FindObject<UKismetStringLibrary>(L"/Script/Engine.Default__KismetStringLibrary");

        static UFunction* function = FindObject<UFunction>(L"/Script/Engine.KismetStringLibrary.Split");
        
        struct params
        {
            FString SourceString;

            FString InStr;

            FString LeftStr; //out parm

            FString RightStr; //out parm

            uint8_t SearchCase;
            
            uint8_t SearchDir;

            bool ReturnValue;
        } function_params;

        function_params.SourceString = source_string;
        function_params.InStr = in_str;
        function_params.SearchCase = search_case;
        function_params.SearchDir = search_dir;

        ProcessEvent(Default__KismetStringLibrary, function, &function_params);

        *left_str = function_params.LeftStr;
        *right_str = function_params.RightStr;

        return function_params.ReturnValue;
    }

    static FString Concat(FString A, FString B)
    {
        static UKismetStringLibrary* Default__KismetStringLibrary = FindObject<UKismetStringLibrary>(L"/Script/Engine.Default__KismetStringLibrary");
        
        return Default__KismetStringLibrary->Call<FString>(L"Concat_StrStr", A, B);
    }

    static bool Equals(FString A, FString B)
    {
        static UKismetStringLibrary* Default__KismetStringLibrary = FindObject<UKismetStringLibrary>(L"/Script/Engine.Default__KismetStringLibrary");
        
        return Default__KismetStringLibrary->Call<bool>(L"EqualEqual_StrStr", A, B);
    }
};

class UFortKismetLibrary : public UObject
{
public:
    static bool EqualEqual_UniqueNetIdReplUniqueNetIdRepl(FUniqueNetIdRepl A, FUniqueNetIdRepl B)
    {
        // Static function to use default object
        static UObject* Default__FortKismetLibrary = FindObject(L"/Script/FortniteGame.Default__FortKismetLibrary");
        
        return Default__FortKismetLibrary->Call<bool>(L"EqualEqual_UniqueNetIdReplUniqueNetIdRepl", A, B);
    }
};