#pragma once

class APlayerState : public AActor
{
public:
    DEFINE_PROPERTY(FUniqueNetIdRepl, UniqueId);
    DEFINE_PROPERTY(int, PlayerID);
    DEFINE_PROPERTY(APawn*, PawnPrivate);
};

class AFortPlayerState : public APlayerState
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortPlayerState")
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

    void ApplyCharacterCustomizationNonNative(class AFortPlayerPawn* FortPlayerPawn)
    {
        // TODO; Apply variants eventually
        
        if (FindPropertyByName(FFortAthenaLoadout::GetClass(), TEXT("Character")))
        {
            auto NewPlayer = FortPlayerPawn->GetController()->Cast<AFortPlayerControllerAthena>();

            if (!NewPlayer) return;
            
            if (NewPlayer->CustomizationLoadout().Character())
            {
                for (auto part : NewPlayer->CustomizationLoadout().Character()->HeroDefinition()->Specializations().Data->Get()->CharacterParts())
                {
                    NewPlayer->Pawn()->ServerChoosePart(part.Get()->CharacterPartType(), part.Get());
                }
            }
        }
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
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortPlayerStateAthena");
    DEFINE_PROPERTY(uint8_t, TeamIndex);
    DEFINE_PROPERTY(uint8_t, SquadId);
    DEFINE_PROPERTY(int, KillScore);
    DEFINE_PROPERTY(int, TeamKillScore);
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
    DEFINE_PROPERTY(int, TotalPlayerScore);
    DEFINE_PROPERTY(bool, bResurrectingNow);
    DEFINE_PROPERTY(int, RebootCounter);
    DEFINE_PROPERTY(FFortResurrectionData, ResurrectionChipAvailable);

    void InitializeActiveTeam()
    {
        // Don't do anything below season 8.
        if (GetFortniteVersionEasy() < 8.0) return;
        
        auto GameState = (*GWorld)->GameState();
        if (!GameState) return;

        auto Pad = GameState->GetTeamsArrayContainer();

        if (!Pad)
        {
            spdlog::info(L"AFortPlayerState::InitializeActiveTeam(): Not initializing active team array! Array not found.");
            return;
        }

        // CREDIT: MILXNOR
        TWeakObjectPtr<AFortPlayerStateAthena> WeakPlayerState = this;

        if (Pad->TeamsArray.Num() > 0 && Pad->TeamsArray.IsIndexValid(TeamIndex()))
        {
            auto& TeamArray = Pad->TeamsArray[TeamIndex()];
            if (TeamArray.ElementData.Num() <= 0)
                TeamArray.ElementData.Reset();

            TeamArray.ElementData.Add(WeakPlayerState);
        }
        else
        {
            Pad->TeamsArray.Reset();
            for (int i = 0; i < 100; i++)
                Pad->TeamsArray.Add({});

            auto& TeamArray = Pad->TeamsArray[TeamIndex()];
            if (TeamArray.ElementData.Num() <= 0)
                TeamArray.ElementData.Reset();

            TeamArray.ElementData.Add(WeakPlayerState);
        }

        if (Pad->SquadsArray.Num() > 0 && Pad->SquadsArray.IsIndexValid(SquadId()))
        {
            auto& SquadArray = Pad->SquadsArray[SquadId()];
            if (SquadArray.ElementData.Num() <= 0)
                SquadArray.ElementData.Reset();

            SquadArray.ElementData.Add(WeakPlayerState);
        }
        else
        {
            Pad->SquadsArray.Reset();
            for (int i = 0; i < 100; i++)
                Pad->SquadsArray.Add({});

            auto& SquadArray = Pad->SquadsArray[SquadId()];
            if (SquadArray.ElementData.Num() <= 0)
                SquadArray.ElementData.Reset();

            SquadArray.ElementData.Add(WeakPlayerState);
        }
    }
    
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

    void OnRep_RebootCounter()
    {
        STATIC_FINDFUNCTION(OnRep_RebootCounter)
        Call(Function_OnRep_RebootCounter);
    }

    void OnRep_TeamKillScore()
    {
        STATIC_FINDFUNCTION(OnRep_TeamKillScore)
        Call(Function_OnRep_TeamKillScore);
    }

    void OnRep_Kills()
    {
        STATIC_FINDFUNCTION(OnRep_Kills)
        Call(Function_OnRep_Kills);
    }

    void ClientReportTeamKill(int TeamKills)
    {
        STATIC_FINDFUNCTION(ClientReportTeamKill)
        Call(Function_ClientReportTeamKill, TeamKills);
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