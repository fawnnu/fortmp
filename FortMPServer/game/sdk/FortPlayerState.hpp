#pragma once

class APlayerState : public AActor
{

};

class AFortPlayerState : public APlayerState
{
public:
    DEFINE_PROPERTY(class UFortHeroType*, HeroType);

    // Used on older builds
    DEFINE_PROPERTY(UCustomCharacterPart*, CharacterParts);

    // Used on newer builds
    DEFINE_PROPERTY(UCustomCharacterPart*, LocalCharacterParts);

    //IP
    DEFINE_PROPERTY(FString, SavedNetworkAddress);
};

struct FDeathInfo
{
    DEFINE_STRUCT_PROPERTY(AActor*, FinisherOrDowner);
    DEFINE_STRUCT_PROPERTY(bool, bDBNO);
    DEFINE_STRUCT_PROPERTY(uint8_t, DeathCause);
    DEFINE_STRUCT_PROPERTY(float, Distance);
    DEFINE_STRUCT_PROPERTY(FVector, DeathLocation);
    DEFINE_STRUCT_PROPERTY(bool, bInitialized);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.DeathInfo");
};

class AFortPlayerStateAthena : public AFortPlayerState
{
public:
    DEFINE_PROPERTY(uint8_t, TeamIndex);
    DEFINE_PROPERTY(uint8_t, SquadId);
    
    DEFINE_PROPERTY(int, KillScore);

    DEFINE_PROPERTY(FDeathInfo, DeathInfo);

    DEFINE_PROPERTY(int, NumRejoins);
    
    void ClientReportKill(AFortPlayerStateAthena* Player)
    {
        Call(L"ClientReportKill", Player);
    }

    uint8_t ToDeathCause(FGameplayTagContainer InTags, bool bWasDBNO)
    {
        return Call<uint8_t>(L"ToDeathCause", InTags, bWasDBNO);
    }
};
