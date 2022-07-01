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
