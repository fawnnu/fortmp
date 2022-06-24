#pragma once

class UGameplayStatics : public UObject
{
public:
    APlayerController* GetPlayerController(UWorld* WorldContextObject, int PlayerIndex)
    {
        static UFunction* function = FindObject<UFunction>(L"/Script/Engine.GameplayStatics.GetPlayerController");
        
        struct params
        {
            // World to look for player in
            UWorld* WorldContextObject;

            // Player to find by index
            int PlayerIndex;

            // Player that was found (if any)
            APlayerController* ReturnValue;
        } function_params;

        function_params.WorldContextObject = WorldContextObject;
        function_params.PlayerIndex = PlayerIndex;

        // TODO: use Call
        ProcessEvent(this, function, &function_params);

        return function_params.ReturnValue;
    }

    TArray<UObject*> GetAllActorsOfClass(UWorld* WorldContextObject, UObject* ActorClass)
    {
        static UFunction* function = FindObject<UFunction>(L"/Script/Engine.GameplayStatics.GetAllActorsOfClass");
        
        struct params
        {
            // World to look for actors in
            UWorld* WorldContextObject;

            // Actor class to look for
            UObject* ActorClass;

            // List of out actors
            TArray<UObject*> OutActors;
        } function_params;
        
        function_params.WorldContextObject = WorldContextObject;
        function_params.ActorClass = ActorClass;

        // TODO: use Call
        ProcessEvent(this, function, &function_params);

        return function_params.OutActors;
    }
};
