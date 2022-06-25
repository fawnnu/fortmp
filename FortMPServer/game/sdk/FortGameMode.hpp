#pragma once

class AGameModeBase : public UObject
{
public:
    AFortPlayerPawn* SpawnDefaultPawnAtTransform(APlayerController* NewPlayer, FTransform SpawnTransform)
    {
        static UFunction* function = FindObject<UFunction>(L"/Script/Engine.GameModeBase.SpawnDefaultPawnAtTransform");
        
        struct params
        {
            // Player to spawn pawn for
            APlayerController* NewPlayer;

            // Location to spawn pawn at
            FTransform SpawnTransform;

            // Pawn that has been spawned
            AFortPlayerPawn* SpawnedPawn;
        } function_params;
        
        function_params.NewPlayer = NewPlayer;
        function_params.SpawnTransform = SpawnTransform;

        // TODO: use Call
        ProcessEvent(this, function, &function_params);

        return function_params.SpawnedPawn;
    }
};

class AGameMode : public AGameModeBase
{
public:
    DEFINE_PROPERTY(UObject*, DefaultPawnClass);

    void StartMatch()
    {
        static UFunction* function = FindObject<UFunction>(L"/Script/Engine.GameMode.StartMatch");
        
        // TODO: use Call
        ProcessEvent(this, function, nullptr);
    }
};

class AFortGameModeZone : public AGameMode
{
public:
    //DEFINE_PROPERTY(TArray<struct FItemAndCount>, StartingItems);
};

class AFortGameModeAthena : public AFortGameModeZone
{
public:
    DEFINE_PROPERTY(float, WarmupCountdownDuration);
    DEFINE_PROPERTY(float, WarmupEarlyCountdownDuration);
};
