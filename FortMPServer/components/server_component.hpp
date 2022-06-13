#pragma once

class server_component : public component
{
public:
    server_component();

private:
    //Creates a new netdriver
    bool (*CreateNamedNetDriver)(UEngine* Engine, UWorld* World, FName NetDriverName, FName NetDriverDefinition);

    // finds a created netdriver
    UNetDriver* (*FindNamedNetDriver)(UEngine* Engine, UWorld* InWorld, FName NetDriverName);

    // sets world in netdriver
    void (*SetWorld)(UNetDriver* NetDriver, UWorld* World);

    // starts listening on a netdriver
    bool (*InitListen)(UNetDriver* NetDriver, void* InNotify, struct FURL& LocalURL, bool bReuseAddressAndPort, FString& Error);

    // replicate actors to the client
    int (*ServerReplicateActors)(UReplicationDriver* ReplicationDriver, float DeltaSeconds);
};
