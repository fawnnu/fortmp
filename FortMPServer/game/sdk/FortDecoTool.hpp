#pragma once

class UFortDecoItemDefinition : public UFortWorldItemDefinition
{
public:
    DEFINE_PROPERTY(TSoftObjectPtr, BlueprintClass);
    DEFINE_PROPERTY(TSoftObjectPtr, PlacementPreviewClass);
};

class AFortDecoTool : public AFortWeapon
{
public:
    DEFINE_PROPERTY(UFortDecoItemDefinition*, ItemDefinition);
};

class ABuildingTrap : public ABuildingSMActor
{
public:
    DEFINE_PROPERTY(UObject*, AbilitySet);
    DEFINE_PROPERTY(ABuildingSMActor*, AttachedTo);

    void OnRep_AttachedTo()
    {
        Call(L"OnRep_AttachedTo");
    }
};

class AFortDecoTool_ContextTrap : public UObject
{
public:
    DEFINE_PROPERTY(UFortItemDefinition*, ContextTrapItemDefinition);
};