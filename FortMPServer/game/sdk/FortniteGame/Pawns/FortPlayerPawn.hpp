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
        STATIC_FINDFUNCTION(SetMovementMode)
        
        Call(Function_SetMovementMode, CustomMovementMode, MovementMode);
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
        STATIC_FINDFUNCTION(EquipWeaponDefinition)
    
        return Call<AFortWeapon*>(Function_EquipWeaponDefinition, WeaponData, ItemEntryGuid);
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
    
    void ServerChoosePart(uint8_t Part, UCustomCharacterPart* ChosenCharacterPart)
    {
        ServerChoosePart_Implementation(this, Part, ChosenCharacterPart);
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
    static UObject* StaticClass()
    {
        static UObject* static_class = FindObject(L"/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");
        
        return static_class;
    }
};