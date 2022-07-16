#include "include.hpp"
#include "replication_component.hpp"

#include <math.h>

replication_component::replication_component()
{
    INIT_POINTER(ReplicationGraph_ServerReplicateActors, memory_manager::find_function_by_string(L"No Replication Lists were returned for connection"));
    INIT_POINTER(SendClientAdjustment, memory_manager::find_pattern("40 53 48 83 EC 20 48 8B 99 ? ? ? ? 48 39 99 ? ? ? ? 74 0A 48 83 B9 ? ? ? ? ?"));
    INIT_POINTER(CallPreReplication, memory_manager::find_pattern("E8 ? ? ? ? 49 8B CD E8 ? ? ? ? 41 83 7D ? ? 0F 84 ? ? ? ?", true, 1));
    INIT_POINTER(SetChannelActor, memory_manager::find_function_by_string(L"UActorChannel::SetChannelActor: actor %s"));
    INIT_POINTER(CreateChannel, memory_manager::find_function_by_string(L"AsyncLoadWorldOverride=", {0xe8}, 1));
    INIT_POINTER(ReplicateActor, memory_manager::find_pattern("E8 ? ? ? ? 41 83 7D ? ? 0F 84 ? ? ? ? 45 33 C0", true, 1));
    INIT_POINTER(IsLevelInitializedForActor, FindObject(L"/Script/OnlineSubsystemUtils.Default__IpNetDriver")->VTable[*reinterpret_cast<int*>(memory_manager::find_pattern("49 8B ? 4C 8B ? 49 8B ? 49 8B ? FF 90 ? ? ? ? 84 C0 0F 84 ? ? ? ?") + 14) / 8]);
    INIT_POINTER(IsPendingKillPending, memory_manager::find_pattern("E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 49 8B ? 4C 8B ? 49 8B ? 49 8B ? FF 90 ? ? ? ?", true, 1));
    INIT_POINTER(Close, memory_manager::find_pattern("48 8B C4 53 48 83 EC 70 48 83 79 ? ? 48 8B D9 0F 84 ? ? ? ? 83 B9 ? ? ? ? ? 0F 84 ? ? ? ? 80 3D ? ? ? ? ? 0F 82 ? ? ? ? 48 89 68 18 48 8D 48 10"));
    INIT_POINTER(SetChannelActorForDestroy, memory_manager::find_function_by_string(L"SetChannelActorForDestroy: Channel %d. NetGUID <%s> Path: %s. Bits: %d"));
    INIT_POINTER(IsNetRelevantFor, memory_manager::find_function_by_string(L"Actor %s / %s has no root component in AActor::IsNetRelevantFor. (Make bAlwaysRelevant=true?)"));
    INIT_POINTER(IsNetReady, FindObject(L"/Script/OnlineSubsystemUtils.Default__IpConnection")->VTable[*reinterpret_cast<int*>(memory_manager::find_pattern("FF 90 ? ? ? ? 85 C0 0F 84 ? ? ? ? 48 8B ? 48 8B ? FF 90 ? ? ? ? 48 85 C0") + 2) / 8]);
    INIT_POINTER(FNetViewer__FNetViewer, memory_manager::find_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 40 48 89 11 48 8B D9 48 8B 42 30 48 85 C0"));
    
    // TODO: Change per version
    NetworkOffsets::ReplicationFrame = 0x2C8;
    NetworkOffsets::NetworkObjects = 0x490;
}

UChannel* replication_component::FindChannelByActor(UNetConnection* Connection, AActor* InActor)
{
    static UObject* ActorChannelClass = FindObject(L"/Script/Engine.ActorChannel");
    
    for (UChannel* OpenChannel : Connection->OpenChannels()) // check all open actor channels
    { 
        if (OpenChannel != nullptr) // make sure it's valid
        {
            if (OpenChannel->Class == ActorChannelClass) // make sure it's an UActorChannel
            {
                if (static_cast<UActorChannel*>(OpenChannel)->Actor() == InActor)
                {
                    return OpenChannel;
                }
            }
        }
    }

    return nullptr;
}

bool replication_component::IsActorRelevantToConnection(AActor* Actor, TArray<FNetViewer>& ConnectionViewers)
{
    for (int ViewerIdx = 0; ViewerIdx < ConnectionViewers.Num(); ViewerIdx++)
    {
        if (IsNetRelevantFor(Actor, ConnectionViewers[ViewerIdx].InViewer, ConnectionViewers[ViewerIdx].ViewTarget, ConnectionViewers[ViewerIdx].ViewLocation))
        {
            return true;
        }
    }

    return false;
}

UNetConnection* replication_component::IsActorOwnedByAndRelevantToConnection(AActor* Actor, TArray<FNetViewer>& ConnectionViewers, bool& bOutHasNullViewTarget)
{
    AActor* ActorOwner = Actor->Owner();

    bOutHasNullViewTarget = false;

    for (int i = 0; i < ConnectionViewers.Num(); i++)
    {
        UNetConnection* ViewerConnection = ConnectionViewers[i].Connection;

        if (ViewerConnection->ViewTarget() == nullptr)
        {
            bOutHasNullViewTarget = true;
        }

        if (ActorOwner == ViewerConnection->PlayerController() ||
            (ViewerConnection->PlayerController() && ActorOwner == ViewerConnection->PlayerController()->Pawn()) ||
            (ViewerConnection->ViewTarget() && ActorOwner == ViewerConnection->ViewTarget()) )
        {
            return ViewerConnection;
        }
    }

    return nullptr;
}

int replication_component::ServerReplicateActors_PrepConnections(UNetDriver* NetDriver, float DeltaSeconds)
{
    int NumClientsToTick = NetDriver->ClientConnections().Num();

    // Decides whether any connections are ready to tick yet
    bool bFoundReadyConnection = false;

    for (int ConnIdx = 0; ConnIdx < NetDriver->ClientConnections().Num(); ConnIdx++)
    {
        UNetConnection* Connection = NetDriver->ClientConnections()[ConnIdx];

        if (Connection == nullptr)
        {
            // Do not handle any connections that are invalid
            continue;
        }

        // TODO: Connection->State == USOCK_Pending || Connection->State == USOCK_Open || Connection->State == USOCK_Closed

        if (Connection->OwningActor() != nullptr && ( Connection->Driver()->Time() - Connection->LastReceiveTime() < 1.5f ) )
        {
            // TODO: World == OwningActor->GetWorld()

            bFoundReadyConnection = true;

            // View target is what the player controller is looking at
            AActor* DesiredViewTarget = Connection->OwningActor();

            if (Connection->PlayerController())
            {
                if (AActor* ViewTarget = Connection->PlayerController()->GetViewTarget())
                {
                    if (ViewTarget == nullptr)
                    {
                        // Do not use an invalid view target
                        continue;
                    }

                    // Safe to use the player controller's view target
                    DesiredViewTarget = ViewTarget;
                }
            }

            Connection->ViewTarget() = DesiredViewTarget;

            for (int ChildIdx = 0; ChildIdx < Connection->Children().Num(); ChildIdx++)
            {
                UNetConnection* Child = Connection->Children()[ChildIdx];
                APlayerController* ChildPlayerController = Child->PlayerController();

                if (ChildPlayerController != nullptr)
                {
                    // Update child view target
                    Child->ViewTarget() = ChildPlayerController->GetViewTarget();
                }
                else
                {
                    Child->ViewTarget() = nullptr;
                }
            }
        }
        else
        {
            Connection->ViewTarget() = nullptr;

            for (int ChildIdx = 0; ChildIdx < Connection->Children().Num(); ChildIdx++)
            {
                // Reset child view target
                Connection->Children()[ChildIdx]->ViewTarget() = nullptr;
            }
        }
    }
    
    return bFoundReadyConnection ? NumClientsToTick : 0;
}

void replication_component::ServerReplicateActors_BuildConsiderList(UNetDriver* NetDriver, std::vector<FNetworkObjectInfo*>& OutConsiderList, float ServerTickTime)
{
    // List of actors to remove after building consider list
    std::vector<AActor*> ActorsToRemove;

    // List of all network objects
    TSharedPtr<FNetworkObjectList>& NetworkObjects = *reinterpret_cast<TSharedPtr<FNetworkObjectList>*>(__int64(NetDriver) + NetworkOffsets::NetworkObjects);
    
    for (TSetElement<TSharedPtr<FNetworkObjectInfo>>& NetworkObjectInfoPtr : NetworkObjects.Object->ActiveNetworkObjects)
    {
        FNetworkObjectInfo* ActorInfo = NetworkObjectInfoPtr.Element.Get();
        
        if (!ActorInfo->bPendingNetUpdate && (*GWorld)->GameState()->ReplicatedWorldTimeSeconds() <= ActorInfo->NextUpdateTime)
        {
            // Not time for the actor to update yet
            //continue;
        }

        AActor* Actor = ActorInfo->Actor;

        // TODO: Actor->IsActorInitialized()
        // TODO: Level->HasVisibilityChangeRequestPending()

        if (IsPendingKillPending(Actor))
        {
            // Remove actors if pending kill pending
            ActorsToRemove.push_back(Actor);

            continue;
        }
        
        if (Actor->RemoteRole() == ENetRole::ROLE_None)
        {
            // Remove actors with no remote role
            ActorsToRemove.push_back(Actor);

            continue;
        }

        if (Actor->NetDriverName() != NetDriver->NetDriverName())
        {
            // Remove actor that belongs to a different net driver
            ActorsToRemove.push_back(Actor);

            continue;
        }

        if (Actor->NetDormancy() == ENetDormancy::DORM_Initial && Actor->bNetStartup())
        {
            // Remove actor if initially dormant
            ActorsToRemove.push_back(Actor);

            continue;
        }

        if (ActorInfo->LastNetReplicateTime == 0)
        {
            // Set defaults if this actor is replicating for the first time
            ActorInfo->LastNetReplicateTime = (*GWorld)->GameState()->ReplicatedWorldTimeSeconds();
            ActorInfo->OptimalNetUpdateDelta = 1.0f / Actor->NetUpdateFrequency();
        }

        const float ScaleDownStartTime = 2.0f;
        const float ScaleDownTimeRange = 5.0f;

        const float LastReplicateDelta = (*GWorld)->GameState()->ReplicatedWorldTimeSeconds() - ActorInfo->LastNetReplicateTime;

        if (LastReplicateDelta > ScaleDownStartTime)
        {
            if (Actor->MinNetUpdateFrequency() == 0.0f)
            {
                Actor->MinNetUpdateFrequency() = 2.0f;
            }

            // Calculate min delta and max delta
            const float MinOptimalDelta = 1.0f / Actor->NetUpdateFrequency();
            const float MaxOptimalDelta = std::max(1.0f / Actor->MinNetUpdateFrequency(), MinOptimalDelta);

            // Interpolate between MinOptimalDelta and MaxOptimalDelta
            const float Alpha = std::clamp((LastReplicateDelta - ScaleDownStartTime) / ScaleDownTimeRange, 0.0f, 1.0f);

            // Update optimal net delta
            ActorInfo->OptimalNetUpdateDelta = MinOptimalDelta + Alpha * (MaxOptimalDelta - MinOptimalDelta);
        }

        if (!ActorInfo->bPendingNetUpdate)
        {
            const float NextUpdateDelta = ActorInfo->OptimalNetUpdateDelta;

            // Set next update time
            ActorInfo->NextUpdateTime = (*GWorld)->GameState()->ReplicatedWorldTimeSeconds() + rand() * ServerTickTime + NextUpdateDelta;

            // Mark when the actor first requested an update
            ActorInfo->LastNetUpdateTime = NetDriver->Time();
        }

        // Clear pending update flag
        ActorInfo->bPendingNetUpdate = false;

        // Add to consider list
        OutConsiderList.push_back(ActorInfo);

        CallPreReplication(ActorInfo->Actor, NetDriver);
    }

    for (AActor* Actor : ActorsToRemove)
    {
        // TODO: Remove from NetworkObjectList
    }
    
    ActorsToRemove.clear();
}

int replication_component::ServerReplicateActors_PrioritizeActors(UNetDriver* NetDriver, UNetConnection* Connection, TArray<FNetViewer>& ConnectionViewers, std::vector<FNetworkObjectInfo*> ConsiderList, bool bCPUSaturated, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors)
{
    // TODO: NetTag++

    int FinalSortedCount = 0;
    int DeletedCount = 0;

    const int MaxSortedActors = ConsiderList.size();

    if (MaxSortedActors > 0)
    {
        OutPriorityList = new FActorPriority[MaxSortedActors];
        OutPriorityActors = new FActorPriority*[MaxSortedActors];

        for (FNetworkObjectInfo* ActorInfo : ConsiderList)
        {
            AActor* Actor = ActorInfo->Actor;
            UActorChannel* Channel = reinterpret_cast<UActorChannel*>(FindChannelByActor(Connection, Actor));

            if (Channel == nullptr)
            {
                if (!IsLevelInitializedForActor(NetDriver, Actor, Connection))
                {
                    // If the level is not loaded on client, skip
                    continue;
                }

                if (!IsActorRelevantToConnection(Actor, ConnectionViewers))
                {
                    // If not relevant, skip
                    continue;
                }
            }

            UNetConnection* PriorityConnection = Connection;

            if (Actor->bOnlyRelevantToOwner())
            {
                // This actor should be owned by a particular connection
                bool bHasNullViewTarget = false;

                //PriorityConnection = IsActorOwnedByAndRelevantToConnection(Actor, ConnectionViewers, bHasNullViewTarget);

                if (PriorityConnection == nullptr)
                {
                    if (!bHasNullViewTarget && Channel != nullptr && NetDriver->Time() - Channel->RelevantTime() >= NetDriver->RelevantTimeout())
                    {
                        // Close channel if not owned by connection 
                        Close(Channel);
                    }

                    // This connection doesn't own this actor
                    continue;
                }
            }
            else
            {
                // TODO: IsActorDormant
                // TODO: ShouldActorGoDormant
            }

            // TODO: UE uses a check for NetTag here

            {
                OutPriorityList[FinalSortedCount] = FActorPriority(ActorInfo, Channel);
                OutPriorityActors[FinalSortedCount] = OutPriorityList + FinalSortedCount;

                FinalSortedCount++;
            }
        }

        // TODO: Sort by priority
    }
    
    return FinalSortedCount;
}

int replication_component::ServerReplicateActors_ProcessPrioritizedActors(UNetDriver* NetDriver, UNetConnection* Connection, TArray<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, int FinalSortedCount, int& OutUpdated)
{
    int ActorUpdatesThisConnection = 0;
    int ActorUpdatesThisConnectionSent = 0;
    int FinalRelevantCount = 0;

    if (!IsNetReady(Connection, 0))
    {
        // Connection saturated, don't process any actors
        return 0;
    }

    for (int j = 0; j < FinalSortedCount; j++)
    {
        FNetworkObjectInfo* ActorInfo = PriorityActors[j]->ActorInfo;

        if (ActorInfo == nullptr && PriorityActors[j]->DestructionInfo)
        {
            // TODO: Make sure client has streaming level loaded
            
            UActorChannel* Channel = reinterpret_cast<UActorChannel*>(CreateChannel(Connection, 2, true, -1));

            if (Channel)
            {
                FinalRelevantCount++;

                // Mark actor channel for destruction
                SetChannelActorForDestroy(Channel, PriorityActors[j]->DestructionInfo);
                
                // TODO: Connection->DestroyStartupOrDormantActors.Remove
            }

            continue;
        }

        UActorChannel* Channel = PriorityActors[j]->Channel;

        if (Channel == nullptr || Channel->Actor())
        {
            AActor* Actor = ActorInfo->Actor;

            bool bIsRelevant = false;
            
            const bool bLevelInitializedForActor = IsLevelInitializedForActor(NetDriver, Actor, Connection);

            if (bLevelInitializedForActor)
            {
                if (!Actor->bTearOff() && (Channel == nullptr || NetDriver->Time() - Channel->RelevantTime() > 1.f))
                {
                    if (IsActorRelevantToConnection(Actor, ConnectionViewers))
                    {
                        bIsRelevant = true;
                    }
                }
            }

            const bool bIsRecentlyRelevant = bIsRelevant || (Channel && NetDriver->Time() - Channel->RelevantTime() < NetDriver->RelevantTimeout()) || ActorInfo->bForceRelevantNextUpdate;

            // Do not mark as relevant for next update
            ActorInfo->bForceRelevantNextUpdate = false;

            if (bIsRecentlyRelevant)
            {
                FinalRelevantCount++;

                if (Channel == nullptr) // TODO: && SupportsObject
                {
                    if (bLevelInitializedForActor)
                    {
                        // Create a new channel for this actor
                        Channel = reinterpret_cast<UActorChannel*>(CreateChannel(Connection, 2, true, -1));

                        if (Channel)
                        {
                            SetChannelActor(Channel, Actor);
                        }
                    }
                    else if (Actor->NetUpdateFrequency() < 1.0f)
                    {
                        ActorInfo->NextUpdateTime = (*GWorld)->GameState()->ReplicatedWorldTimeSeconds() + 0.2f * rand();
                    }
                }

                if (Channel)
                {
                    if (bIsRelevant)
                    {
                        // Mark as relevant for a short amount of time
                        Channel->RelevantTime() = NetDriver->Time() + 0.5f * rand();
                    }

                    if (IsNetReady(Channel->Connection(), 0))
                    {
                        if (ReplicateActor(Channel))
                        {
                            ActorUpdatesThisConnectionSent++;

                            // Calculate min and max delta
                            const float MinOptimalDelta = 1.0f / Actor->NetUpdateFrequency();
                            const float MaxOptimalDelta = std::max(1.0f / Actor->MinNetUpdateFrequency(), MinOptimalDelta);
                            const float DeltaBetweenReplications = (*GWorld)->GameState()->ReplicatedWorldTimeSeconds() - ActorInfo->LastNetReplicateTime;

                            // Choose an optimal time
                            ActorInfo->OptimalNetUpdateDelta = std::clamp(DeltaBetweenReplications * 0.7f, MinOptimalDelta, MaxOptimalDelta);
                            ActorInfo->LastNetReplicateTime = (*GWorld)->GameState()->ReplicatedWorldTimeSeconds();
                        }

                        ActorUpdatesThisConnection++;
                        OutUpdated++;
                    }
                    else
                    {
                        // Force net update
                        Actor->ForceNetUpdate();
                    }

                    if (!IsNetReady(Connection, 0))
                    {
                        // Stop now as connection is already saturated
                        return j;
                    }
                }
            }

            if ( (!bIsRecentlyRelevant || Actor->bTearOff()) && Channel != nullptr)
            {
                if (!bLevelInitializedForActor || !Actor->bNetStartup())
                {
                    // Close channel for an actor that is no longer relevant
                    Close(Channel);
                }
            }
        }
    }

    return FinalSortedCount;
}

int replication_component::ServerReplicateActors(UNetDriver* NetDriver, float DeltaSeconds)
{
    if (NetDriver->ClientConnections().Num() == 0)
    {
        return 0;
    }

    static bool has_replication_driver = NetDriver->HasProperty(L"ReplicationDriver");

    if (has_replication_driver)
    {
        if (NetDriver->ReplicationDriver())
        {
            // Handle replication with ReplicationGraph if it exists
            return ReplicationGraph_ServerReplicateActors(NetDriver->ReplicationDriver(), DeltaSeconds);
        }
    }

    uint32_t& ReplicationFrame = *reinterpret_cast<uint32_t*>(__int64(NetDriver) + NetworkOffsets::ReplicationFrame);
    
    // Bump the ReplicationFrame value to invalidate any properties marked as "unchanged" for this frame
    ReplicationFrame++;

    int Updated = 0;

    // Amount of clients to tick for this frame
    const int NumClientsToTick = ServerReplicateActors_PrepConnections(NetDriver, DeltaSeconds);

    if (NumClientsToTick == 0)
    {
        // No connections are ready this frame
        return 0;
    }

    AWorldSettings* WorldSettings = (*GWorld)->PersistentLevel()->WorldSettings();

    static auto net_viewer_property = FindPropertyByName(WorldSettings->Class, L"ReplicationViewers");

    bool bCPUSaturated = false;
    
    // Legacy builds should only run at 20 ticks per second
    static float ServerTickTime = 1.f / 20.0f;
    
    std::vector<FNetworkObjectInfo*> ConsiderList;

    // Build the actor consider list
    ServerReplicateActors_BuildConsiderList(NetDriver, ConsiderList, ServerTickTime);
    
    for (int i = 0; i < NetDriver->ClientConnections().Num(); i++)
    {
        UNetConnection* Connection = NetDriver->ClientConnections()[i];

        if (Connection == nullptr)
        {
            // Do not handle any connections that are invalid
            continue;
        }

        if (i >= NumClientsToTick)
        {   
            for (int ConsiderIdx = 0; ConsiderIdx < ConsiderList.size(); ConsiderIdx++)
            {
                AActor* Actor = ConsiderList[ConsiderIdx]->Actor;

                if (Actor != nullptr && !ConsiderList[ConsiderIdx]->bPendingNetUpdate)
                {
                    UActorChannel* Channel = reinterpret_cast<UActorChannel*>(FindChannelByActor(Connection, Actor));

                    if (Channel != nullptr && Channel->LastUpdateTime() < ConsiderList[ConsiderIdx]->LastNetUpdateTime)
                    {
                        // Flag for a pending update
                        ConsiderList[ConsiderIdx]->bPendingNetUpdate = true;
                    }
                }
            }

            // TODO: Connection->TimeSensitive
        }
        else if (Connection->ViewTarget())
        {
            // List of viewers that the connection should consider
            TArray<FNetViewer>& ConnectionViewers = WorldSettings->ReplicationViewers();

            for (int j = 0; j < ConnectionViewers.Num(); j++)
            {
                GenericArray_Remove(&ConnectionViewers, net_viewer_property, j);
            }
            
            ConnectionViewers.Add(*FNetViewer__FNetViewer(new FNetViewer(), Connection, DeltaSeconds));

            for (int ViewerIndex = 0; ViewerIndex < Connection->Children().Num(); ViewerIndex++)
            {
                if (Connection->Children()[ViewerIndex]->ViewTarget() != nullptr)
                {
                    // TODO: new FNetViewer()
                }
            }
            
            if (Connection->PlayerController())
            {
                // Send ClientAdjustment if necessary
                SendClientAdjustment(Connection->PlayerController());
            }

            for ( int32_t ChildIdx = 0; ChildIdx < Connection->Children().Num(); ChildIdx++)
            {
                if ( Connection->Children()[ChildIdx]->PlayerController() != NULL )
                {
                    // Send ClientAdjustment if necessary to any child player controllers
                    SendClientAdjustment(Connection->Children()[ChildIdx]->PlayerController());
                }
            }

            FActorPriority* PriorityList = nullptr;
            FActorPriority** PriorityActors = nullptr;

            // Get a sorted list of actors for this connection
            const int FinalSortedCount = ServerReplicateActors_PrioritizeActors(NetDriver, Connection, ConnectionViewers, ConsiderList, bCPUSaturated, PriorityList, PriorityActors);

            // Process the sorted list of actors for this connection
            const int LastProcessedActor = ServerReplicateActors_ProcessPrioritizedActors(NetDriver, Connection, ConnectionViewers, PriorityActors, FinalSortedCount, Updated);

            for (int k = LastProcessedActor; k < FinalSortedCount; k++)
            {
                if (!PriorityActors[k]->ActorInfo)
                {
                    // A deletion entry so skip
                    continue;
                }

                // Get actor and channel from priority list
                AActor* Actor = PriorityActors[k]->ActorInfo->Actor;
                UActorChannel* Channel = PriorityActors[k]->Channel;

                if (Channel != nullptr && NetDriver->Time() - Channel->RelevantTime() <= 1.f)
                {
                    // Flag for net update
                    PriorityActors[k]->ActorInfo->bPendingNetUpdate = true;
                }

                // TODO: IsActorRelevantToConnection

                
            }
            
            for (int j = 0; j < ConnectionViewers.Num(); j++)
            {
                GenericArray_Remove(&ConnectionViewers, net_viewer_property, j);
            }
        }
    }
    
    // Clear consider list
    ConsiderList.clear();
    
    return Updated;
}
