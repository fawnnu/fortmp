#pragma once

DEFINE_UCLASS(APlayerController, "/Script/Engine.PlayerController", public UObject)
{
    public:

    GENERATE_STATIC_CLASS(APlayerController);

    // Player pointer in APlayerController
    DEFINE_UPROPERTY(UObject*, Player, APlayerController);
};