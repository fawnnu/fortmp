#pragma once

class UNetDriver : public UObject
{
public:
    DEFINE_PROPERTY(UObject*, ReplicationDriver);

    DEFINE_PROPERTY(TArray<UNetConnection*>, ClientConnections);

    DEFINE_PROPERTY(float, RelevantTimeout);
    DEFINE_PROPERTY(float, Time);

    DEFINE_PROPERTY(FName, NetDriverName);

    DEFINE_PROPERTY(int, NetServerMaxTickRate);

    DEFINE_PROPERTY(int, MaxInternetClientRate);
    DEFINE_PROPERTY(int, MaxClientRate);
};

class UFortReplicationGraph : public UObject
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortReplicationGraph");
};