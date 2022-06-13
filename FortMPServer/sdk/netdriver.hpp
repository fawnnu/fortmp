#pragma once

DEFINE_UCLASS(UNetDriver, "/Script/Engine.NetDriver", public UObject)
{
    public:
    GENERATE_STATIC_CLASS(UNetDriver);

    DEFINE_UPROPERTY(UReplicationDriver*, ReplicationDriver, UNetDriver);
};