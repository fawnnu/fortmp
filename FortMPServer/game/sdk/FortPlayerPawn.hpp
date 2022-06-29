#pragma once

class AFortPawn : public AActor
{
public:
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
