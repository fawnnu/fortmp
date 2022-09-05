#pragma once

class UActorComponent : public UObject
{
public:
    AActor* GetOwner()
    {
        return Call<AActor*>(L"GetOwner");
    }
};

class UFortControllerComponent_Aircraft : public UActorComponent
{
public:
    DEFINE_PROPERTY(AFortAircraft*, CurrentAircraft);
};

class USceneComponent : public UActorComponent
{
public:
    void K2_SetWorldLocation(FVector Translation, bool bSweep, bool bTeleport, struct FHitResult* OutHitResult)
    {
        Call(L"K2_SetWorldLocation", Translation, bSweep, OutHitResult, bTeleport);
    }

    void SetPhysicsLinearVelocity(FVector LinearVelocity, bool bAddToCurrent, FName BoneName)
    {
        Call(L"SetPhysicsLinearVelocity", LinearVelocity, bAddToCurrent, BoneName);
    }
    
    void SetPhysicsAngularVelocityInRadians(FVector AngularVelocity, bool bAddToCurrent, FName BoneName)
    {
        Call(L"SetPhysicsAngularVelocityInRadians", AngularVelocity, bAddToCurrent, BoneName);
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
};