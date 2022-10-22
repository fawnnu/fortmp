#pragma once

struct FFastArraySerializer;

// Marks an array for replication
inline void (*FastArraySerializer_MarkArrayDirty)(FFastArraySerializer& FastArraySerializer);

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
    char UnknownData_0[0xb0];
    
    void MarkItemDirty(FFastArraySerializerItem& Item)
    {
        int* IDCounter = reinterpret_cast<int*>(__int64(this) + 0x50);
        
        if (Item.ReplicationID == -1)
        {
            Item.ReplicationID = ++(*IDCounter);

            if ((*IDCounter) == -1)
                (*IDCounter)++;
        }

        Item.ReplicationKey++;
        
        // Mark array for replication
        MarkArrayDirty();
    }
    
    void MarkArrayDirty()
    {
        // Mark array as dirty using native Engine function to avoid dealing with struct 
        FastArraySerializer_MarkArrayDirty(*this);

        int* ArrayReplicationKey = reinterpret_cast<int*>(__int64(this) + 0x54);
        int* CachedNumItems = reinterpret_cast<int*>(__int64(this) + 0xA4);
        
        // Mark CachedNumItems as INDEX_NONE
        (*CachedNumItems) = -1;

        {
            (*ArrayReplicationKey)++;
        
            if ((*ArrayReplicationKey) == -1)
            {
                (*ArrayReplicationKey)++;
            }
        }
    }
};

struct FUniqueNetIdRepl
{
    char unk[0x28];
};

struct FNetworkGUID
{
    uint32_t Value;

    bool operator==(FNetworkGUID cmp)
    {
        return Value == cmp.Value;
    }
};