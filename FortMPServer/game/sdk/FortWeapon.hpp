#pragma once

class AFortWeapon : public AActor
{
public:
    DEFINE_PROPERTY(UFortItemDefinition*, WeaponData);
    DEFINE_PROPERTY(FGuid, ItemEntryGuid);

    int GetMagazineAmmoCount()
    {
        return Call<int>(L"GetMagazineAmmoCount");
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