#pragma once

class AFortWeapon : public UObject
{
public:
    DEFINE_PROPERTY(UObject*, WeaponData);
    DEFINE_PROPERTY(FGuid, ItemEntryGuid);
};
