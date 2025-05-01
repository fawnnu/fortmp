#pragma once

class UActorComponent : public UObject
{
public:
    AActor* GetOwner()
    {
        STATIC_FINDFUNCTION(GetOwner)
        
        return Call<AActor*>(Function_GetOwner);
    }

    void RegisterComponent()
    {
        if (UActorComponent__RegisterComponentInWorld)
        {
            UActorComponent__RegisterComponentInWorld(this, *GWorld);
        }
        else
        {
            spdlog::error("Failed to register component in World: UActorComponent__RegisterComponentInWorld is NULL");
        }
    }
};

class AFortAircraft;

class UFortControllerComponent_Aircraft : public UActorComponent
{
public:
    DEFINE_PROPERTY(AFortAircraft*, CurrentAircraft);
};

struct FHitResult
{
    DEFINE_STRUCT_PROPERTY(FVector, Location);
    DEFINE_STRUCT_PROPERTY(FVector, TraceEnd);
    DEFINE_STRUCT_PROPERTY(FVector, ImpactPoint);
    DEFINE_STRUCT_PROPERTY(FVector, TraceStart);
    DEFINE_STRUCT_PROPERTY(FVector, Normal);
    DEFINE_STRUCT_PROPERTY(FVector, ImpactNormal);
    DEFINE_STRUCT_PROPERTY(TWeakObjectPtr<AActor>, Actor);
    DEFINE_STRUCT_PROPERTY(FName, BoneName);
    char pad[0x88];
    
    DEFINE_STRUCT_CLASS(L"/Script/Engine.HitResult")
    OVERLOAD_NEW_OPERATOR();

    FORCEINLINE std::string ToAnsiString()
    {
        return std::string("Location: ") +
            std::string(Location()) +
            "Trace End: " +
            std::string(TraceEnd()) +
            "ImpactPoint: " +
            std::string(ImpactPoint()) +
            "ImpactNormal: " +
            std::string(ImpactNormal()) +
            "Actor: " +
            std::string(Actor().Get()->sGetName()) +
            "BoneName: " +
            std::string(BoneName().sToString()) +
            "TraceStart: " +
            std::string(TraceStart()) +
            "Normal: " +
            std::string(Normal());
    }

    FORCEINLINE std::wstring ToUnicodeString()
    {
        return std::wstring(L"Location: ") +
            std::wstring(Location()) +
            L"Trace End: " +
            std::wstring(TraceEnd()) +
            L"ImpactPoint: " +
            std::wstring(ImpactPoint()) +
            L"ImpactNormal: " +
            std::wstring(ImpactNormal()) +
            L"Actor: " +
            std::wstring(Actor().Get()->GetName()) +
            L"BoneName: " +
            std::wstring(BoneName().ToString()) +
            L"TraceStart: " +
            std::wstring(TraceStart()) +
            L"Normal: " +
            std::wstring(Normal());
    }

    FORCEINLINE operator const char*()
    {
        return ToAnsiString().c_str();
    }

    FORCEINLINE operator const wchar_t*()
    {
        return ToUnicodeString().c_str();
    }
};

class EAttachmentRule : public UEnum
{
public:
    DEFINE_STATIC_CLASS(L"/Script/Engine.EAttachmentRule")

    ENUM_PROPERTY(KeepRelative);
};

class USceneComponent : public UActorComponent
{
public:
    DEFINE_PROPERTY(USceneComponent*, AttachParent);
    DEFINE_PROPERTY(TArray<USceneComponent*>, AttachChildren);

    bool K2_AttachToComponent(USceneComponent* Parent, FName SocketName, uint8_t LocationRule, uint8_t RotationRule, uint8_t ScaleRule, bool bWeldSimulatedBodies)
    {
        STATIC_FINDFUNCTION(K2_AttachToComponent)

        return Call<bool>(Function_K2_AttachToComponent, Parent, SocketName, LocationRule, RotationRule, ScaleRule, bWeldSimulatedBodies);
    }
    
    FVector K2_GetComponentLocation()
    {
        STATIC_FINDFUNCTION(K2_GetComponentLocation)

        return Call<FVector>(Function_K2_GetComponentLocation);
    }
    
    FVector GetSocketLocation(FName InSocketName)
    {
        STATIC_FINDFUNCTION(GetSocketLocation)

        return Call<FVector>(Function_GetSocketLocation, InSocketName);
    }
    
    FVector GetForwardVector()
    {
        STATIC_FINDFUNCTION(GetForwardVector)

        return Call<FVector>(Function_GetForwardVector);
    }
    
    void K2_SetWorldLocation(FVector Translation, bool bSweep, bool bTeleport, FHitResult* OutHitResult)
    {
        STATIC_FINDFUNCTION(K2_SetWorldLocation)
        
        Call(Function_K2_SetWorldLocation, Translation, bSweep, OutHitResult, bTeleport);
    }

    void SetPhysicsLinearVelocity(FVector LinearVelocity, bool bAddToCurrent, FName BoneName)
    {
        STATIC_FINDFUNCTION(SetPhysicsLinearVelocity)
        
        Call(Function_SetPhysicsLinearVelocity, LinearVelocity, bAddToCurrent, BoneName);
    }
    
    void SetPhysicsAngularVelocityInRadians(FVector AngularVelocity, bool bAddToCurrent, FName BoneName)
    {
        STATIC_FINDFUNCTION(SetPhysicsAngularVelocityInRadians)
        
        Call(Function_SetPhysicsAngularVelocityInRadians, AngularVelocity, bAddToCurrent, BoneName);
    }
};

class UMovementComponent : public UActorComponent
{
public:
    DEFINE_PROPERTY(FVector, Velocity);
};

class UProjectileMovementComponent : public UMovementComponent
{
public:
    DEFINE_PROPERTY(float, MaxSpeed);
    DEFINE_PROPERTY(TScriptMulticastDelegate<UProjectileMovementComponent>, OnProjectileStop)

    DEFINE_STATIC_CLASS(L"/Script/Engine.ProjectileMovementComponent")
};

class ECollisionEnabled : public UEnum
{
public:
    DEFINE_STATIC_ENUM_CLASS(L"/Script/Engine.ECollisionEnabled")

    ENUM_PROPERTY(QueryOnly);
};

class ECollisionChannel : public UEnum
{
public:
    DEFINE_STATIC_ENUM_CLASS(L"/Script/Engine.ECollisionChannel")

    ENUM_PROPERTY(ECC_WorldDynamic);
};

class ECollisionResponse : public UEnum
{
public:
    DEFINE_STATIC_ENUM_CLASS(L"/Script/Engine.ECollisionResponse")

    ENUM_PROPERTY(ECR_Overlap);
};

class UPrimitiveComponent : public USceneComponent
{
public:
    DEFINE_PROPERTY(TArray<AActor*>, MoveIgnoreActors);

    void SetCollisionEnabled(uint8_t NewType)
    {
        STATIC_FINDFUNCTION(SetCollisionEnabled)
        
        Call(Function_SetCollisionEnabled, NewType);
    }

    void SetCollisionResponseToChannel(uint8_t Channel, uint8_t NewResponse)
    {
        STATIC_FINDFUNCTION(SetCollisionResponseToChannel)
        
        Call(Function_SetCollisionResponseToChannel, Channel, NewResponse);
    }

    void SetCollisionResponseToAllChannels(uint8_t NewResponse)
    {
        STATIC_FINDFUNCTION(SetCollisionResponseToAllChannels)
        
        Call(Function_SetCollisionResponseToAllChannels, NewResponse);
    }
    
    uint8_t GetCollisionObjectType()
    {
        STATIC_FINDFUNCTION(GetCollisionObjectType)
        
        return Call<uint8_t>(Function_GetCollisionObjectType);
    }
    
    void SetCollisionObjectType(uint8_t Collision)
    {
        STATIC_FINDFUNCTION(SetCollisionObjectType)
        
        Call(Function_SetCollisionObjectType, Collision);
    }
};

struct FMarkerID
{
    FMarkerID() = default;
    FMarkerID(int InPlayerID, int InInstanceId)
    {
        PlayerID = InPlayerID;
        InstanceId = InInstanceId;
    }
    
    int PlayerID;
    int InstanceId;
};

struct FFortClientMarkerRequest
{
    DEFINE_STRUCT_PROPERTY(int, InstanceId);
    DEFINE_STRUCT_PROPERTY(uint8_t, MarkerType);
    DEFINE_STRUCT_PROPERTY(FVector, WorldPosition);
    DEFINE_STRUCT_PROPERTY(FVector, WorldPositionOffset);
    DEFINE_STRUCT_PROPERTY(FVector, WorldNormal);
    DEFINE_STRUCT_PROPERTY(AActor*, MarkedActor);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortClientMarkerRequest")
};

struct FFortWorldMarkerData
{
    DEFINE_STRUCT_PROPERTY(FMarkerID, MarkerID);
    DEFINE_STRUCT_PROPERTY(class AFortPlayerState*, Owner);
    DEFINE_STRUCT_PROPERTY(uint8_t, MarkerType);
    DEFINE_STRUCT_PROPERTY(FVector, WorldPosition);
    DEFINE_STRUCT_PROPERTY(FVector, WorldPositionOffset);
    DEFINE_STRUCT_PROPERTY(FVector, WorldNormal);
    DEFINE_STRUCT_PROPERTY(UFortItemDefinition*, ItemDefinition);
    DEFINE_STRUCT_PROPERTY(int, ItemCount);
    DEFINE_STRUCT_PROPERTY(TSoftObjectPtr<AActor>, MarkedActor);
    DEFINE_STRUCT_PROPERTY(TSoftObjectPtr<UObject>, MarkedActorClass);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortWorldMarkerData")
};

class UAthenaMarkerComponent : public UActorComponent
{
public:

    void ClientAddMarker(FFortWorldMarkerData* MarkerData)
    {
        STATIC_FINDFUNCTION(ClientAddMarker)

        ProcessEvent(this, Function_ClientAddMarker, MarkerData);
    }
    
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.AthenaMarkerComponent");
};

class UBoxComponent : public USceneComponent
{
public:
    FVector GetUnscaledBoxExtent()
    {
        STATIC_FINDFUNCTION(GetUnscaledBoxExtent)

        return Call<FVector>(Function_GetUnscaledBoxExtent);
    }
    FVector GetScaledBoxExtent()
    {
        STATIC_FINDFUNCTION(GetScaledBoxExtent)

        return Call<FVector>(Function_GetScaledBoxExtent);
    }
};

class USphereComponent : public UPrimitiveComponent
{
public:
    DEFINE_STATIC_CLASS(L"/Script/Engine.SphereComponent");

    void SetSphereRadius(float InSphereRadius, bool bUpdateOverlaps)
    {
        STATIC_FINDFUNCTION(SetSphereRadius)

        Call(Function_SetSphereRadius, InSphereRadius, bUpdateOverlaps);
    }
};