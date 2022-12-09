#pragma once

class AFortWeapon : public AActor
{
public:
    static UObject* StaticClass()
    {
        static UObject* static_class = FindObject(L"/Script/FortniteGame.FortWeapon");

        return static_class;
    }
    
    DEFINE_PROPERTY(TScriptMulticastDelegate<AFortWeapon>, OnLocalAmmoChanged);
    DEFINE_PROPERTY(int, AmmoCount);
    DEFINE_PROPERTY(UFortItemDefinition*, WeaponData);
    DEFINE_PROPERTY(FGuid, ItemEntryGuid);

    int GetLocalMagazineAmmoCount()
    {
        STATIC_FINDFUNCTION(GetLocalMagazineAmmoCount)
        
        return Call<int>(Function_GetLocalMagazineAmmoCount);
    }
    
    int GetRemainingAmmo()
    {
        STATIC_FINDFUNCTION(GetRemainingAmmo)
        
        return Call<int>(Function_GetRemainingAmmo);
    }
};

struct FAthenaBatchedDamageGameplayCues_Shared
{
    DEFINE_STRUCT_PROPERTY(float, Magnitude);
    DEFINE_STRUCT_PROPERTY(float, NonPlayerMagnitude);

    DEFINE_STRUCT_PROPERTY(bool, bIsCritical);
        
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.AthenaBatchedDamageGameplayCues_Shared");
};

struct FAthenaBatchedDamageGameplayCues_NonShared
{
    DEFINE_STRUCT_PROPERTY(AActor*, HitActor);
    DEFINE_STRUCT_PROPERTY(AActor*, NonPlayerHitActor);
        
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.AthenaBatchedDamageGameplayCues_NonShared");
};

struct FAthenaBatchedDamageGameplayCues
{
    DEFINE_STRUCT_PROPERTY(float, Magnitude);
    DEFINE_STRUCT_PROPERTY(float, NonPlayerMagnitude);

    DEFINE_STRUCT_PROPERTY(bool, bIsCritical);

    DEFINE_STRUCT_PROPERTY(AActor*, HitActor);
    DEFINE_STRUCT_PROPERTY(AActor*, NonPlayerHitActor);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.AthenaBatchedDamageGameplayCues");
};