#pragma once

class UPlayer : public UObject
{
public:
    DEFINE_PROPERTY(class AFortPlayerController*, PlayerController);
};

class UNetConnection : public UPlayer
{
public:
    DEFINE_PROPERTY(AActor*, ViewTarget);
    
    DEFINE_PROPERTY(FUniqueNetIdRepl, PlayerID);
};
