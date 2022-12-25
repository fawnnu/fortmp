#pragma once

struct FNetworkObjectInfo;
struct FActorPriority;
struct FActorDestructionInfo;

class replication_component
{
public:
    replication_component();

    static UChannel* FindChannelByActor(UNetConnection* Connection, AActor* InActor);

    static bool IsActorRelevantToConnection(AActor* Actor, TArray<FNetViewer>& ConnectionViewers);
    static bool IsActorDormant(FNetworkObjectInfo* ActorInfo, TWeakObjectPtr<UNetConnection>& Connection);
    static bool ShouldActorGoDormant(AActor* Actor, TArray<FNetViewer>& ConnectionViewers, UActorChannel* Channel, float Time, bool bLowNetBandwidth);
    
    static UNetConnection* IsActorOwnedByAndRelevantToConnection(AActor* Actor, TArray<FNetViewer>& ConnectionViewers, bool& bOutHasNullViewTarget );

    static void SetChannelActorForDestroy(UChannel* Channel, FActorDestructionInfo* InDestructionInfo);
    
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
    UE::TSet<TWeakObjectPtr<UNetConnection>> DormantConnections;

    // List of connections this actor has recently been dormant on
    UE::TSet<TWeakObjectPtr<UNetConnection>> RecentlyDormantConnections;
};

struct FNetworkObjectList
{
    UE::TSet<TSharedPtr<FNetworkObjectInfo>> AllNetworkObjects;
    UE::TSet<TSharedPtr<FNetworkObjectInfo>> ActiveNetworkObjects;
    UE::TSet<TSharedPtr<FNetworkObjectInfo>> ObjectsDormantOnAllConnections;

    UE::TMap<TWeakObjectPtr<UObject>, int32_t> NumDormantObjectsPerConnection;
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
    
    FActorPriority(UNetConnection* InConnection, UActorChannel* InChannel, FNetworkObjectInfo* InActorInfo, const TArray<FNetViewer>& Viewers, bool bLowBandwidth);
    
    FActorPriority(UNetConnection* InConnection, FActorDestructionInfo * DestructInfo, const TArray<struct FNetViewer>& Viewers );
};

struct FCompareFActorPriority
{
    FORCEINLINE bool operator()( const FActorPriority& A, const FActorPriority& B ) const
    {
        return B.Priority < A.Priority;
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

// IsLevelInitializedForActor
inline bool (*IsLevelInitializedForActor)(UNetDriver*, AActor*, UNetConnection*);

// IsPendingKillPending
inline bool (*IsPendingKillPending)(AActor*);

// Close a UChannel
inline void (*Close)(UChannel*);

// Is the actor relevant to the view target?
inline bool (*IsNetRelevantFor)(AActor* Actor, AActor* RealViewer, AActor* ViewTarget, FVector& SrcLocation);

// IsNetReady
inline int (*IsNetReady)(UNetConnection*, bool);

// Construct netviewer
inline FNetViewer* (*FNetViewer__FNetViewer)(FNetViewer*, UNetConnection*, float);

// Remove from consider list
inline void (*Remove)(FNetworkObjectList*, AActor*);

// Construct an FOutBunch
inline char* (*FOutBunch__FOutBunch)(char*, UChannel* InChannel, bool bInClose);

// WriteObject
inline bool (*WriteObject)(UPackageMap*, char*, UObject*, FNetworkGUID, FString*);

// SendBunch
inline void* (*SendBunch)(UChannel*, uintptr_t*, char*, bool);

// Get an actor's dormancy state
inline bool (*GetNetDormancy)(AActor* Actor, FVector& ViewPos, FVector& ViewDir, AActor* Viewer, AActor* ViewTarget, UActorChannel* InChannel, float Time, bool bLowBandwidth);

// Start dormancy on channel
inline void (*StartBecomingDormant)(UActorChannel* ActorChannel);

// SetChannelActorForDestroy (The function isn't stripped after 2.5)
inline void (*SetChannelActorForDestroy)(UChannel*, struct FActorDestructionInfo*);