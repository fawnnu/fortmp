#pragma once

class UGameplayStatics : public UObject
{
public:
    APlayerController* GetPlayerController(UWorld* WorldContextObject, int PlayerIndex)
    {
        return Call<APlayerController*>(L"GetPlayerController", WorldContextObject, PlayerIndex);
    }

    UObject* SpawnObject(UObject* Class, UObject* Outer)
    {
        return Call<UObject*>(L"SpawnObject", Class, Outer);
    }

    TArray<AActor*> GetAllActorsOfClass(UWorld* WorldContextObject, UObject* ActorClass)
    {
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

        // TODO: use Call
        ProcessEvent(this, function, &function_params);

        return function_params.OutActors;
    }
};
