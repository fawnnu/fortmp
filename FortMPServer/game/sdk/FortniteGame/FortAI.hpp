#pragma once

class UFortAthenaAIBotCharacterCustomization : public UObject
{
public:
    DEFINE_PROPERTY(FFortAthenaLoadout, CustomizationLoadout);
};

class UFortAthenaAIBotInventoryItems : public UObject
{
public:
    // Changed to FItemAndCount after like s13!
    DEFINE_PROPERTY(TArray<UFortItemDefinition*>, Items);
};

class UFortAthenaAIBotCustomizationData : public UObject
{
public:

    DEFINE_PROPERTY(class AFortPlayerPawnAthena*, PawnClass);
    DEFINE_PROPERTY(class UFortAthenaAIBotCharacterCustomization*, CharacterCustomization);
    DEFINE_PROPERTY(class UFortAthenaAIBotInventoryItems*, StartupInventory);
};

class UFortServerBotManagerAthena : public UObject
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortServerBotManagerAthena")

    DEFINE_PROPERTY(class AFortGameModeAthena*, CachedGameMode);
    DEFINE_PROPERTY(class AFortGameStateAthena*, CachedGameState);
    DEFINE_PROPERTY(class AFortAthenaMutator_Bots*, CachedBotMutator);
};
