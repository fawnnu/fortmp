#pragma once
#include "../../Engine/Components/ActorComponent.hpp"

inline uintptr_t (*ServerChoosePart_Implementation)(class AFortPlayerPawn*, int, UCustomCharacterPart*);

class AFortPlayerController;
class AFortWeapon;
class AFortPickup;
class UFortDecoItemDefinition;

class UCustomCharacterPart : public UObject
{
public:
    DEFINE_PROPERTY(uint8_t, CharacterPartType);
    DEFINE_PROPERTY(uint8_t, GenderPermitted);
};

class UFortHeroSpecialization : public UObject
{
public:
    DEFINE_PROPERTY(TArray<TSoftObjectPtr<UCustomCharacterPart>>, CharacterParts);
};

class UFortHeroType : public UObject
{
public:
    DEFINE_PROPERTY(TArray<TSoftObjectPtr<UFortHeroSpecialization>>, Specializations);
};

struct FReplicatedPhysicsPawnState
{
    struct FVector Translation;                                  
    unsigned char UnknownData00[0x4];            
    struct FQuat Rotation;                      
    struct FVector LinearVelocity;                              
    struct FVector AngularVelocity;                             
    uint16_t SyncKey;                       
    unsigned char UnknownData01[0x6];      
};

class UCharacterMovementComponent : public UObject
{
public:
    DEFINE_PROPERTY(uint8_t, MovementMode);
    DEFINE_BIT_PROPERTY(bool, bCheatFlying);
    
    void SetMovementMode(uint8_t CustomMovementMode, uint8_t MovementMode)
    {
        STATIC_FINDFUNCTION(SetMovementMode)
        
        Call(Function_SetMovementMode, CustomMovementMode, MovementMode);
    }
};

class APawn : public AActor
{
public:
    DEFINE_PROPERTY(AFortPlayerStateAthena*, PlayerState);

    class AController* GetController()
    {
        STATIC_FINDFUNCTION(GetController)

        return Call<AController*>(Function_GetController);
    }
};

class ACharacter : public APawn
{
public:
    DEFINE_BIT_PROPERTY(bool, bIsInvulnerable);
    
    DEFINE_PROPERTY(UCharacterMovementComponent*, CharacterMovement);
    DEFINE_PROPERTY(UPrimitiveComponent*, CapsuleComponent);
};

class UFortMovementSet : public UObject
{
public:
    DEFINE_PROPERTY(struct FGameplayAttributeData, RunSpeed);
    DEFINE_PROPERTY(struct FGameplayAttributeData, SpeedMultiplier);
};

class AFortPawn : public ACharacter
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortPawn");
    DEFINE_PROPERTY(AFortPlayerController*, Controller);
    DEFINE_PROPERTY(AFortWeapon*, CurrentWeapon);
    DEFINE_PROPERTY(class UAbilitySystemComponent*, AbilitySystemComponent);
    DEFINE_PROPERTY(UFortMovementSet*, MovementSet);
    DEFINE_PROPERTY(TArray<AFortWeapon*>, CurrentWeaponList);
    DEFINE_PROPERTY(TScriptMulticastDelegate<AFortPawn>, OnPawnLanded);
    DEFINE_PROPERTY(TArray<AFortPickup*>, IncomingPickups);
    
    DEFINE_PROPERTY(float, EmoteWalkSpeed);
    DEFINE_PROPERTY(UFortItemDefinition*, LastEmoteItemDef);
    DEFINE_PROPERTY(UFortItemDefinition*, LastReplicatedEmoteExecuted);
    
    AFortWeapon* EquipWeaponDefinition(UObject* WeaponData, FGuid ItemEntryGuid)
    {
        STATIC_FINDFUNCTION(EquipWeaponDefinition)
    
        return Call<AFortWeapon*>(Function_EquipWeaponDefinition, WeaponData, ItemEntryGuid);
    }

    void LaunchCharacterJump(FVector LaunchVelocity, bool bXYOverride, bool bZOverride, bool bIgnoreFallDamage, bool bPlayFeedbackEvent)
    {
        STATIC_FINDFUNCTION(LaunchCharacterJump)

        Call(Function_LaunchCharacterJump, LaunchVelocity, bXYOverride, bZOverride, bIgnoreFallDamage, bPlayFeedbackEvent);
    }

    bool IsDBNO()
    {
        STATIC_FINDFUNCTION(IsDBNO)
        
        return Call<bool>(Function_IsDBNO);
    }

    bool IsDead()
    {
        STATIC_FINDFUNCTION(IsDead)
        
        return Call<bool>(Function_IsDead);
    }

    bool PickUpActor(AActor* PickupTarget, UFortDecoItemDefinition* PlacementDecoItemDefinition)
    {
        STATIC_FINDFUNCTION(PickUpActor)
        
        return Call<bool>(Function_PickUpActor, PickupTarget, PlacementDecoItemDefinition);
    }

    void SetHealth(float NewHealthVal)
    {
        STATIC_FINDFUNCTION(SetHealth)
        
        Call(Function_SetHealth, NewHealthVal);
    }

    float GetHealth()
    {
        STATIC_FINDFUNCTION(GetHealth)
        
        return Call<float>(Function_GetHealth);
    }
    
    float GetShield()
    {
        STATIC_FINDFUNCTION(GetShield)
        
        return Call<float>(Function_GetShield);
    }
};


class AFortPlayerPawn : public AFortPawn
{
public:
    DEFINE_PROPERTY(UObject*, Owner);
    DEFINE_PROPERTY(FGameplayTag, EventReviveTag);
    DEFINE_PROPERTY(struct FZiplinePawnState, ZiplineState);
    DEFINE_PROPERTY(bool, bIsSkydivingFromBus);
    DEFINE_PROPERTY(struct FCurveTableRowHandle, ZiplineJumpDampening);
    DEFINE_PROPERTY(struct FCurveTableRowHandle, ZiplineJumpStrength);

    bool IsInVehicle()
    {
        STATIC_FINDFUNCTION(IsInVehicle)

        return Call<bool>(Function_IsInVehicle);
    }
    class AFortAthenaVehicle* GetVehicle()
    {
        STATIC_FINDFUNCTION(GetVehicle)

        return Call<AFortAthenaVehicle*>(Function_GetVehicle);
    }
    unsigned char GetVehicleSeatIndex()
    {
        STATIC_FINDFUNCTION(GetVehicleSeatIndex)

        return Call<unsigned char>(Function_GetVehicleSeatIndex);
    }
    void ServerChoosePart(uint8_t Part, UCustomCharacterPart* ChosenCharacterPart)
    {
        ServerChoosePart_Implementation(this, Part, ChosenCharacterPart);
    }
};

struct FWeaponSeatDefinition
{
    DEFINE_STRUCT_PROPERTY(int, SeatIndex);
    DEFINE_STRUCT_PROPERTY(UFortWeaponItemDefinition*, VehicleWeapon);
    DEFINE_STRUCT_PROPERTY(UFortWeaponItemDefinition*, VehicleWeaponOverride);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.WeaponSeatDefinition");
};

class UFortVehicleSeatWeaponComponent : public UActorComponent
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortVehicleSeatWeaponComponent")
    DEFINE_PROPERTY(TArray<FWeaponSeatDefinition>, WeaponSeatDefinitions);
    DEFINE_PROPERTY(AFortWeapon*, CachedWeapon);
    DEFINE_PROPERTY(UFortWeaponItemDefinition*, CachedWeaponDef);
};

class AFortAthenaVehicle : public APawn
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaVehicle")

    UFortVehicleSeatWeaponComponent* GetSeatWeaponComponent(int SeatIndex)
    {
        STATIC_FINDFUNCTION(GetSeatWeaponComponent)

        return Call<UFortVehicleSeatWeaponComponent*>(Function_GetSeatWeaponComponent, SeatIndex);
    }
};

class AFortPlayerPawnAthena : public AFortPlayerPawn
{
public:
    DEFINE_PROPERTY(float, PickupSpeedMultiplier);

    void TeleportToSkyDive(float HeighAboveGround)
    {
        STATIC_FINDFUNCTION(TeleportToSkyDive)
        
        Call(Function_TeleportToSkyDive, HeighAboveGround);
    }
};

class AFortAIPawn : public UObject
{
public:
    void AddFortAbilitySet(UObject* FortAbilitySet)
    {
        STATIC_FINDFUNCTIONOBJECT(L"/Script/FortniteGame.FortAIPawn.AddFortAbilitySet")
        
        Call(FunctionObject, FortAbilitySet);
    }

    void GrabPickupCustomized(AFortPickup* FortPickup, float FlyTime)
    {
        STATIC_FINDFUNCTIONOBJECT(L"/Script/FortniteGame.FortAIPawn.GrabPickupCustomized")
        
        Call(FunctionObject, FortPickup, FlyTime);
    }
};

class APlayerPawn_Athena_C : public AFortPlayerPawnAthena
{
public:
    DEFINE_STATIC_CLASS(L"/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C")
};