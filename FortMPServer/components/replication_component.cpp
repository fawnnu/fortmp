#include "include.hpp"
#include "replication_component.hpp"

replication_component::replication_component()
{
    INIT_POINTER(ReplicationGraph_ServerReplicateActors, memory_manager::find_function_by_string(L"No Replication Lists were returned for connection"));
    
    INIT_POINTER(SendClientAdjustment, memory_manager::find_pattern("40 53 48 83 EC 20 48 8B 99 ? ? ? ? 48 39 99 ? ? ? ? 74 0A 48 83 B9 ? ? ? ? ?"));
    INIT_POINTER(CallPreReplication, memory_manager::find_pattern("E8 ? ? ? ? 48 8B 53 78 45 33 C9 4C 8B 83 ? ? ? ? 49 8B CE 48 8B 12 E8 ? ? ? ?", true, 1));
    INIT_POINTER(CallPreReplication, memory_manager::find_pattern("49 8B ? ? 8B ? 68 E8 ? ? ? ? ? 8B ? E8", true, 8));
    INIT_POINTER(SetChannelActor, memory_manager::find_function_by_string(L"UActorChannel::SetChannelActor: actor %s"));
    INIT_POINTER(SetChannelActor, memory_manager::find_pattern("4C 8B DC 55 53 57 41 54 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 45 33 E4"));
    INIT_POINTER(CreateChannel, memory_manager::find_function_by_string(L"AsyncLoadWorldOverride=", {0xe8}, 1));
    INIT_POINTER(ReplicateActor, memory_manager::find_function_by_string(L"ReplicateActor: bPausedUntilReliableACK"));
    INIT_POINTER(IsLevelInitializedForActor, memory_manager::find_pattern("48 89 5C 24 ? 57 48 83 EC 20 48 8B 81 ? ? ? ? 49 8B D8 48 8B FA"));
    INIT_POINTER(IsPendingKillPending, memory_manager::find_function_by_string(L"AFortAIController::Possess is missing a", {0xe8}, 1));
    INIT_POINTER(IsPendingKillPending, memory_manager::find_pattern("E8 ? ? ? ? 84 C0 75 17 48 8B 07 48 8B CF 48 8B 74 24 ? 48 83 C4 50", true, 1));
    INIT_POINTER(Close, memory_manager::find_function_by_string(L"UActorChannel::Close: ChIndex: %d, Actor: %s"));
    INIT_POINTER(FNetViewer__FNetViewer, memory_manager::find_pattern("E8 ? ? ? ? 49 8B 07 4C 8D 8C 24 ? ? ? ? 4C 8B 84 24 ? ? ? ? 49 8B CF", true, 1));
    INIT_POINTER(FNetViewer__FNetViewer, memory_manager::find_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 40 48 89 11 48 8B D9 48 8B 42 30 48 85 C0"));
    INIT_POINTER(Remove, memory_manager::find_pattern("48 85 D2 0F 84 ? ? ? ? 55 56 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 4C 8B F2 4C 8B C2 48 8D 55 6F"));
    INIT_POINTER(FOutBunch__FOutBunch, memory_manager::find_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B DA 41 0F B6 F8 48 8B 52 28 48 8B F1 8B 82 ? ? ? ?"));
    INIT_POINTER(WriteObject, memory_manager::find_function_by_string(L"WroteObject %s NetGUID <%s>"));
    INIT_POINTER(SendBunch, memory_manager::find_function_by_string(L"SendBunch: Reliable"));
    INIT_POINTER(StartBecomingDormant, memory_manager::find_function_by_string(L"StartBecomingDormant: %s"));
    INIT_POINTER(::SetChannelActorForDestroy, memory_manager::find_function_by_string(L"SetChannelActorForDestroy: Channel %d. NetGUID <%s>"))

    // make sure the address is valid to prevent crashes
    if (const uintptr_t is_net_ready_address = memory_manager::find_pattern("33 D2 ? 8B ? FF 90 ? ? ? ? 85 C0 0F 84 ? ? ? ? ? 8B"))
    {
        INIT_POINTER(IsNetReady, FindObject(L"/Script/OnlineSubsystemUtils.Default__IpConnection")->VTable[*reinterpret_cast<int*>(is_net_ready_address + 7) / 8]);
    }

    if (const uintptr_t replication_frame_address = memory_manager::find_pattern("8B ? ? 8B ? 68 E8 ? ? ? ? ? 8B ? E8 ? ? ? ?"))
    {
        NetworkOffsets::ReplicationFrame = *reinterpret_cast<int*>(replication_frame_address - 5);
    }
    if (const uintptr_t replication_frame_address = memory_manager::find_pattern("41 FF 84 24 ? ? ? ? 48 8B CE E8 ? ? ? ?"))
    {
        NetworkOffsets::ReplicationFrame = *reinterpret_cast<int*>(replication_frame_address + 4);
    }
    if (const uintptr_t network_objects_address = memory_manager::find_pattern("48 8B 8B ? ? ? ? 33 C0 48 89 83 ? ? ? ? 48 89 83 ? ? ? ? 48 89 83 ? ? ? ? E8 ? ? ? ? 48 85 FF"))
    {
        NetworkOffsets::NetworkObjects = *reinterpret_cast<int*>(network_objects_address + 3);
    }
    if (const uintptr_t destroyed_startup_or_dormant_actors_address = memory_manager::find_pattern("48 8D ? ? ? ? ? 45 33 C9 48 8D 55 ? 4C 8B E8 4C 8D 40 1C E8 ? ? ? ? 48 8B 55 6F 48 8D 4D ? E8 ? ? ? ?"))
    {
        NetworkOffsets::DestroyedStartupOrDormantActors = *reinterpret_cast<int*>(destroyed_startup_or_dormant_actors_address + 3);
    }
    if (const uintptr_t destroyed_startup_or_dormant_actors_netdriver_address = memory_manager::find_pattern("48 ? ? ? ? ? ? 8B 43 08 3B 43 34 74 69 8B 43 48 4C 8D 4B 38"))
    {
        NetworkOffsets::DestroyedStartupOrDormantActors_NetDriver = *reinterpret_cast<int*>(destroyed_startup_or_dormant_actors_netdriver_address + 3);
    }
    if (const uintptr_t time_seconds_address = memory_manager::find_pattern("8B 88 ? ? ? ? 48 8B 03 89 8B ? ? ? ? 48 8B CB FF 90 ? ? ? ? 80 3D ? ? ? ? ?"))
    {
        NetworkOffsets::TimeSeconds = *reinterpret_cast<int*>(time_seconds_address + 2);
    }
    
    // Retrieve the VTable index of IsNetRelevantFor dynamically
    for (int TableIdx = 0; TableIdx < INT_MAX; TableIdx++)
    {
        static auto ActorTable = FindObject(L"/Script/Engine.Default__Actor")->VTable;

        static uintptr_t IsNetRelevantBaseFunction = memory_manager::find_function_by_string(L"Actor %s / %s has no root component in AActor::IsNetRelevantFor. (Make bAlwaysRelevant=true?)");

        if (auto Function = ActorTable[TableIdx])
        {
            if (reinterpret_cast<uintptr_t>(Function) == IsNetRelevantBaseFunction)
            {
                NetworkOffsets::IsNetRelevantForVTableIdx = TableIdx;
                NetworkOffsets::GetNetOwnerVTableIdx = NetworkOffsets::IsNetRelevantForVTableIdx + 6;
                
                break;
            }
        }
        else
        {
            break;
        }
    }
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
        // Call function by VFTable index due to existing overrides
        if (reinterpret_cast<bool(*)(AActor*, AActor*, AActor*, FVector&)>(Actor->VTable[NetworkOffsets::IsNetRelevantForVTableIdx])(Actor, ConnectionViewers[ViewerIdx].InViewer, ConnectionViewers[ViewerIdx].ViewTarget, ConnectionViewers[ViewerIdx].ViewLocation))
        {
            return true;
        }
    }

    return false;
}

bool replication_component::IsActorDormant(FNetworkObjectInfo* ActorInfo, TWeakObjectPtr<UNetConnection>& Connection)
{
    return ActorInfo->DormantConnections.Contains(Connection);
}

bool replication_component::ShouldActorGoDormant(AActor* Actor, TArray<FNetViewer>& ConnectionViewers, UActorChannel* Channel, float Time, bool bLowNetBandwidth)
{
    if (Actor->NetDormancy() <= ENetDormancy::DORM_Awake || Channel == nullptr || Channel->BitField().bPendingDormancy || Channel->BitField().Dormant)
    {
        // Either shouldn't go dormant, or is already dormant
        return false;
    }

    if (Actor->NetDormancy() == ENetDormancy::DORM_DormantPartial)
    {
        for (int ViewerIdx = 0; ViewerIdx < ConnectionViewers.Num(); ViewerIdx++)
        {
            GetNetDormancy = decltype(GetNetDormancy)(Actor->VTable[0x71]); // Initialize GetNetDormancy from the Actor's VTable
            
            if (!GetNetDormancy(Actor, ConnectionViewers[ViewerIdx].ViewLocation, ConnectionViewers[ViewerIdx].ViewDir, ConnectionViewers[ViewerIdx].InViewer, ConnectionViewers[ViewerIdx].ViewTarget, Channel, Time, bLowNetBandwidth))
            {
                return false;
            }
        }
    }

    return true;
}

UNetConnection* replication_component::IsActorOwnedByAndRelevantToConnection(AActor* Actor, TArray<FNetViewer>& ConnectionViewers, bool& bOutHasNullViewTarget)
{
    AActor* ActorOwner = Actor->GetNetOwner();

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

void replication_component::SetChannelActorForDestroy(UChannel* Channel, FActorDestructionInfo* InDestructionInfo)
{
    // We don't need to know the exact size of the FOutBunch struct, we just need to make sure we allocate enough memory for its constructor
    char CloseBunch[1024];
                
    FOutBunch__FOutBunch(CloseBunch, Channel, true);

    WriteObject(Channel->Connection()->PackageMap(), CloseBunch, InDestructionInfo->ObjOuter.Get(), InDestructionInfo->NetGUID, &InDestructionInfo->PathName);

    uintptr_t SendBunchParameter = _mm_unpacklo_ps({}, {}).m128_u64[0];
                
    SendBunch(Channel, &SendBunchParameter, CloseBunch, false);
}


FActorPriority::FActorPriority(UNetConnection* InConnection, UActorChannel* InChannel, FNetworkObjectInfo* InActorInfo, const TArray<FNetViewer>& Viewers, bool bLowBandwidth)
    : ActorInfo(InActorInfo), Channel(InChannel), DestructionInfo(NULL)
{
    float Time  = Channel ? (InConnection->Driver()->Time() - Channel->LastUpdateTime()) : InConnection->Driver()->SpawnPrioritySeconds();
    // take the highest priority of the viewers on this connection
    Priority = 0;
    for (int32 i = 0; i < Viewers.Num(); i++)
    {
        Priority = FGenericPlatformMath::Max<int32_t>(Priority, FGenericPlatformMath::RoundToInt(65536.0f * ActorInfo->Actor->GetNetPriority(Viewers[i].ViewLocation, Viewers[i].ViewDir, Viewers[i].InViewer, Viewers[i].ViewTarget, InChannel, Time, bLowBandwidth)));
    }
}

FActorPriority::FActorPriority(UNetConnection* InConnection, FActorDestructionInfo* Info, const TArray<FNetViewer>& Viewers)
    : ActorInfo(NULL), Channel(NULL), DestructionInfo(Info)
{
    Priority = 0;

    for (int32 i = 0; i < Viewers.Num(); i++)
    {
        float Time  = InConnection->Driver()->SpawnPrioritySeconds();

        FVector Dir = DestructionInfo->DestroyedPosition - Viewers[i].ViewLocation;
        float DistSq = Dir.SizeSquared();
		
        // adjust priority based on distance and whether actor is in front of viewer
        if ( (Viewers[i].ViewDir | Dir) < 0.f )
        {
            if ( DistSq > NEARSIGHTTHRESHOLDSQUARED )
                Time *= 0.2f;
            else if ( DistSq > CLOSEPROXIMITYSQUARED )
                Time *= 0.4f;
        }
        else if ( DistSq > MEDSIGHTTHRESHOLDSQUARED )
            Time *= 0.4f;

        Priority = FMath::Max<int32>(Priority, 65536.0f * Time);
    }   
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
    
    for (TSharedPtr<FNetworkObjectInfo>& NetworkObjectInfoPtr : NetworkObjects.Object->ActiveNetworkObjects)
    {
        FNetworkObjectInfo* ActorInfo = NetworkObjectInfoPtr.Get();

        if (!ActorInfo->bPendingNetUpdate && (*GWorld)->TimeSeconds() <= ActorInfo->NextUpdateTime)
        {
            // Not time for the actor to update yet
            continue;
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
            ActorInfo->LastNetReplicateTime = (*GWorld)->TimeSeconds();
            ActorInfo->OptimalNetUpdateDelta = 1.0f / Actor->NetUpdateFrequency();
        }

        const float ScaleDownStartTime = 2.0f;
        const float ScaleDownTimeRange = 5.0f;

        const float LastReplicateDelta = (*GWorld)->TimeSeconds() - ActorInfo->LastNetReplicateTime;

        if (LastReplicateDelta > ScaleDownStartTime)
        {
            if (Actor->MinNetUpdateFrequency() == 0.0f)
            {
                Actor->MinNetUpdateFrequency() = 2.0f;
            }

            // Calculate min delta and max delta
            const float MinOptimalDelta = 1.0f / Actor->NetUpdateFrequency();
            const float MaxOptimalDelta = FMath::Max(1.0f / Actor->MinNetUpdateFrequency(), MinOptimalDelta);

            // Interpolate between MinOptimalDelta and MaxOptimalDelta
            const float Alpha = FMath::Clamp((LastReplicateDelta - ScaleDownStartTime) / ScaleDownTimeRange, 0.0f, 1.0f);

            // Update optimal net delta
            ActorInfo->OptimalNetUpdateDelta = MinOptimalDelta + Alpha * (MaxOptimalDelta - MinOptimalDelta);
        }

        if (!ActorInfo->bPendingNetUpdate)
        {
            const float NextUpdateDelta = ActorInfo->OptimalNetUpdateDelta;

            // Set next update time
            ActorInfo->NextUpdateTime = (*GWorld)->TimeSeconds() + SRand() * ServerTickTime + NextUpdateDelta;

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
        Remove(NetworkObjects.Get(), Actor);
    }
    
    ActorsToRemove.clear();
}

int replication_component::ServerReplicateActors_PrioritizeActors(UNetDriver* NetDriver, UNetConnection* Connection, TArray<FNetViewer>& ConnectionViewers, std::vector<FNetworkObjectInfo*> ConsiderList, bool bCPUSaturated, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors)
{
    NetDriver->NetTag()++;

    for ( int32 j = 0; j < Connection->SentTemporaries().Num(); j++ )
    {
        Connection->SentTemporaries()[j]->NetTag() = NetDriver->NetTag();
    }

    int FinalSortedCount = 0;
    int DeletedCount = 0;

    // Make weak ptr once for IsActorDormantCall
    TWeakObjectPtr<UNetConnection> WeakConnection(Connection);

    const bool bLowNetBandwidth = false;
    
    UE::TMap<FNetworkGUID, FActorDestructionInfo>& DestroyedStartupOrDormantActors = *reinterpret_cast<UE::TMap<FNetworkGUID, FActorDestructionInfo>*>(uintptr_t(NetDriver) + NetworkOffsets::DestroyedStartupOrDormantActors_NetDriver);
    
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

                PriorityConnection = IsActorOwnedByAndRelevantToConnection(Actor, ConnectionViewers, bHasNullViewTarget);

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
                // TODO: Fix time, this doesn't work properly
                if (IsActorDormant(ActorInfo, WeakConnection)) //TODO: fix this returning false even when actor is dormant
                {
                    // Skip actor if dormant
                    continue;
                }

                if (ShouldActorGoDormant(Actor, ConnectionViewers, Channel, NetDriver->Time(), bLowNetBandwidth))
                {
                    // Channel is marked to go dormant now once all properties have been replicated
                    StartBecomingDormant(Channel);
                }
            }

            if (Actor->NetTag() != NetDriver->NetTag())
            {
                Actor->NetTag() = NetDriver->NetTag();
                
                OutPriorityList[FinalSortedCount] =  FActorPriority( PriorityConnection, Channel, ActorInfo, ConnectionViewers, bLowNetBandwidth );
                OutPriorityActors[FinalSortedCount] = OutPriorityList + FinalSortedCount;

                FinalSortedCount++;
            }
        }

        // Add destroyed actors
        /*for (FNetworkGUID NetworkGuid : *reinterpret_cast<UE::TSet<FNetworkGUID>*>(uintptr_t(Connection) + NetworkOffsets::DestroyedStartupOrDormantActors))
        {
            FActorDestructionInfo& DInfo = DestroyedStartupOrDormantActors[NetworkGuid];
            
            OutPriorityList[FinalSortedCount] =  FActorPriority( Connection, &DInfo, ConnectionViewers );
            OutPriorityActors[FinalSortedCount] = OutPriorityList + FinalSortedCount;

            FinalSortedCount++;
        }*/

        // Sort by priority
        Sort(OutPriorityActors, FinalSortedCount, FCompareFActorPriority());
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

                // Re-implemented SetChannelActorForDestroy
                SetChannelActorForDestroy(Channel, PriorityActors[j]->DestructionInfo);
                
                reinterpret_cast<UE::TSet<FNetworkGUID>*>(uintptr_t(Connection) + NetworkOffsets::DestroyedStartupOrDormantActors)->Remove(PriorityActors[j]->DestructionInfo->NetGUID);
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
                        ActorInfo->NextUpdateTime = (*GWorld)->TimeSeconds() + 0.2f * SRand();
                    }
                }

                if (Channel)
                {
                    if (bIsRelevant)
                    {
                        // Mark as relevant for a short amount of time
                        Channel->RelevantTime() = NetDriver->Time() + 0.5f * SRand();
                    }

                    if (IsNetReady(Channel->Connection(), 0))
                    {
                        if (ReplicateActor(Channel))
                        {
                            ActorUpdatesThisConnectionSent++;

                            // Calculate min and max delta
                            const float MinOptimalDelta = 1.0f / Actor->NetUpdateFrequency();
                            const float MaxOptimalDelta = std::max(1.0f / Actor->MinNetUpdateFrequency(), MinOptimalDelta);
                            const float DeltaBetweenReplications = (*GWorld)->TimeSeconds() - ActorInfo->LastNetReplicateTime;

                            // Choose an optimal time
                            ActorInfo->OptimalNetUpdateDelta = std::clamp(DeltaBetweenReplications * 0.7f, MinOptimalDelta, MaxOptimalDelta);
                            ActorInfo->LastNetReplicateTime = (*GWorld)->TimeSeconds();
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

    if (STATIC_HASPROPERTY(NetDriver, ReplicationDriver))
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

    // Default Fortnite tick rate for legacy replication
    float ServerTickTime = 20.0f;

    if (ServerTickTime == 0.f)
    {
        ServerTickTime = DeltaSeconds;
    }
    else
    {
        ServerTickTime = 1.f / ServerTickTime;
        bCPUSaturated = DeltaSeconds > 1.2f * ServerTickTime;
    }

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

            // Add connection to viewers
            ConnectionViewers.Add(*FNetViewer__FNetViewer(new FNetViewer(), Connection, DeltaSeconds));

            for (int ViewerIndex = 0; ViewerIndex < Connection->Children().Num(); ViewerIndex++)
            {
                if (Connection->Children()[ViewerIndex]->ViewTarget() != nullptr)
                {
                    // Add child connection to viewers
                    ConnectionViewers.Add(*FNetViewer__FNetViewer(new FNetViewer(), Connection->Children()[ViewerIndex], DeltaSeconds));
                }
            }
            
            if (Connection->PlayerController())
            {
                // Send ClientAdjustment if necessary
                SendClientAdjustment(Connection->PlayerController());
            }

            for ( int32_t ChildIdx = 0; ChildIdx < Connection->Children().Num(); ChildIdx++)
            {
                if (Connection->Children()[ChildIdx]->PlayerController() != nullptr)
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
                else if (IsActorRelevantToConnection(Actor, ConnectionViewers))
                {
                    // Force another update for next frame
                    PriorityActors[k]->ActorInfo->bPendingNetUpdate = true;

                    if (Channel != nullptr)
                    {
                        // Update relevant time
                        Channel->RelevantTime() = NetDriver->Time() + 0.5f * SRand();
                    }
                }
            }

            // Free memory
            delete[] PriorityList;
            delete[] PriorityActors;
            
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
