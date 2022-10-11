#pragma once

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
    DEFINE_BIT_PROPERTY(bool, bIsInvulnerable);
    
    DEFINE_PROPERTY(UCharacterMovementComponent*, CharacterMovement);
};

class AFortPawn : public ACharacter
{
public:
    DEFINE_PROPERTY(AFortPlayerController*, Controller);
    DEFINE_PROPERTY(AFortWeapon*, CurrentWeapon);
    DEFINE_PROPERTY(class UAbilitySystemComponent*, AbilitySystemComponent);
    
    AFortWeapon* EquipWeaponDefinition(UObject* WeaponData, FGuid ItemEntryGuid)
    {
        return Call<AFortWeapon*>(L"EquipWeaponDefinition", WeaponData, ItemEntryGuid);
    }

    bool IsDBNO()
    {
        return Call<bool>(L"IsDBNO");
    }

    bool IsDead()
    {
        return Call<bool>(L"IsDead");
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

    void ServerChoosePart(uint8_t Part, UCustomCharacterPart* ChosenCharacterPart)
    {
        ServerChoosePart_Implementation(this, Part, ChosenCharacterPart);
    }

    void ServerChooseGender(uint8_t Gender)
    {
        Call(L"ServerChooseGender", Gender);
    }
};

class AFortPlayerPawnAthena : public AFortPlayerPawn
{
public:
    DEFINE_PROPERTY(float, PickupSpeedMultiplier);
};

class AFortAIPawn : public UObject
{
public:
    void AddFortAbilitySet(UObject* FortAbilitySet)
    {
        Call(L"/Script/FortniteGame.FortAIPawn.AddFortAbilitySet", FortAbilitySet);
    }

    void GrabPickupCustomized(AFortPickup* FortPickup, float FlyTime)
    {
        Call(L"/Script/FortniteGame.FortAIPawn.GrabPickupCustomized", FortPickup, FlyTime);
    }
};
