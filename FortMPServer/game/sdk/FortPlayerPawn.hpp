#pragma once

class AFortPlayerController;
class AFortWeapon;
class AFortPickup;

class AFortPawn : public AActor
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
};


class AFortPlayerPawn : public AFortPawn
{
public:
    DEFINE_PROPERTY(UObject*, Owner);

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
