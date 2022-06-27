#pragma once

class UNetConnection;

class AController : public AActor
{
public:
    DEFINE_PROPERTY(UNetConnection*, NetConnection);
    
    void Possess(AFortPlayerPawn* InPawn)
    {
        Call(L"Possess", InPawn);
    }
};

class APlayerController : public AController
{
public:
    DEFINE_PROPERTY(AFortPlayerStateAthena*, PlayerState);

    void SwitchLevel(FString URL)
    {
        Call(L"SwitchLevel", URL);
    }
};

class AFortPlayerController : public APlayerController
{
public:
    DEFINE_PROPERTY(bool, bHasServerFinishedLoading);

    DEFINE_PROPERTY(AActor*, QuickBars);

    DEFINE_PROPERTY(UObject*, WorldInventory);

    bool IsInAircraft()
    {
        return Call<bool>(L"IsInAircraft");
    }

    void ServerAttemptAircraftJump(FRotator ClientRotation)
    {
        Call(L"ServerAttemptAircraftJump", ClientRotation);
    }
};
