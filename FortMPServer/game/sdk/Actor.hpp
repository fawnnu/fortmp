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

    DEFINE_PROPERTY(ENetRole, RemoteRole);
    DEFINE_PROPERTY(ENetRole, Role);
    DEFINE_PROPERTY(ENetDormancy, NetDormancy);

    DEFINE_PROPERTY(FName, NetDriverName);

    DEFINE_PROPERTY(float, NetUpdateFrequency);
    DEFINE_PROPERTY(float, MinNetUpdateFrequency);

    DEFINE_BIT_PROPERTY(bool, bNetStartup);
    DEFINE_BIT_PROPERTY(bool, bOnlyRelevantToOwner);
    DEFINE_BIT_PROPERTY(bool, bTearOff);
    
    FTransform GetTransform()
    {
        return Call<FTransform>(L"GetTransform");
    }

    FVector K2_GetActorLocation()
    {
        return Call<FVector>(L"K2_GetActorLocation");
    }
    
    FRotator K2_GetActorRotation()
    {
        return Call<FRotator>(L"K2_GetActorRotation");
    }

    bool K2_SetActorRotation(FRotator NewRotation, bool bTeleportPhysics)
    {
        return Call<bool>(L"K2_SetActorRotation", NewRotation, bTeleportPhysics);
    }

    void SetOwner(UObject* NewOwner)
    {
        Call(L"SetOwner", NewOwner);
    }

    AActor* GetOwner()
    {
        return Call<AActor*>(L"GetOwner");
    }

    float GetDistanceTo(AActor* OtherActor)
    {
        return Call<float>(L"GetDistanceTo", OtherActor);
    }

    void ForceNetUpdate()
    {
        Call(L"ForceNetUpdate");
    }
};

class UChannel : public UObject
{
public:
    DEFINE_PROPERTY(UNetConnection*, Connection);
};

class UActorChannel : public UChannel
{
public:
    DEFINE_PROPERTY(AActor*, Actor);

    DEFINE_PROPERTY_BY_OFFSET(double, RelevantTime, 0x80);
    DEFINE_PROPERTY_BY_OFFSET(double, LastUpdateTime, 0x88);
};


