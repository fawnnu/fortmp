#pragma once

class UGameplayStatics : public UObject
{
public:
    static APlayerController* GetPlayerController(UWorld* WorldContextObject, int PlayerIndex)
    {
        UGameplayStatics* Default__GameplayStatics = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.GameplayStatics.GetPlayerController")
        
        return Default__GameplayStatics->Call<APlayerController*>(FunctionObject, WorldContextObject, PlayerIndex);
    }

    static void OpenLevel(UWorld* WorldContextObject, FName LevelName, bool bAbsolute, FString Options)
    {
        UGameplayStatics* Default__GameplayStatics = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.GameplayStatics.OpenLevel")
        
        Default__GameplayStatics->Call(FunctionObject, WorldContextObject, LevelName, bAbsolute, Options);
    }

    static UObject* SpawnObject(UObject* Class, UObject* Outer)
    {
        UGameplayStatics* Default__GameplayStatics = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.GameplayStatics.SpawnObject")
        
        return Default__GameplayStatics->Call<UObject*>(FunctionObject, Class, Outer);
    }
    
    static FString ParseOption(FString Options, FString Key)
    {
        UGameplayStatics* Default__GameplayStatics = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.GameplayStatics.ParseOption")

        struct
        {
            FString Options;
            FString Key;
            FString ReturnValue;
        } params;

        params.Options = Options;
        params.Key = Key;
        
        ProcessEvent(Default__GameplayStatics, FunctionObject, &params);

        return params.ReturnValue;
    }
    
    static bool HasOption(FString Options, FString Key)
    {
        UGameplayStatics* Default__GameplayStatics = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.GameplayStatics.HasOption")

        struct
        {
            FString Options;
            FString Key;
            bool ReturnValue;
        } params;

        params.Options = Options;
        params.Key = Key;
        
        ProcessEvent(Default__GameplayStatics, FunctionObject, &params);

        return params.ReturnValue;
    }

    template <typename T = AActor>
    static TArray<T*> GetAllActorsOfClass(UWorld* WorldContextObject, UObject* ActorClass)
    {
        UGameplayStatics* Default__GameplayStatics = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics");
        
        static UFunction* function = FindObject<UFunction>(L"/Script/Engine.GameplayStatics.GetAllActorsOfClass");
        
        struct params
        {
            // World to look for actors in
            UWorld* WorldContextObject;

            // Actor class to look for
            UObject* ActorClass;

            // List of out actors
            TArray<T*> OutActors;
        } function_params;
        
        function_params.WorldContextObject = WorldContextObject;
        function_params.ActorClass = ActorClass;

        ProcessEvent(Default__GameplayStatics, function, &function_params);

        return function_params.OutActors;
    }
};
