#pragma once

struct FNetworkObjectInfo;
struct FActorPriority;

class replication_component
{
public:
    replication_component();

    static UChannel* FindChannelByActor(UNetConnection* Connection, AActor* InActor);

    static int ServerReplicateActors_PrepConnections(UNetDriver* NetDriver, float DeltaSeconds);
    static void ServerReplicateActors_BuildConsiderList(UNetDriver* NetDriver, std::vector<FNetworkObjectInfo*>& OutConsiderList, float ServerTickTime);
    static int ServerReplicateActors_PrioritizeActors(UNetDriver* NetDriver, UNetConnection* Connection, TArray<FNetViewer>& ConnectionViewers, std::vector<FNetworkObjectInfo*> ConsiderList, bool bCPUSaturated, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors);
    static int ServerReplicateActors_ProcessPrioritizedActors(UNetDriver* NetDriver, UNetConnection* Connection, TArray<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, int FinalSortedCount, int& OutUpdated);
    
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

struct FNetworkObjectList
{
    TSet<TSharedPtr<FNetworkObjectInfo>> AllNetworkObjects;
    TSet<TSharedPtr<FNetworkObjectInfo>> ActiveNetworkObjects;
    TSet<TSharedPtr<FNetworkObjectInfo>> ObjectsDormantOnAllConnections;

    TMap<TWeakObjectPtr<UObject>, int32_t> NumDormantObjectsPerConnection;
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
    UActorChannel* Channel;

    // Destroy an actor
    FActorDestructionInfo* DestructionInfo;

    FActorPriority() :
        Priority(0), ActorInfo(nullptr), Channel(nullptr), DestructionInfo(nullptr)
    {
    }
};

// Replicates actors using the ReplicationGraph
inline int (*ReplicationGraph_ServerReplicateActors)(UObject* ReplicationDriver, float DeltaSeconds);

// Called on server at end of tick, to let client Pawns handle updates from the server.
inline void (*SendClientAdjustment)(APlayerController*);

// Called by the networking system to call PreReplication on this actor and its components using the given NetDriver to find or create RepChangedPropertyTrackers.
inline void (*CallPreReplication)(AActor*, UNetDriver*);

// Creates an actor channel
inline UChannel* (*CreateChannel)(UNetConnection*, int, bool, int);

// Sets the actor channel
inline void (*SetChannelActor)(UChannel*, AActor*);

// Called to replicate an actor
inline bool (*ReplicateActor)(UChannel*);