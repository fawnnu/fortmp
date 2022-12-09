#pragma once

struct UNetConnection;

enum class ENetRole : uint8_t
{
    ROLE_None                      = 0,
    ROLE_SimulatedProxy            = 1,
    ROLE_AutonomousProxy           = 2,
    ROLE_Authority                 = 3,
    ROLE_MAX                       = 4
};

enum class ENetDormancy : uint8_t
{
    DORM_Never                     = 0,
    DORM_Awake                     = 1,
    DORM_DormantAll                = 2,
    DORM_DormantPartial            = 3,
    DORM_Initial                   = 4,
    DORM_MAX                       = 5
};

class AActor : public UObject
{
public:
    DEFINE_PROPERTY(class USceneComponent*, RootComponent);

    DEFINE_PROPERTY(TArray<class UActorComponent*>, BlueprintCreatedComponents);
    
    DEFINE_PROPERTY(ENetRole, RemoteRole);
    DEFINE_PROPERTY(ENetRole, Role);
    DEFINE_PROPERTY(ENetDormancy, NetDormancy);
    DEFINE_PROPERTY(AActor*, Owner);

    DEFINE_PROPERTY(FName, NetDriverName);

    DEFINE_PROPERTY(float, NetUpdateFrequency);
    DEFINE_PROPERTY(float, MinNetUpdateFrequency);

    DEFINE_BIT_PROPERTY(bool, bNetStartup);
    DEFINE_BIT_PROPERTY(bool, bOnlyRelevantToOwner);
    DEFINE_BIT_PROPERTY(bool, bTearOff);
    
    FTransform GetTransform()
    {
        STATIC_FINDFUNCTION(GetTransform)
        
        return Call<FTransform>(Function_GetTransform);
    }

    FVector K2_GetActorLocation()
    {
        STATIC_FINDFUNCTION(K2_GetActorLocation)
        
        return Call<FVector>(Function_K2_GetActorLocation);
    }
    
    FRotator K2_GetActorRotation()
    {
        STATIC_FINDFUNCTION(K2_GetActorRotation)
        
        return Call<FRotator>(Function_K2_GetActorRotation);
    }

    bool K2_SetActorRotation(FRotator NewRotation, bool bTeleportPhysics)
    {
        STATIC_FINDFUNCTION(K2_SetActorRotation)
        
        return Call<bool>(Function_K2_SetActorRotation, NewRotation, bTeleportPhysics);
    }

    bool K2_TeleportTo(FVector DestLocation, FRotator DestRotation)
    {
        STATIC_FINDFUNCTION(K2_TeleportTo)
        
        return Call<bool>(Function_K2_TeleportTo, DestLocation, DestRotation);
    }

    void SetOwner(UObject* NewOwner)
    {
        STATIC_FINDFUNCTION(SetOwner)
        
        Call(Function_SetOwner, NewOwner);
    }

    AActor* GetOwner()
    {
        STATIC_FINDFUNCTION(GetOwner)
    
        return Call<AActor*>(Function_GetOwner);
    }

    float GetDistanceTo(AActor* OtherActor)
    {
        STATIC_FINDFUNCTION(GetDistanceTo)
        
        return Call<float>(Function_GetDistanceTo, OtherActor);
    }

    void ForceNetUpdate()
    {
        STATIC_FINDFUNCTION(ForceNetUpdate)
        
        Call(Function_ForceNetUpdate);
    }

    void K2_DestroyActor()
    {
        STATIC_FINDFUNCTION(K2_DestroyActor)
        
        Call(Function_K2_DestroyActor);
    }
};

struct UChannelBitField
{
    uint8_t OpenAcked : 1;
    uint8_t Closing : 1;
    uint8_t	Dormant : 1;
    uint8_t	bIsReplicationPaused : 1;
    uint8_t	OpenTemporary : 1;
    uint8_t	Broken : 1;
    uint8_t bTornOff : 1;
    uint8_t bPendingDormancy : 1;
};

class UChannel : public UObject
{
public:
    DEFINE_PROPERTY(UNetConnection*, Connection);

    // Fix for non-UProperty bit field
    DEFINE_PROPERTY_BY_OFFSET(UChannelBitField, BitField, 0x30);
};

class UActorChannel : public UChannel
{
public:
    DEFINE_PROPERTY(AActor*, Actor);

    DEFINE_PROPERTY_BY_OFFSET(double, RelevantTime, 0x78);
    DEFINE_PROPERTY_BY_OFFSET(double, LastUpdateTime, 0x80);
};


