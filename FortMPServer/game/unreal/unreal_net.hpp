#pragma once

struct FFastArraySerializerItem
{
    int ReplicationID;
    int ReplicationKey;
    int MostRecentArrayReplicationKey;

    FFastArraySerializerItem() :
        ReplicationID(-1), ReplicationKey(-1), MostRecentArrayReplicationKey(-1)
    {
    }
};

struct FFastArraySerializer
{
    char UnknownData_0[0xB0];
};