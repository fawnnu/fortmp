#pragma once

class AFortPlayerController;
class AFortWeapon;

class AFortPawn : public AActor
{
public:
    DEFINE_PROPERTY(AFortPlayerController*, Controller);
    DEFINE_PROPERTY(AFortWeapon*, CurrentWeapon);
    
    UObject* EquipWeaponDefinition(UObject* WeaponData, FGuid ItemEntryGuid)
    {
        return Call<UObject*>(L"EquipWeaponDefinition", WeaponData, ItemEntryGuid);
    }
};


class AFortPlayerPawn : public AFortPawn
{
public:
    DEFINE_PROPERTY(UObject*, Owner);
};

class AFortAIPawn : public UObject
{
public:
    void AddFortAbilitySet(UObject* FortAbilitySet)
    {
        Call(L"/Script/FortniteGame.FortAIPawn.AddFortAbilitySet", FortAbilitySet);
    }
};
