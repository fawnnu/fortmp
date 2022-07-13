#pragma once

struct FNetworkObjectInfo;

class replication_component
{
public:
    replication_component();

    static int ServerReplicateActors_PrepConnections(UNetDriver* NetDriver, float DeltaSeconds);
    static void ServerReplicateActors_BuildConsiderList(UNetDriver* NetDriver, std::vector<FNetworkObjectInfo*>& OutConsiderList, float ServerTickTime);
    
    static int ServerReplicateActors(UNetDriver* NetDriver, float DeltaSeconds);
};;

struct FNetworkObjectInfo
{
    // Pointer to the replicated actor
    AActor* Actor;

    // WeakPtr to actor
    TWeakObjectPtr<AActor> WeakActor;

    // Next time to consider replicating the actor
    double NextUpdateTime;
    
    // Last absolute time in seconds since actor actually replicated something
    double LastNetReplicateTime;

    // Optimal delta between replication updates based on how often properties are actually changing
    float OptimalNetUpdateDelta;

    // Last time this actor was updated for replication
    float LastNetUpdateTime;

    // Is object still pending a full net update?
    uint32_t bPendingNetUpdate : 1;

    // Should this object be forced as relevant for at least one frame?
    uint32_t bForceRelevantNextUpdate : 1;

    // List of connections that this actor is dormant on
    TSet<TWeakObjectPtr<UNetConnection>> DormantConnections;

    // List of connections this actor has recently been dormant on
    TSet<TWeakObjectPtr<UNetConnection>> RecentlyDormantConnections;
};

struct FActorDestructionInfo
{
    TWeakObjectPtr<UObject> Level;
    TWeakObjectPtr<UObject> ObjOuter;
    
    // Location of the destroyed actor
    FVector DestroyedPosition;

    // Net GUID of the destroyed actor
    FNetworkGUID NetGUID;

    // Path name of the destroyed actor
    FString PathName;

    // Streaming level that the actor was destroyed in
    FName StreamingLevelName;
};

struct FActorPriority
{
    // Update priority
    int Priority;

    // Actor info
    FNetworkObjectInfo* ActorInfo;

    // Actor channel
    UObject* Channel;

    // Destroy an actor
    FActorDestructionInfo* DestructionInfo;

    FActorPriority() :
        Priority(0), ActorInfo(nullptr), Channel(nullptr), DestructionInfo(nullptr)
    {
    }
};

// Replicates actors using the ReplicationGraph
inline int (*ReplicationGraph_ServerReplicateActors)(UObject* ReplicationDriver, float DeltaSeconds);