#pragma once

class AFortPlayerController;
class AFortWeapon;
class AFortPickup;
class UFortDecoItemDefinition;

class UCharacterMovementComponent : public UObject
{
public:
    DEFINE_PROPERTY(uint8_t, MovementMode);
    
    void SetMovementMode(uint8_t CustomMovementMode, uint8_t MovementMode)
    {
        Call(L"SetMovementMode", CustomMovementMode, MovementMode);
    }
};

class APawn : public AActor
{
public:
    DEFINE_PROPERTY(AFortPlayerStateAthena*, PlayerState);
};

class ACharacter : public APawn
{
public:
    DEFINE_PROPERTY(UCharacterMovementComponent*, CharacterMovement);
};

class AFortPawn : public ACharacter
{
public:
    DEFINE_PROPERTY(AFortPlayerController*, Controller);
    DEFINE_PROPERTY(AFortWeapon*, CurrentWeapon);
    DEFINE_PROPERTY(class UAbilitySystemComponent*, AbilitySystemComponent);
    
    UObject* EquipWeaponDefinition(UObject* WeaponData, FGuid ItemEntryGuid)
    {
        return Call<UObject*>(L"EquipWeaponDefinition", WeaponData, ItemEntryGuid);
    }

    bool IsDBNO()
    {
        return Call<bool>(L"IsDBNO");
    }

    bool PickUpActor(AActor* PickupTarget, UFortDecoItemDefinition* PlacementDecoItemDefinition)
    {
        return Call<bool>(L"PickUpActor", PickupTarget, PlacementDecoItemDefinition);
    }

    void SetHealth(float NewHealthVal)
    {
        Call(L"SetHealth", NewHealthVal);
    }
};


class AFortPlayerPawn : public AFortPawn
{
public:
    DEFINE_PROPERTY(UObject*, Owner);
    
    DEFINE_PROPERTY(struct FZiplinePawnState, ZiplineState);

    void ServerHandlePickup(AFortPickup* Pickup, float InFlyTime, FVector InStartDirection, bool bPlayPickupSound)
    {
        Call(L"ServerHandlePickup", Pickup, InFlyTime, InStartDirection, bPlayPickupSound);
    }
};

class AFortAIPawn : public UObject
{
public:
    void AddFortAbilitySet(UObject* FortAbilitySet)
    {
        Call(L"/Script/FortniteGame.FortAIPawn.AddFortAbilitySet", FortAbilitySet);
    }
};
