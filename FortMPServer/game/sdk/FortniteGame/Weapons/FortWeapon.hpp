#pragma once

class AFortWeapon : public AActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortWeapon")
    
    DEFINE_PROPERTY(TScriptMulticastDelegate<AFortWeapon>, OnLocalAmmoChanged);
    DEFINE_PROPERTY(int, AmmoCount);
    DEFINE_PROPERTY(UFortItemDefinition*, WeaponData);
    DEFINE_PROPERTY(FGuid, ItemEntryGuid);

    float GetRange()
    {
        STATIC_FINDFUNCTION(GetRange)

        return Call<float>(Function_GetRange);
    }

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

    bool IsProjectileWeapon()
    {
        STATIC_FINDFUNCTION(IsProjectileWeapon)
        
        return Call<bool>(Function_IsProjectileWeapon);
    }

    bool IsSuppressed()
    {
        STATIC_FINDFUNCTION(IsSuppressed)

        if (Function_IsSuppressed)
        {
            return Call<bool>(Function_IsSuppressed);
        }

        return false;
    }

    float GetFiringRate()
    {
        STATIC_FINDFUNCTION(GetFiringRate)

        return Call<float>(Function_GetFiringRate);
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

struct FMountedWeaponInfoRepped
{
    DEFINE_STRUCT_PROPERTY(AActor*, HostVehicleCachedActor);
    DEFINE_STRUCT_PROPERTY(int, HostVehicleSeatIndexCached);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.MountedWeaponInfoRepped");
};

class AFortWeaponRanged : public AFortWeapon
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortWeaponRanged")
};

class AFortWeaponRangedForVehicle : public AFortWeapon
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortWeaponRangedForVehicle");

    DEFINE_PROPERTY(FMountedWeaponInfoRepped, MountedWeaponInfoRepped);

    void OnHostVehicleSetup()
    {
        STATIC_FINDFUNCTION(OnHostVehicleSetup)

        Call(Function_OnHostVehicleSetup);
    }
    void OnRep_MountedWeaponInfoRepped()
    {
        STATIC_FINDFUNCTION(OnRep_MountedWeaponInfoRepped)

        Call(Function_OnRep_MountedWeaponInfoRepped);
    }
};

class AFortWeaponRangedDualForVehicle : public AFortWeapon
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortWeaponRangedDualForVehicle");

    DEFINE_PROPERTY(FMountedWeaponInfoRepped, MountedWeaponInfoRepped);

    void OnHostVehicleSetup()
    {
        STATIC_FINDFUNCTION(OnHostVehicleSetup)

        Call(Function_OnHostVehicleSetup);
    }
    void OnRep_MountedWeaponInfoRepped()
    {
        STATIC_FINDFUNCTION(OnRep_MountedWeaponInfoRepped)

        Call(Function_OnRep_MountedWeaponInfoRepped);
    }
};

class AFortProjectileAthena : public AActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortProjectileAthena");

    DEFINE_PROPERTY(FVector, FireStartLoc);
};