#pragma once

class AController : public AActor
{
public:
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
};
