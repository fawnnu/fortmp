#pragma once
#include "../../Engine/Components/ActorComponent.hpp"
#include "game/sdk/FortniteGame/Items/FortItem.hpp"

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

class EFortCustomGender : public UEnum
{
public:
    DEFINE_STATIC_ENUM_CLASS(L"/Script/FortniteGame.EFortCustomGender");
    
    ENUM_PROPERTY(Male);
    ENUM_PROPERTY(Female);
};

class UFortHeroSpecialization : public UObject
{
public:
    DEFINE_PROPERTY(TArray<TSoftObjectPtr<UCustomCharacterPart>>, CharacterParts);
};

class UFortHeroType : public UFortItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortHeroType")
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

struct FNetTowhookAttachState
{
    DEFINE_STRUCT_PROPERTY(UPrimitiveComponent*, Component);
    DEFINE_STRUCT_PROPERTY(struct FVector, LocalLocation); 
    DEFINE_STRUCT_PROPERTY(struct FVector, LocalNormal);

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.NetTowhookAttachState");
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
    DEFINE_PROPERTY(class AFortPlayerStateAthena*, PlayerState);

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
    DEFINE_PROPERTY(TScriptMulticastDelegate<AFortPawn>, OnDamaged);
    DEFINE_PROPERTY(TArray<AFortPickup*>, IncomingPickups);
    DEFINE_PROPERTY(class UFortHealthSet*, HealthSet);
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

    void SetMaxHealth(float NewHealthVal)
    {
        STATIC_FINDFUNCTION(SetMaxHealth)
        
        Call(Function_SetMaxHealth, NewHealthVal);
    }

    void SetShield(float NewShieldVal);

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

    float GetMaxHealth()
    {
        STATIC_FINDFUNCTION(GetMaxHealth)
        
        return Call<float>(Function_GetMaxHealth);
    }
};

struct FFortClientObservedStat : FFastArraySerializerItem
{
    DEFINE_STRUCT_PROPERTY(FName, StatName);
    DEFINE_STRUCT_PROPERTY(int32, StatValue);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortClientObservedStat");
};

struct FBodyInstance
{
    DEFINE_STRUCT_CLASS(L"/Script/Engine.BodyInstance");
};

struct FFortClientObservedStatArray : FFastArraySerializer
{
    DEFINE_STRUCT_PROPERTY(TArray<FFortClientObservedStat>, ObservedStats);

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortClientObservedStatArray");
};

class AFortPlayerPawn : public AFortPawn
{
public:
    DEFINE_PROPERTY(UObject*, Owner);
    DEFINE_PROPERTY(FGameplayTag, EventReviveTag);
    DEFINE_PROPERTY(struct FZiplinePawnState, ZiplineState);
    DEFINE_BIT_PROPERTY(bool, bIsSkydivingFromBus);
    DEFINE_PROPERTY(struct FCurveTableRowHandle, ZiplineJumpDampening);
    DEFINE_PROPERTY(struct FCurveTableRowHandle, ZiplineJumpStrength);
    DEFINE_PROPERTY(TWeakObjectPtr<UFortHero>, Hero);
    DEFINE_PROPERTY(UObject*, AnimBPOverride);
    DEFINE_PROPERTY(uint8, PreviousStasisMode);
    DEFINE_PROPERTY(FFortClientObservedStatArray, ClientObservedStats);

    FFortAthenaLoadout& CustomizationLoadout()
    {
        static void* native_property = FindPropertyByName(this->Class, TEXT("CustomizationLoadout")) ? FindPropertyByName(this->Class, TEXT("CustomizationLoadout")) : FindPropertyByName(this->Class, TEXT("CosmeticLoadout"));                                          
        static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);   
        
        return *reinterpret_cast<FFortAthenaLoadout*>(__int64(this) + native_property_offset);                
    }

    void OnRep_CosmeticLoadout()
    {
        STATIC_FINDFUNCTION(OnRep_CosmeticLoadout)

        Call(Function_OnRep_CosmeticLoadout);
    }
    
    void OnRep_AnimBPOverride()
    {
        STATIC_FINDFUNCTION(OnRep_AnimBPOverride)

        Call(Function_OnRep_AnimBPOverride);
    }

    void SetStasisMode(uint8 InStasisMode)
    {
        STATIC_FINDFUNCTION(SetStasisMode)

        Call(Function_SetStasisMode, InStasisMode);
    }

    void HandleInStasis()
    {
        STATIC_FINDFUNCTION(HandleInStasis)

        Call(Function_HandleInStasis);
    }
    
    class AFortAthenaVehicle* ServerOnExitVehicle()
    {
        STATIC_FINDFUNCTION(ServerOnExitVehicle)

        if (Function_ServerOnExitVehicle)
        {
            //static bool bExists_ExitForceBehavior = FindPropertyByName(Function_ServerOnExitVehicle, L"ExitForceBehavior") != nullptr;
            //UEnum* ETryExitVehicleBehavior = FindObject<UEnum>(L"/Script/FortniteGame.ETryExitVehicleBehavior");
//
            //if (bExists_ExitForceBehavior && ETryExitVehicleBehavior)
            //{
            //    return Call<AFortAthenaVehicle*>(Function_ServerOnExitVehicle, ETryExitVehicleBehavior->GetEnumIndex(L"ForceAlways"));
            //}
            //else
            {
                return Call<AFortAthenaVehicle*>(Function_ServerOnExitVehicle);
            }
        }

        return nullptr;
    }
    bool IsInVehicle()
    {
        STATIC_FINDFUNCTION(IsInVehicle)

        return Call<bool>(Function_IsInVehicle);
    }
    int GetNumActiveBalloons()
    {
        STATIC_FINDFUNCTION(GetNumActiveBalloons)

        return Call<int>(Function_GetNumActiveBalloons);
    }
    bool IsSkydiving()
    {
        STATIC_FINDFUNCTION(IsSkydiving)

        return Call<bool>(Function_IsSkydiving);
    }

    void SetPawnVisibility(bool bNewHidden, bool bPropagateToWeapon)
    {
        STATIC_FINDFUNCTION(SetPawnVisibility)

        Call(Function_SetPawnVisibility, bNewHidden, bPropagateToWeapon);
    }
    
    void BeginSkydiving(bool bFromBus)
    {
        STATIC_FINDFUNCTION(BeginSkydiving)

        Call(Function_BeginSkydiving, bFromBus);
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

    DEFINE_PROPERTY(float, GravityMultiplier);
    
    UFortVehicleSeatWeaponComponent* GetSeatWeaponComponent(int SeatIndex)
    {
        STATIC_FINDFUNCTION(GetSeatWeaponComponent)

        return Call<UFortVehicleSeatWeaponComponent*>(Function_GetSeatWeaponComponent, SeatIndex);
    }

    AFortPlayerPawn* GetPawnAtSeat(int SeatIdx)
    {
        STATIC_FINDFUNCTION(GetPawnAtSeat)

        return Call<AFortPlayerPawn*>(Function_GetPawnAtSeat, SeatIdx);
    }

    AFortPlayerPawn* GetDriver()
    {
        STATIC_FINDFUNCTION(GetDriver)

        return Call<AFortPlayerPawn*>(Function_GetDriver);
    }

    float GetHealth()
    {
        STATIC_FINDFUNCTION(GetHealth)

        return Call<float>(Function_GetHealth);
    }

    void OnRep_GravityMultiplier()
    {
        STATIC_FINDFUNCTION(OnRep_GravityMultiplier)

        Call(Function_OnRep_GravityMultiplier);
    }
};

class AFortAthenaSKVehicle : public AFortAthenaVehicle
{
public:
    DEFINE_PROPERTY(float, WheelSpinFR);
    DEFINE_PROPERTY(float, WheelSpinFL);
    DEFINE_PROPERTY(float, WheelSpinBR);
    DEFINE_PROPERTY(float, WheelSpinBL);
    DEFINE_PROPERTY(float, WheelSpinVelocityFR);
    DEFINE_PROPERTY(float, WheelSpinVelocityFL);
    DEFINE_PROPERTY(float, WheelSpinVelocityBR);
    DEFINE_PROPERTY(float, WheelSpinVelocityBL);
};

class AShoppingCartVehicleSK : public AFortAthenaVehicle
{
public:
    DEFINE_CLASS(L"/Game/Athena/DrivableVehicles/ShoppingCartVehicleSK.ShoppingCartVehicleSK_C")
};

class AGolfCartVehicleSK : public AFortAthenaVehicle
{
public:
    DEFINE_CLASS(L"/Game/Athena/DrivableVehicles/Golf_Cart/Golf_Cart_Base/Blueprints/GolfCartVehicleSK.GolfCartVehicleSK_C")
};

class AFortOctopusVehicle : public AFortAthenaVehicle
{
public:
    DEFINE_CLASS(L"/Script/FortniteGame.FortOctopusVehicle")

    DEFINE_PROPERTY(FNetTowhookAttachState, ReplicatedAttachState);
    
    bool IsTowhookAttached()
    {
        STATIC_FINDFUNCTION(IsTowhookAttached)

        return Call<bool>(Function_IsTowhookAttached);
    }

    void OnRep_ReplicatedAttachState()
    {
        STATIC_FINDFUNCTION(OnRep_ReplicatedAttachState)
        
        Call(Function_OnRep_ReplicatedAttachState);
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

    bool IsActivelyStrafingInAir()
    {
        STATIC_FINDFUNCTION(IsActivelyStrafingInAir)
        
        return Call<bool>(Function_IsActivelyStrafingInAir);
    }

    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortPlayerPawnAthena");
};

class AFortAthenaDoghouseVehicle : public AFortAthenaVehicle
{
public:
    bool IsFlying()
    {
        STATIC_FINDFUNCTION(IsFlying)

        return Call<bool>(Function_IsFlying);
    }
};

class AFortAthenaFerretVehicle : public AFortAthenaDoghouseVehicle
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaFerretVehicle")
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
    DEFINE_CLASS(L"/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C")
};

class AFortAthenaSKPushCannon : public AFortAthenaVehicle
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaSKPushCannon");
    
    void OnPreLaunchPawn(AFortPlayerPawn* Pawn, FVector& LaunchDir)
    {
        STATIC_FINDFUNCTION(OnPreLaunchPawn)

        Call(Function_OnPreLaunchPawn, Pawn, LaunchDir);
    }
    
    void MultiCastPushCannonLaunchedPlayer()
    {
        STATIC_FINDFUNCTION(MultiCastPushCannonLaunchedPlayer)

        Call(Function_MultiCastPushCannonLaunchedPlayer);
    }
};

class AFortAthenaJackalVehicle : public AFortAthenaVehicle
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaJackalVehicle");
};