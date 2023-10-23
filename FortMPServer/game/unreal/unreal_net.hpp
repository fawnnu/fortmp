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
    char unk[0x18];

    TArray<unsigned char> ReplicationBytes;
};

struct FNetworkGUID
{
    uint32_t Value;

    bool operator==(FNetworkGUID cmp)
    {
        return Value == cmp.Value;
    }
};

struct FReplicationFlags
{
    union
    {
        struct
        {
            /** True if replicating actor is owned by the player controller on the target machine. */
            uint32_t bNetOwner:1;
            /** True if this is the initial network update for the replicating actor. */
            uint32_t bNetInitial:1;
            /** True if this is actor is RemoteRole simulated. */
            uint32_t bNetSimulated:1;
            /** True if this is actor's ReplicatedMovement.bRepPhysics flag is true. */
            uint32_t bRepPhysics:1;
            /** True if this actor is replicating on a replay connection. */
            uint32_t bReplay:1;
            /** True if this actor's RPCs should be ignored. */
            uint32_t bIgnoreRPCs:1;
        };

        uint32_t	Value;
    };
    FReplicationFlags()
    {
        Value = 0;
    }
};

struct FRepRecord
{
    class UProperty* Property;
    int32_t Index;
    FRepRecord(UProperty* InProperty,int32_t InIndex)
    : Property(InProperty), Index(InIndex)
    {}
};

enum ELifetimeCondition
{
	COND_None = 0,
	COND_InitialOnly = 1,
	COND_OwnerOnly = 2,				
	COND_SkipOwner = 3,				
	COND_SimulatedOnly = 4,			
	COND_AutonomousOnly = 5,			
	COND_SimulatedOrPhysics = 6,		
	COND_InitialOrOwner = 7,			
	COND_Custom = 8,					
	COND_ReplayOrOwner = 9,			
	COND_ReplayOnly = 10,			
	COND_SimulatedOnlyNoReplay = 11,	
	COND_SimulatedOrPhysicsNoReplay = 12, 
	COND_SkipReplay = 13,
	COND_Max = 14					
};

class FRepParentCmd
{
public:
    UProperty*	Property;
    int	ArrayIndex;
    uint16_t CmdStart;
    uint16_t CmdEnd;
    int	RoleSwapIndex;
    ELifetimeCondition Condition;
    int	RepNotifyCondition;
    int	Flags;
};