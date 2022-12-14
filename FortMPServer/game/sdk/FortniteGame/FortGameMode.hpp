#pragma once

class AGameSession : public UObject
{
public:
    DEFINE_PROPERTY(int, MaxPlayers);
};

class AFortGameSessionDedicated : public AGameSession
{
public:
    static UObject* StaticClass()
    {
        static UObject* static_class = FindObject(L"/Script/FortniteGame.FortGameSessionDedicated");
        
        return static_class;
    }
};

class AFortGameSessionDedicatedAthena : public AFortGameSessionDedicated
{
public:
    static UObject* StaticClass()
    {
        static UObject* static_class = FindObject(L"/Script/FortniteGame.FortGameSessionDedicatedAthena");
        
        return static_class;
    }
};

class AGameModeBase : public UObject
{
public:
    DEFINE_PROPERTY(AGameSession*, GameSession);
    DEFINE_PROPERTY(UObject*, GameSessionClass);
    
    AFortPlayerPawn* SpawnDefaultPawnAtTransform(APlayerController* NewPlayer, FTransform SpawnTransform)
    {
        STATIC_FINDFUNCTION(SpawnDefaultPawnAtTransform)
        
        return Call<AFortPlayerPawn*>(Function_SpawnDefaultPawnAtTransform, NewPlayer, SpawnTransform);
    }

    AActor* ChoosePlayerStart(APlayerController* NewPlayer)
    {
        STATIC_FINDFUNCTION(ChoosePlayerStart)
        
        return Call<AActor*>(Function_ChoosePlayerStart, NewPlayer);
    }
};

class AGameMode : public AGameModeBase
{
public:
    DEFINE_PROPERTY(UObject*, DefaultPawnClass);

    void StartMatch()
    {
        STATIC_FINDFUNCTION(StartMatch)
        
        Call(Function_StartMatch);
    }
};

class AFortGameMode : public AGameMode
{
public:
    DEFINE_PROPERTY(TArray<AFortPlayerController*>, ConnectedPlayerControllers);
    DEFINE_PROPERTY(AGameSession*, FortGameSession);
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

    DEFINE_PROPERTY(bool, bDisableGCOnServerDuringMatch);
    
    DEFINE_PROPERTY(float, WarmupCountdownDuration);
    DEFINE_PROPERTY(int, WarmupRequiredPlayerCount);
    DEFINE_PROPERTY(float, WarmupEarlyCountdownDuration);
    DEFINE_PROPERTY(float, EndGameKickPlayersDelay);
    
    DEFINE_PROPERTY(int, SafeZonePhase);
    
    DEFINE_PROPERTY(bool, bMapInfoInitialized);
    
    DEFINE_PROPERTY(AFortSafeZoneIndicator*, SafeZoneIndicator);

    DEFINE_PROPERTY(TArray<AFortPlayerController*>, AlivePlayers);

    // Custom event
    void OnServerReady()
    {
        STATIC_FINDFUNCTION(OnServerReady)
        
        Call(Function_OnServerReady);
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
    
    void OnPawnFinishedSpawning(AFortPawn* TargetPawn)
    {
        if (UFunction* TargetFunction = static_cast<UFunction*>(FindFunctionByName(this->Class, L"OnPawnFinishedSpawning", 1)))
        {
            struct
            {
                AFortPawn* TargetPawn;
            } Params;

            Params.TargetPawn = TargetPawn;

            ProcessEvent(this, TargetFunction, &Params);
        }
    }
};
