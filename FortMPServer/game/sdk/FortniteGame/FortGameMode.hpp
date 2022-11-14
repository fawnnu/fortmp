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
    static UObject* StaticClass()
    {
        static UObject* static_class = FindObject(L"/Script/FortniteGame.FortGameModeAthena");

        return static_class;
    }
    
    DEFINE_PROPERTY(float, WarmupCountdownDuration);
    DEFINE_PROPERTY(float, WarmupEarlyCountdownDuration);
    DEFINE_PROPERTY(float, EndGameKickPlayersDelay);
    
    DEFINE_PROPERTY(int, SafeZonePhase);
    
    DEFINE_PROPERTY(bool, bMapInfoInitialized);
    
    DEFINE_PROPERTY(AFortSafeZoneIndicator*, SafeZoneIndicator);

    DEFINE_PROPERTY(TArray<AFortPlayerController*>, AlivePlayers);

    // Custom event
    void OnServerReady()
    {
        Call(L"OnServerReady");
    }

    void OnPlayerJoined(APlayerController* NewPlayer)
    {
        if (UFunction* TargetFunction = static_cast<UFunction*>(FindFunctionByName(this->Class, L"OnPlayerJoined", 1)))
        {
            struct
            {
                APlayerController* NewPlayer;
            } Params;

            Params.NewPlayer = NewPlayer;

            ProcessEvent(this, TargetFunction, &Params);
        }
    }

    bool& OnUpdateStorm()
    {
        if (UFunction* TargetFunction = static_cast<UFunction*>(FindFunctionByName(this->Class, L"OnUpdateStorm", 1)))
        {
            struct
            {
                bool ReturnValue;
            } Params;

            ProcessEvent(this, TargetFunction, &Params);

            return Params.ReturnValue;
        }
    }
    
    UFortPlaylistAthena*& OnSetPlaylist()
    {
        if (UFunction* TargetFunction = static_cast<UFunction*>(FindFunctionByName(this->Class, L"OnSetPlaylist", 1)))
        {
            struct
            {
                UFortPlaylistAthena* ReturnValue;
            } Params;

            ProcessEvent(this, TargetFunction, &Params);

            return Params.ReturnValue;
        }
    }
    
    void OnEnterAircraft(APlayerController* TargetPlayer)
    {
        if (UFunction* TargetFunction = static_cast<UFunction*>(FindFunctionByName(this->Class, L"OnEnterAircraft", 1)))
        {
            struct
            {
                APlayerController* NewPlayer;
            } Params;

            Params.NewPlayer = TargetPlayer;

            ProcessEvent(this, TargetFunction, &Params);
        }
    }
};
