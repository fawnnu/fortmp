#pragma once

struct FLatentActionInfo
{
public:
    int32                                        Linkage;                                           // 0x0(0x4)(ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
    int32                                        UUID;                                              // 0x4(0x4)(ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
    class FName                                  ExecutionFunction;                                 // 0x8(0x8)(ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
    class UObject*                               CallbackTarget;                                    // 0x10(0x8)(ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
};

class UGameplayStatics : public UObject
{
public:
    DEFINE_STATIC_CLASS(L"/Script/Engine.GameplayStatics");;
    static APlayerController* GetPlayerController(UWorld* WorldContextObject, int PlayerIndex)
    {
        UGameplayStatics* Default__GameplayStatics = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.GameplayStatics.GetPlayerController")
        
        return Default__GameplayStatics->Call<APlayerController*>(FunctionObject, WorldContextObject, PlayerIndex);
    }

    static void UnloadStreamLevel(UWorld* WorldContextObject, FName LevelToUnload, FLatentActionInfo LatentInfo, bool bShouldBlockOnUnload)
    {
        UGameplayStatics* Default__GameplayStatics = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.GameplayStatics.UnloadStreamLevel")
        
        Default__GameplayStatics->Call(FunctionObject, WorldContextObject, LevelToUnload, LatentInfo, bShouldBlockOnUnload);
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

    static AActor* FinishSpawningActor(AActor* Actor, FTransform SpawnTransform)
    {
        UGameplayStatics* Default__GameplayStatics = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.GameplayStatics.FinishSpawningActor")
        
        return Default__GameplayStatics->Call<AActor*>(FunctionObject, Actor, SpawnTransform);
    }

    static AActor* BeginDeferredActorSpawnFromClass(UObject* WorldContextObject, UObject* ActorClass, FTransform SpawnTransform, uint8_t CollisionHandlingOverride, AActor* Owner)
    {
        UGameplayStatics* Default__GameplayStatics = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/Engine.GameplayStatics.BeginDeferredActorSpawnFromClass")
        
        return Default__GameplayStatics->Call<AActor*>(FunctionObject, WorldContextObject, ActorClass, SpawnTransform, CollisionHandlingOverride, Owner);
    }

    static AActor* SpawnActor(UObject* ActorClass, FTransform SpawnTransform, uint8_t CollisionHandlingOverride, AActor* Owner)
    {
        auto preactor = BeginDeferredActorSpawnFromClass((*GWorld), ActorClass, SpawnTransform, CollisionHandlingOverride, Owner);
        return FinishSpawningActor(preactor, SpawnTransform);
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
