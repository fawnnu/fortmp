#pragma once

class UFortDecoItemDefinition : public UFortWorldItemDefinition
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortDecoItemDefinition");
    DEFINE_PROPERTY(TSoftObjectPtr<UObject>, BlueprintClass);
    DEFINE_PROPERTY(TSoftObjectPtr<UObject>, PlacementPreviewClass);
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
        STATIC_FINDFUNCTION(OnRep_AttachedTo)
        
        Call(Function_OnRep_AttachedTo);
    }
};

class AFortDecoTool_ContextTrap : public AFortDecoTool
{
public:
    DEFINE_PROPERTY(UFortItemDefinition*, ContextTrapItemDefinition);

    void SetContextTrapItemDefinition(UFortItemDefinition* InContextTrapItemDefinition)
    {
        STATIC_FINDFUNCTION(SetContextTrapItemDefinition)
        
        Call(Function_SetContextTrapItemDefinition, InContextTrapItemDefinition);
    }
};