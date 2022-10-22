#pragma once

class AGameSession : public UObject
{
public:
    DEFINE_PROPERTY(int, MaxPlayers);
};

class AGameModeBase : public UObject
{
public:
    DEFINE_PROPERTY(AGameSession*, GameSession);
    
    AFortPlayerPawn* SpawnDefaultPawnAtTransform(APlayerController* NewPlayer, FTransform SpawnTransform)
    {
        return Call<AFortPlayerPawn*>(L"SpawnDefaultPawnAtTransform", NewPlayer, SpawnTransform);
    }

    AActor* ChoosePlayerStart(APlayerController* NewPlayer)
    {
        return Call<AActor*>(L"ChoosePlayerStart", NewPlayer);
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

class AFortGameMode : public AGameMode
{
public:
    DEFINE_PROPERTY(TArray<AFortPlayerController*>, ConnectedPlayerControllers);
};

class AFortGameModeZone : public AFortGameMode
{
public:
    DEFINE_PROPERTY(TArray<struct FItemAndCount>, StartingItems);
};

class AFortGameModeAthena : public AFortGameModeZone
{
public:
    DEFINE_PROPERTY(float, WarmupCountdownDuration);
    DEFINE_PROPERTY(float, WarmupEarlyCountdownDuration);
    DEFINE_PROPERTY(float, EndGameKickPlayersDelay);
    
    DEFINE_PROPERTY(int, SafeZonePhase);
    
    DEFINE_PROPERTY(bool, bMapInfoInitialized);
    
    DEFINE_PROPERTY(AFortSafeZoneIndicator*, SafeZoneIndicator);

    DEFINE_PROPERTY(TArray<AFortPlayerController*>, AlivePlayers);
};
