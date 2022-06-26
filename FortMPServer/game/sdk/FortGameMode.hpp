#pragma once

class AGameModeBase : public UObject
{
public:
    AFortPlayerPawn* SpawnDefaultPawnAtTransform(APlayerController* NewPlayer, FTransform SpawnTransform)
    {
        return Call<AFortPlayerPawn*>(L"SpawnDefaultPawnAtTransform", NewPlayer, SpawnTransform);
    }
};

class AGameMode : public AGameModeBase
{
public:
    DEFINE_PROPERTY(UObject*, DefaultPawnClass);

    void StartMatch()
    {
        Call(L"StartMatch");
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
