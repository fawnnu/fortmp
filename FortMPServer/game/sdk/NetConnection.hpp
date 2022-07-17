#pragma once

class UNetDriver;
class UChildConnection;

class UPlayer : public UObject
{
public:
    DEFINE_PROPERTY(class AFortPlayerController*, PlayerController);
};

class UPackageMap : public UObject
{
public:
};

class UNetConnection : public UPlayer
{
public:
    DEFINE_PROPERTY(TArray<UChildConnection*>, Children);
    
    DEFINE_PROPERTY(AActor*, ViewTarget);
    DEFINE_PROPERTY(AActor*, OwningActor);

    DEFINE_PROPERTY(UNetDriver*, Driver);
    
    DEFINE_PROPERTY(FUniqueNetIdRepl, PlayerID);

    DEFINE_PROPERTY(uint8_t, InternalAck);

    DEFINE_PROPERTY(double, LastReceiveTime);

    DEFINE_PROPERTY(TArray<UChannel*>, OpenChannels);

    DEFINE_PROPERTY(UPackageMap*, PackageMap);
};

class UChildConnection : public UNetConnection
{
public:
    DEFINE_PROPERTY(UNetConnection*, Parent);
};
