#pragma once

class UGameplayStatics : public UObject
{
public:
    static APlayerController* GetPlayerController(UWorld* WorldContextObject, int PlayerIndex)
    {
        UGameplayStatics* Default__GameplayStatics = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics");
        
        return Default__GameplayStatics->Call<APlayerController*>(L"GetPlayerController", WorldContextObject, PlayerIndex);
    }

    static UObject* SpawnObject(UObject* Class, UObject* Outer)
    {
        UGameplayStatics* Default__GameplayStatics = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics");
        
        return Default__GameplayStatics->Call<UObject*>(L"SpawnObject", Class, Outer);
    }

    static TArray<AActor*> GetAllActorsOfClass(UWorld* WorldContextObject, UObject* ActorClass)
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
            TArray<AActor*> OutActors;
        } function_params;
        
        function_params.WorldContextObject = WorldContextObject;
        function_params.ActorClass = ActorClass;

        ProcessEvent(Default__GameplayStatics, function, &function_params);

        return function_params.OutActors;
    }
};
