#pragma once

class APlayerState : public AActor
{
public:
    DEFINE_PROPERTY(FUniqueNetIdRepl, UniqueId);
    DEFINE_PROPERTY(int, PlayerID);
};

class AFortPlayerState : public APlayerState
{
public:
    DEFINE_PROPERTY(class UFortHeroType*, HeroType);

    DEFINE_PROPERTY(FVector, PawnDeathLocation);
    
    // Used on older builds
    DEFINE_PROPERTY(UCustomCharacterPart*, CharacterParts);

    // Used on newer builds
    DEFINE_PROPERTY(UCustomCharacterPart*, LocalCharacterParts);

    DEFINE_PROPERTY(int, WorldPlayerId);

    DEFINE_PROPERTY(int, CachedPreviousWorldPlayerId);
    
    DEFINE_BIT_PROPERTY(bool, bHasStartedPlaying);
    
    //IP
    DEFINE_PROPERTY(FString, SavedNetworkAddress);

    void OnRep_CharacterData()
    {
        STATIC_FINDFUNCTION(OnRep_CharacterData)

        Call(Function_OnRep_CharacterData);
    }
    
    void ApplyCharacterCustomization(class AFortPlayerPawn* FortPlayerPawn)
    {
        static uintptr_t ApplyCharacterCustomization_Address = memory_manager::find_function_by_string(L"AFortPlayerState::ApplyCharacterCustomization");

        // If ApplyCharacterCustomization exists, use it
        if (ApplyCharacterCustomization_Address)
        {
            reinterpret_cast<void (*)(AFortPlayerState*, AFortPlayerPawn*)>(ApplyCharacterCustomization_Address)(this, FortPlayerPawn);
            
            return;
        }

        // TODO: Apply character parts ourselves if the native function does not exist
    }
};

struct FDeathInfo
{
    DEFINE_STRUCT_PROPERTY(AActor*, FinisherOrDowner);
    DEFINE_STRUCT_PROPERTY(bool, bDBNO);
    DEFINE_STRUCT_PROPERTY(uint8_t, DeathCause);
    DEFINE_STRUCT_PROPERTY(float, Distance);
    DEFINE_STRUCT_PROPERTY(FVector, DeathLocation);
    DEFINE_STRUCT_PROPERTY(bool, bInitialized);
    DEFINE_STRUCT_PROPERTY(FGameplayTagContainer, DeathTags);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.DeathInfo");
};

struct FPlayerBannerInfo
{
    DEFINE_STRUCT_PROPERTY(FString, IconId);
    DEFINE_STRUCT_PROPERTY(FString, ColorId);
    DEFINE_STRUCT_PROPERTY(int, Level);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.PlayerBannerInfo")
};

class AFortPlayerStateAthena : public AFortPlayerState
{
public:
    DEFINE_PROPERTY(uint8_t, TeamIndex);
    DEFINE_PROPERTY(uint8_t, SquadId);
    DEFINE_PROPERTY(int, KillScore);
    DEFINE_PROPERTY(bool, bShouldUpdateReplicatedPing);
    DEFINE_PROPERTY(uint8_t, Ping);
    DEFINE_PROPERTY(FUniqueNetIdRepl, PartyOwnerUniqueId);
    DEFINE_PROPERTY(FUniqueNetIdRepl, PlatformUniqueNetId);
    DEFINE_PROPERTY(UFortHeroType*, HeroType);
    DEFINE_PROPERTY(FDeathInfo, DeathInfo);
    DEFINE_PROPERTY(uint8_t, ReplicatedTeamMemberState);
    DEFINE_PROPERTY(uint8_t, TeamMemberState);
    DEFINE_PROPERTY(int, NumRejoins);
    DEFINE_PROPERTY(FPlayerBannerInfo, Banner);
    
    void OnRep_ReplicatedTeamMemberState()
    {
        STATIC_FINDFUNCTION(OnRep_ReplicatedTeamMemberState)

        if (Function_OnRep_ReplicatedTeamMemberState)
        {
            Call(Function_OnRep_ReplicatedTeamMemberState);
        }
    }

    void OnRep_SquadId()
    {
        STATIC_FINDFUNCTION(OnRep_SquadId)
        Call(Function_OnRep_SquadId);
    }
    
    void ClientReportKill(AFortPlayerStateAthena* Player)
    {
        STATIC_FINDFUNCTION(ClientReportKill)
        
        Call(Function_ClientReportKill, Player);
    }

    uint8_t ToDeathCause(FGameplayTagContainer InTags, bool bWasDBNO)
    {
        STATIC_FINDFUNCTION(ToDeathCause)
        
        return Call<uint8_t>(Function_ToDeathCause, InTags, bWasDBNO);
    }
};

class UFortHealthSet : public UObject
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortHealthSet");

    DEFINE_PROPERTY(FGameplayAttributeData, CurrentShield);
    void OnRep_CurrentShield()
    {
        STATIC_FINDFUNCTION(OnRep_CurrentShield);

        Call(Function_OnRep_CurrentShield);
    }
};