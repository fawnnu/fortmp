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

class AFortInventory;

class AFortPlayerController : public APlayerController
{
public:
    DEFINE_PROPERTY(AActor*, Pawn);
    
    DEFINE_PROPERTY(bool, bHasServerFinishedLoading);

    DEFINE_PROPERTY(class AFortQuickBars*, QuickBars);
    DEFINE_PROPERTY(class AFortQuickBars*, ClientQuickBars);

    DEFINE_PROPERTY(AFortInventory*, WorldInventory);

    bool IsInAircraft()
    {
        return Call<bool>(L"IsInAircraft");
    }

    void ServerAttemptAircraftJump(FRotator ClientRotation)
    {
        Call(L"ServerAttemptAircraftJump", ClientRotation);
    }
};
