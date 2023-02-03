#pragma once

class AFortGameplayMutator : public AActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortGameplayMutator");
    DEFINE_BIT_PROPERTY(bool, bMutatorActive);
};

class AFortAthenaMutator : public AFortGameplayMutator
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaMutator");
};

class AFortAthenaMutator_InventoryOverride : public AFortAthenaMutator
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaMutator_InventoryOverride")

    DEFINE_PROPERTY(uint8, DropAllItemsOverride);
    DEFINE_PROPERTY(uint8, TrapDropOverride);
    DEFINE_PROPERTY(uint8, WeaponDropOverride);
    DEFINE_PROPERTY(uint8, MaterialDropOverride);
    DEFINE_PROPERTY(uint8, GadgetDropOverride);
    DEFINE_PROPERTY(uint8, ConsumableDropOverride);
    DEFINE_PROPERTY(uint8, AmmoDropOverride);
};