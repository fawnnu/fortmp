#pragma once

struct FTimerHandle
{
    uint64_t Handle;
};

class UKismetSystemLibrary : public UObject
{
public:
    static bool SphereOverlapActors(UObject* WorldContextObject, FVector SpherePos, float SphereRadius, TArray<uint8_t> ObjectTypes, UObject* ActorClassFilter, TArray<AActor*> ActorsToIgnore, TArray<AActor*>* OutActors)
    {
        static UObject* Default__KismetSystemLibrary = FindObject(L"/Script/Engine.Default__KismetSystemLibrary");
        
        struct
        {
            UObject* WorldContextObject;
            FVector SpherePos;
            float SphereRadius;
            TArray<uint8_t> ObjectTypes;
            UObject* ActorClassFilter;
            TArray<AActor*> ActorsToIgnore;
            TArray<AActor*> OutActors;
            bool ReturnValue;
        } parameters;

        parameters.WorldContextObject = WorldContextObject;
        parameters.SpherePos = SpherePos;
        parameters.SphereRadius = SphereRadius;
        parameters.ObjectTypes = ObjectTypes;
        parameters.ActorClassFilter = ActorClassFilter;
        parameters.ActorsToIgnore = ActorsToIgnore;

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.KismetSystemLibrary.SphereOverlapActors")
        
        ProcessEvent(Default__KismetSystemLibrary, FunctionObject, &parameters);

        if (OutActors)
        {
            *OutActors = parameters.OutActors;
        }
        
        return parameters.ReturnValue;
    }
    
    static void SetBoolPropertyByName(UObject* Target, FName PropertyName, bool Value)
    {
        // Static function so use default object
        static UObject* Default__KismetSystemLibrary = FindObject(L"/Script/Engine.Default__KismetSystemLibrary");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.KismetSystemLibrary.SetBoolPropertyByName")
        
        Default__KismetSystemLibrary->Call(FunctionObject, Target, PropertyName, Value);
    }

    static void ExecuteConsoleCommand(FString Command, class APlayerController* SpecificPlayer = nullptr)
    {
        static UObject* Default__KismetSystemLibrary = FindObject(L"/Script/Engine.Default__KismetSystemLibrary");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.KismetSystemLibrary.ExecuteConsoleCommand")
        
        Default__KismetSystemLibrary->Call(FunctionObject, *GWorld, Command, SpecificPlayer);
    }

    static FString GetPathName(UObject* Object)
    {
        static UObject* Default__KismetSystemLibrary = FindObject(L"/Script/Engine.Default__KismetSystemLibrary");
        static UFunction* GetPathName = FindObject<UFunction>(L"/Script/Engine.KismetSystemLibrary.GetPathName");

        struct
        {
            UObject* Object;
            FString ReturnValue;
        } params;

        params.Object = Object;

        ProcessEvent(Default__KismetSystemLibrary, GetPathName, &params);

        return params.ReturnValue;
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

    static void K2_ClearTimer(UObject* Target, FString FunctionName)
    {
        static UObject* Default__KismetSystemLibrary = FindObject(L"/Script/Engine.Default__KismetSystemLibrary");
        static UFunction* K2_ClearTimer = FindObject<UFunction>(L"/Script/Engine.KismetSystemLibrary.K2_ClearTimer");

        struct
        {
            UObject* Target;
            FString FunctionName;
        } params;

        params.Target = Target;
        params.FunctionName = FunctionName;

        ProcessEvent(Default__KismetSystemLibrary, K2_ClearTimer, &params);
    }

    static bool K2_IsTimerActive(UObject* Target, FString FunctionName)
    {
        static UObject* Default__KismetSystemLibrary = FindObject(L"/Script/Engine.Default__KismetSystemLibrary");
        static UFunction* K2_IsTimerActive = FindObject<UFunction>(L"/Script/Engine.KismetSystemLibrary.K2_IsTimerActive");
        
        struct
        {
            UObject* Target;
            FString FunctionName;
            bool ReturnValue;
        } params;

        params.Target = Target;
        params.FunctionName = FunctionName;

        ProcessEvent(Default__KismetSystemLibrary, K2_IsTimerActive, &params);

        return params.ReturnValue;
    }
};

class UKismetMathLibrary : public UObject
{
public:
    static FVector Conv_RotatorToVector(FRotator InRotator)
    {
        // Static function to use default object
        static UObject* Default__KismetMathLibrary = FindObject(L"/Script/Engine.Default__KismetMathLibrary");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.KismetMathLibrary.Conv_RotatorToVector")
        
        return Default__KismetMathLibrary->Call<FVector>(FunctionObject, InRotator);
    }
    
    static FVector RandomUnitVectorInConeInDegrees(FVector InDir, float ConeHalfAngle)
    {
        // Static function to use default object
        static UObject* Default__KismetMathLibrary = FindObject(L"/Script/Engine.Default__KismetMathLibrary");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.KismetMathLibrary.RandomUnitVectorInConeInDegrees")
        
        return Default__KismetMathLibrary->Call<FVector>(FunctionObject, InDir, ConeHalfAngle);
    }

    static FVector Add_VectorVector(FVector A, FVector B)
    {
        // Static function to use default object
        static UObject* Default__KismetMathLibrary = FindObject(L"/Script/Engine.Default__KismetMathLibrary");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.KismetMathLibrary.Add_VectorVector")
        
        return Default__KismetMathLibrary->Call<FVector>(FunctionObject, A, B); 
    }

    static FVector GetRightVector(FRotator InRot)
    {
        // Static function to use default object
        static UObject* Default__KismetMathLibrary = FindObject(L"/Script/Engine.Default__KismetMathLibrary");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.KismetMathLibrary.GetRightVector")
        
        return Default__KismetMathLibrary->Call<FVector>(FunctionObject, InRot); 
    }
    
    static float RandomFloatInRange(float Min, float Max)
    {
        // Static function to use default object
        static UObject* Default__KismetMathLibrary = FindObject(L"/Script/Engine.Default__KismetMathLibrary");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.KismetMathLibrary.RandomFloatInRange")
        
        return Default__KismetMathLibrary->Call<float>(FunctionObject, Min, Max);
    }
    
    static int RandomIntegerInRange(int Min, int Max)
    {
        // Static function to use default object
        static UObject* Default__KismetMathLibrary = FindObject(L"/Script/Engine.Default__KismetMathLibrary");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.KismetMathLibrary.RandomIntegerInRange")
        
        return Default__KismetMathLibrary->Call<int>(FunctionObject, Min, Max);
    }
    
    static bool RandomBool()
    {
        // Static function to use default object
        static UObject* Default__KismetMathLibrary = FindObject(L"/Script/Engine.Default__KismetMathLibrary");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.KismetMathLibrary.RandomBool")
        
        return Default__KismetMathLibrary->Call<bool>(FunctionObject);
    }

    static int Round(float A)
    {
        // Static function to use default object
        static UObject* Default__KismetMathLibrary = FindObject(L"/Script/Engine.Default__KismetMathLibrary");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.KismetMathLibrary.Round")
        
        return Default__KismetMathLibrary->Call<int>(FunctionObject);
    }
    
    static bool RandomBoolWithWeight(float Weight)
    {
        // Static function to use default object
        static UObject* Default__KismetMathLibrary = FindObject(L"/Script/Engine.Default__KismetMathLibrary");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.KismetMathLibrary.RandomBoolWithWeight")
        
        return Default__KismetMathLibrary->Call<bool>(FunctionObject, Weight);
    }

    static FRotator Quat_Rotator(FQuat Q)
    {
        // Static function to use default object
        static UObject* Default__KismetMathLibrary = FindObject(L"/Script/Engine.Default__KismetMathLibrary");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.KismetMathLibrary.Quat_Rotator")
        
        return Default__KismetMathLibrary->Call<FRotator>(FunctionObject, Q);
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

    static bool Equals(FString A, FString B)
    {
        static UKismetStringLibrary* Default__KismetStringLibrary = FindObject<UKismetStringLibrary>(L"/Script/Engine.Default__KismetStringLibrary");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.KismetStringLibrary.EqualEqual_StrStr")
        
        return Default__KismetStringLibrary->Call<bool>(FunctionObject, A, B);
    }
};

class UFortKismetLibrary : public UObject
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortKismetLibrary")
    
    static bool EqualEqual_UniqueNetIdReplUniqueNetIdRepl(FUniqueNetIdRepl A, FUniqueNetIdRepl B)
    {
        // Static function to use default object
        static UObject* Default__FortKismetLibrary = FindObject(L"/Script/FortniteGame.Default__FortKismetLibrary");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/FortniteGame.FortKismetLibrary.EqualEqual_UniqueNetIdReplUniqueNetIdRepl")
        
        return Default__FortKismetLibrary->Call<bool>(FunctionObject, A, B);
    }
    
    static void K2_GiveItemToAllPlayers(class UObject* WorldContextObject, class UFortWorldItemDefinition* ItemDefinition, int32 NumberToGive, bool bNotifyPlayer)
    {
        // Static function to use default object
        static UObject* Default__FortKismetLibrary = FindObject(L"/Script/FortniteGame.Default__FortKismetLibrary");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/FortniteGame.FortKismetLibrary.K2_GiveItemToAllPlayers")
        
        Default__FortKismetLibrary->Call(FunctionObject, WorldContextObject, ItemDefinition, NumberToGive, bNotifyPlayer);
    }
};

class UAbilitySystemBlueprintLibrary : public UObject
{
public:
    static void SendGameplayEventToActor(AActor* Actor, FGameplayTag EventTag, FGameplayEventData* Payload)
    {
        static UObject* Default__AbilitySystemBlueprintLibrary = FindObject(L"/Script/GameplayAbilities.Default__AbilitySystemBlueprintLibrary");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/GameplayAbilities.AbilitySystemBlueprintLibrary.SendGameplayEventToActor");

        // GET PARAMS SIZE
        int16_t parms_size = 0;
            
        for (UObject* Property = *reinterpret_cast<UObject**>(__int64(FunctionObject) + CoreOffsets::Children); Property; Property = *reinterpret_cast<UObject**>(__int64(Property) + CoreOffsets::Next))
        {
            int offset = *reinterpret_cast<int*>(__int64(Property) + CoreOffsets::Offset_Internal);
            
            parms_size = offset + // Offset_Internal
                *reinterpret_cast<int*>(__int64(Property) + CoreOffsets::ElementSize); // ElementSize
        }

        // SET PARAMS
        struct FunctionParams
        {
            AActor* Actor;
            FGameplayTag EventTag;
            FGameplayEventData Payload;
        };

        FunctionParams* params = static_cast<FunctionParams*>(alloca(parms_size));
        params->Actor = Actor;
        params->EventTag = EventTag;
        memcpy(&params->Payload, Payload, FGameplayEventData::GetStructSize());

        ProcessEvent(Default__AbilitySystemBlueprintLibrary, FunctionObject, params);
    }
};

class UAIBlueprintHelperLibrary : public UObject
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortKismetLibrary")
    
    static class AController* GetAIController(AActor* ControlledActor)
    {
        // Static function to use default object
        static UObject* Default__AIBlueprintHelperLibrary = FindObject(L"/Script/AIModule.Default__AIBlueprintHelperLibrary");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/AIModule.AIBlueprintHelperLibrary.GetAIController")
        
        return Default__AIBlueprintHelperLibrary->Call<AController*>(FunctionObject, ControlledActor);
    }
};