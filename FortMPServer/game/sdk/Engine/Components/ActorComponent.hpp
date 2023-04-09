#pragma once

class UActorComponent : public UObject
{
public:
    AActor* GetOwner()
    {
        STATIC_FINDFUNCTION(GetOwner)
        
        return Call<AActor*>(Function_GetOwner);
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
    
    DEFINE_STRUCT_CLASS(TEXT("/Script/Engine.HitResult"))
};

class USceneComponent : public UActorComponent
{
public:
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
};

class UPrimitiveComponent : public UActorComponent
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
    
    uint8_t GetCollisionObjectType()
    {
        STATIC_FINDFUNCTION(GetCollisionObjectType)
        
        return Call<uint8_t>(Function_GetCollisionObjectType);
    }
};