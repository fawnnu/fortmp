#pragma once

#define CLOSEPROXIMITY					500.f
#define NEARSIGHTTHRESHOLD				2000.f
#define MEDSIGHTTHRESHOLD				3162.f
#define FARSIGHTTHRESHOLD				8000.f
#define CLOSEPROXIMITYSQUARED			(CLOSEPROXIMITY*CLOSEPROXIMITY)
#define NEARSIGHTTHRESHOLDSQUARED		(NEARSIGHTTHRESHOLD*NEARSIGHTTHRESHOLD)
#define MEDSIGHTTHRESHOLDSQUARED		(MEDSIGHTTHRESHOLD*MEDSIGHTTHRESHOLD)
#define FARSIGHTTHRESHOLDSQUARED		(FARSIGHTTHRESHOLD*FARSIGHTTHRESHOLD)

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

    DEFINE_STATIC_CLASS(L"/Script/Engine.Actor");
    
    DEFINE_PROPERTY(class USceneComponent*, RootComponent);
    DEFINE_PROPERTY(float, CustomTimeDilation);
    DEFINE_PROPERTY(TArray<class UActorComponent*>, BlueprintCreatedComponents);

    DEFINE_PROPERTY(AActor*, Instigator);
    
    DEFINE_PROPERTY(ENetRole, RemoteRole);
    DEFINE_PROPERTY(ENetRole, Role);
    DEFINE_PROPERTY(ENetDormancy, NetDormancy);
    DEFINE_PROPERTY(AActor*, Owner);

    DEFINE_PROPERTY(FName, NetDriverName);
    DEFINE_PROPERTY(int, NetTag);
    DEFINE_PROPERTY(float, NetUpdateFrequency);
    DEFINE_PROPERTY(float, MinNetUpdateFrequency);

    DEFINE_BIT_PROPERTY(bool, bNetStartup);
    DEFINE_BIT_PROPERTY(bool, bOnlyRelevantToOwner);
    DEFINE_BIT_PROPERTY(bool, bTearOff);
    DEFINE_BIT_PROPERTY(bool, bNetUseOwnerRelevancy);
    DEFINE_BIT_PROPERTY(bool, bHidden);
    DEFINE_PROPERTY(float, NetPriority);

    AActor* GetNetOwner()
    {
        return reinterpret_cast<AActor* (*)(AActor* Self)>(this->VTable[NetworkOffsets::GetNetOwnerVTableIdx])(this);
    }
    
    FTransform GetTransform()
    {
        STATIC_FINDFUNCTION(GetTransform)
        
        return Call<FTransform>(Function_GetTransform);
    }

    void SetActorScale3D(FVector& NewScale3D)
    {
        STATIC_FINDFUNCTION(SetActorScale3D)
        
        Call(Function_SetActorScale3D, NewScale3D);
    }
    
    FVector K2_GetActorLocation()
    {
        STATIC_FINDFUNCTION(K2_GetActorLocation)
        
        return Call<FVector>(Function_K2_GetActorLocation);
    }

    FVector GetActorForwardVector()
    {
        STATIC_FINDFUNCTION(GetActorForwardVector)
        
        return Call<FVector>(Function_GetActorForwardVector);
    }
    
    FRotator K2_GetActorRotation()
    {
        STATIC_FINDFUNCTION(K2_GetActorRotation)
        
        return Call<FRotator>(Function_K2_GetActorRotation);
    }

    void SetActorHiddenInGame(bool bNewHidden)
    {
        STATIC_FINDFUNCTION(SetActorHiddenInGame)
        
        Call(Function_SetActorHiddenInGame);
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

    float GetNetPriority(const FVector& ViewPos, const FVector& ViewDir, AActor* Viewer, AActor* ViewTarget, class UActorChannel* InChannel, float Time, bool bLowBandwidth)
    {
        AActor* Actor = this;
        
        if (!Actor) return 0;

        if (Actor->bNetUseOwnerRelevancy() && Actor->Owner())
        {
            // If we should use our owner's priority, pass it through
            return Actor->Owner()->GetNetPriority(ViewPos, ViewDir, Viewer, ViewTarget, InChannel, Time, bLowBandwidth);
        }

        if (ViewTarget && (Actor == ViewTarget || Actor->Instigator() == ViewTarget))
        {
            // If we're the view target or owned by the view target, use a high priority
            Time *= 4.f;
        }
        else if (!Actor->bHidden() && Actor->RootComponent() != NULL)
        {
            // If this actor has a location, adjust priority based on location
            FVector Dir = Actor->K2_GetActorLocation() - ViewPos;
            float DistSq = Dir.SizeSquared();

            // Adjust priority based on distance and whether actor is in front of viewer
            if ((ViewDir | Dir) < 0.f)
            {
                if (DistSq > NEARSIGHTTHRESHOLDSQUARED)
                {
                    Time *= 0.2f;
                }
                else if (DistSq > CLOSEPROXIMITYSQUARED)
                {
                    Time *= 0.4f;
                }
            }
            else if ((DistSq < FARSIGHTTHRESHOLDSQUARED) && (FMath::Square(ViewDir | Dir) > 0.5f * DistSq))
            {
                // Compute the amount of distance along the ViewDir vector. Dir is not normalized
                // Increase priority if we're being looked directly at
                Time *= 2.f;
            }
            else if (DistSq > MEDSIGHTTHRESHOLDSQUARED)
            {
                Time *= 0.4f;
            }
        }

        return Actor->NetPriority() * Time;
    }

    FVector GetVelocity()
    {
        STATIC_FINDFUNCTION(GetVelocity)

        return Call<FVector>(Function_GetVelocity);
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

    uint32_t Closing()
    {
        static void* native_property = FindPropertyByName(this->Class, "Connection");                                                                              
        static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);

        struct BitField
        {
            uint32_t OpenAcked : 1;
            uint32_t Closing : 1;
        };
        
        return reinterpret_cast<BitField*>(reinterpret_cast<uintptr_t>(this) + native_property_offset + 8)->Closing;
    }

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


