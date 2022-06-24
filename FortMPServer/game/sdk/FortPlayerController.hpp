#pragma once

class UNetConnection;

class AController : public AActor
{
public:
    DEFINE_PROPERTY(UNetConnection*, NetConnection);
    
    void Possess(AFortPlayerPawn* InPawn)
    {
        static UFunction* function = FindObject<UFunction>(L"/Script/Engine.Controller.Possess");
        
        // TODO: use Call
        ProcessEvent(this, function, &InPawn);
    }
};

class APlayerController : public AController
{
public:
    DEFINE_PROPERTY(AFortPlayerStateAthena, PlayerState);

    void SwitchLevel(FString URL)
    {
        static UFunction* function = FindObject<UFunction>(L"/Script/Engine.PlayerController.SwitchLevel");
        
        // TODO: use Call
        ProcessEvent(this, function, &URL);
    }
};

class AFortPlayerController : public APlayerController
{
public:
    DEFINE_PROPERTY(bool, bHasServerFinishedLoading);

    DEFINE_PROPERTY(AActor*, QuickBars);

    bool IsInAircraft()
    {
        static UFunction* function = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerAthena.IsInAircraft");

        bool bIsInAircraft;
        
        // TODO: use Call
        ProcessEvent(this, function, &bIsInAircraft);

        return bIsInAircraft;
    }

    void ServerAttemptAircraftJump(FRotator ClientRotation)
    {
        static UFunction* function = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerAthena.ServerAttemptAircraftJump");

        // TODO: use Call
        ProcessEvent(this, function, &ClientRotation);
    }
};
