#include "include.hpp"
#include "replication_component.hpp"

replication_component::replication_component()
{
    INIT_POINTER(ReplicationGraph_ServerReplicateActors, memory_manager::find_function_by_string(L"No Replication Lists were returned for connection"));
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
    int NumInitiallyDormant = 0;

    // List of actors to remove after building consider list
    std::vector<AActor*> ActorsToRemove;
}

int replication_component::ServerReplicateActors(UNetDriver* NetDriver, float DeltaSeconds)
{
    if (NetDriver->ClientConnections().Num() == 0)
    {
        return 0;
    }

    if (NetDriver->ReplicationDriver())
    {
        // Handle replication with ReplicationGraph if it exists
        return ReplicationGraph_ServerReplicateActors(NetDriver->ReplicationDriver(), DeltaSeconds);
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

    // Legacy builds should only run at 20 ticks per second
    float ServerTickTime = 1.f / 20.0f;

    // TODO: ServerReplicateActors_BuildConsiderList

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
            // TODO: Add
        }
        else if (Connection->ViewTarget())
        {
            // TODO: Add
        }
    }
    
    return Updated;
}
