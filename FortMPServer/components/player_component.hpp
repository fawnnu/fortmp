#pragma once

enum class EDamageCauser : uint8_t
{
    None,
    Pistol,
    SuppressedPistol,
    HandCannonPistol,
    PumpShotgun,
    TacticalShotgun,
    HeavyShotgun,
    Shotgun,
    AssaultRifle,
    HeavyAssaultRifle,
    InfantryAssaultRifle,
    SuppressedAssaultRifle,
    BurstAssaultRifle,
    Sniper,
    HuntingSniper,
    HeavySniper,
    BoltSniper,
    Grenade,
    Clinger,
    StinkGrenade,
    SMG,
    SuppressedSMG,
    GrenadeLauncher,
    LMG,
    Minigun,
    RPG,
    Pickaxe,
    Headshot,
    Jewel,
    C4,
    CannonBall
};

struct FWeaponDamageEntry
{
    FWeaponDamageEntry() = default;

    int Damage = 0;
    bool bFromAbove = false;
};

struct FSpecialDamageDealt
{
    FSpecialDamageDealt() = default;

    FWeaponDamageEntry JewelOpponent;
    FWeaponDamageEntry C4BuildingDamage;
};

struct FDamageDealtMapValue
{
    FDamageDealtMapValue() = default;

    int GetPlayerDamage()
    {
        return PistolDamage.Damage + ShotgunDamage.Damage + AssaultRifleDamage.Damage + SniperDamage.Damage + GrenadeDamage.Damage + SMGDamage.Damage + GrenadeLauncherDamage.Damage + RPGDamage.Damage + LMGDamage.Damage + MinigunDamage.Damage + PickaxeDamage.Damage;
    }

    FWeaponDamageEntry HandCannonPistolDamage;
    FWeaponDamageEntry PistolDamage;
    FWeaponDamageEntry PumpShotgunDamage;
    FWeaponDamageEntry TacticalShotgunDamage;
    FWeaponDamageEntry HeavyShotgunDamage;
    FWeaponDamageEntry ShotgunDamage;
    FWeaponDamageEntry AssaultRifleDamage;
    FWeaponDamageEntry BurstAssaultRifleDamage;
    FWeaponDamageEntry HeavyAssaultRifleDamage;
    FWeaponDamageEntry InfantryAssaultRifleDamage;
    FWeaponDamageEntry SuppressedAssaultRifleDamage;
    FWeaponDamageEntry SniperDamage;
    FWeaponDamageEntry BoltSniperDamage;
    FWeaponDamageEntry HeavySniperDamage;
    FWeaponDamageEntry HuntingSniperDamage;
    FWeaponDamageEntry GrenadeDamage;
    FWeaponDamageEntry ClingerDamage;
    FWeaponDamageEntry StinkGrenadeDamage;
    FWeaponDamageEntry SMGDamage;
    FWeaponDamageEntry GrenadeLauncherDamage;
    FWeaponDamageEntry RPGDamage;
    FWeaponDamageEntry SuppressedSMGDamage;
    FWeaponDamageEntry SuppressedPistolDamage;
    FWeaponDamageEntry MinigunDamage;
    FWeaponDamageEntry LMGDamage;
    FWeaponDamageEntry StructureDamage;
    FWeaponDamageEntry EnemyStructureDamage;
    FWeaponDamageEntry EnemyVehicleDamage;
    FWeaponDamageEntry SupplyDropBalloon;
    FWeaponDamageEntry HeadshotDamage;
    FWeaponDamageEntry PickaxeDamage;
    FWeaponDamageEntry CannonBallDamage;
    FSpecialDamageDealt SpecialDamage;
};

struct FPlayerStats
{
    FPlayerStats()
    {
        Eliminations = 0;
        Assists = 0;
        Revives = 0;
        DamageTaken = 0;
        DistanceTraveled = 0;
        LastPos = FVector(0, 0, 0);
        MaterialsGathered = 0;
        MaterialsUsed = 0;
        Hits = 0;
        HeadShots = 0;
        Placement = 0;
        TemporaryPlacedBuilds = 0;
        TemporaryMaterialsGathered = {0, 0, 0};
    }

    struct FMaterialSats
    {
        int Wood;
        int Stone;
        int Metal;
    };
    
    int Eliminations;
    int Assists;
    int Revives;
    int DamageTaken;
    int DistanceTraveled;
    FVector LastPos;
    int MaterialsGathered;
    int MaterialsUsed;
    int Hits;
    int HeadShots;
    int Placement;
    int TemporaryPlacedBuilds;
    FMaterialSats TemporaryMaterialsGathered;
    int TemporaryTrickPoints;
    int TemporaryObtainedShield;
    int TemporaryObtainedMushroomShield;
    int TemporaryObtainedMiniShield;
    int TemporaryObtainedBigShield;
    int TemporaryCampfireHealth;
};

enum EEraPlatform : uint8_t
{
    PC = 0,
    Android = 1,
    iOS = 2,
    Switch = 3
};

enum EEraMessageType : uint8_t
{
    Json = 0,
    Std = 1
};

struct FEraPlayerTrafficHistoryElement
{
    FEraPlayerTrafficHistoryElement() = default;
    
    FEraPlayerTrafficHistoryElement(std::string in_channel, std::string in_event_type, nlohmann::json in_json_parameters, int in_ref) :
        channel(in_channel), event_type(in_event_type), json_parameters(in_json_parameters), reference(in_ref)
    {
        message_type = Json;
    }
    
    FEraPlayerTrafficHistoryElement(std::string in_channel, std::string in_event_type, std::map<std::string, std::string> in_std_parameters, int in_ref) :
        channel(in_channel), event_type(in_event_type), std_parameters(in_std_parameters), reference(in_ref)
    {
        message_type = Std;
    }

    bool operator==(int in_reference)
    {
        return reference == in_reference;
    }
    
    std::string channel;
    std::string event_type;
    nlohmann::json json_parameters;
    std::map<std::string, std::string> std_parameters;
    EEraMessageType message_type;
    int reference;
};

// Struct that keeps the player's data Era needs in PlayerController
struct FEraPlayerInfo
{
    bool bStartedQueryProfileReq;
    nlohmann::json Profile;
    int ProfileFailures;
    int IllegalBuildingPlacements;
    bool bPendingKick;
    bool bProfileReady;
    std::vector<std::pair<nlohmann::json, UObject*>> PendingVariantsData;
    nlohmann::json PendingWrapsData;
    std::wstring PlayerPlatform;
    EEraPlatform PlayerPlatform_Enum;
    std::wstring AccountId;
    std::string sAccountId;
    std::string IP;
    FDamageDealtMapValue DamageDealtTracker;
    FPlayerStats PlayerStatsTracker;
    int PositionNotChangedCount;
    std::vector<UAthenaDanceItemDefinition*> OwnedEmotes;
    bool bLastKillHeadshot;
    bool bIsDisconneting;
    std::vector<FGameplayTag> CurrentPOIList;
    ABuildingGameplayActorSpawnMachine* CurrentSpawnMachine;
    int Competitive_PendingPayoutCurrency;
    bool bJustUsedGeyser;
    bool bJustLandedFromGeyser;

    // Keep track of everything sent to handle eventual connection losses.
    std::vector<FEraPlayerTrafficHistoryElement> MessageHistory;
    std::deque<FEraPlayerTrafficHistoryElement> MessageQueue;
    nlohmann::json JoinParameters; 
    bool bSentSummary;
    bool bTrafficLocked;
};

// For performance reasons, keep information about chest's items here
struct FEraContainerInfo
{
    struct LootInfo
    {
        LootInfo(UFortItemDefinition* InItemDefinition, int InCount)
        {
            ItemDefinition= InItemDefinition;
            Count = InCount;
        }
        LootInfo() = default;
        
        UFortItemDefinition* ItemDefinition;
        int Count;
    };
    
    std::vector<LootInfo> LootToSpawn;
    bool bPreInitialized;
};

// Struct that keeps information about a shotgun pellet. We need this data to make
struct FEraPelletInfo
{
    FEraPelletInfo()
    {
        CartridgeID = 0;
        WeaponInfo = 0;
        PawnToHit = nullptr;
        NumPelletsHit = 0;
    }
    
    int CartridgeID;
    int WeaponInfo;
    AFortPawn* PawnToHit;
    int NumPelletsHit;
};

class player_component
{
public:
    player_component();

    static void OnPawnLandedImplementation(AFortPlayerPawn* Self, FFrame& StackFrame);
    static void ClearJustLandedFromGeyser(AFortPlayerPawn* Self, FFrame& StackFrame);
    static void GatherDistanceTraveled(AFortPlayerControllerAthena* Self, FFrame& StackFrame);
    static void DelayedEndConnection(AFortPlayerControllerAthena* Self, FFrame& StackFrame);

    // Damage timers
    static void PistolDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void HandCannonPistolDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void PumpShotgunDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void TacticalShotgunDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void HeavyShotgunDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void ShotgunDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void HeavyAssaultRifleDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void InfantryAssaultRifleDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void AssaultRifleDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void BurstAssaultRifleDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void SuppressedAssaultRifleDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void SniperDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void HuntingSniperDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void HeavySniperDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void BoltSniperDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void GrenadeDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void StinkGrenadeDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void ClingerDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void SMGDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void GrenadeLauncherDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void RPGDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void SuppressedSMGDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void SuppressedPistolDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void MinigunDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void LMGDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void EnemyStructureDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void SupplyDropDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void EnemyVehicleDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void PickaxeDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void HeadshotDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void JewelDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void C4BuildingDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void CannonBallDamageCallback(AFortPlayerController* Self, FFrame& StackFrame);
    
    static void HandleDebouncedDamage(AFortPlayerController* Self, EDamageCauser DamageCauser);

    // Debounced buildings tracking
    static void BuildingPlacedCallback(AFortPlayerController* Self, FFrame& StackFrame);

    // Debounced harvesting tracking
    static void WoodResourceCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void StoneResourceCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void MetalResourceCallback(AFortPlayerController* Self, FFrame& StackFrame);

    // Debounced trick points tracking
    static void TrickPointsCallback(AFortPlayerController* Self, FFrame& StackFrame);

    // Health debounced events
    static void ShieldCallback(AFortPlayerController* Self, FFrame& StackFrame);
    // Shields debounced events
    static void MushroomShieldCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void MiniShieldCallback(AFortPlayerController* Self, FFrame& StackFrame);
    static void BigShieldCallback(AFortPlayerController* Self, FFrame& StackFrame);
    
    static void CampfireHealthCallback(AFortPlayerController* Self, FFrame& StackFrame);
    
    static bool Login(const FUniqueNetIdRepl& PlayerID, const FString& Options);
    static bool PlatformLogin(const FUniqueNetIdRepl& PlayerID, const FString& Options);
    static void PostLogin(AFortPlayerController* FortPlayerController);
    static void Setup(AFortPlayerController* NewPlayer, const FString& Options, UNetConnection* Connection);
    static bool is_live_spectator(APlayerController* player_controller);
    static void ApplyBackpack(AFortPlayerControllerAthena* NewPlayer);
    static void play_emote_item(AFortPlayerController* player_controller, UAthenaDanceItemDefinition* dance_asset, bool b_requires_validation = true);
    static AFortPlayerPawn* create_player(AFortPlayerController* player_controller, FTransform spawn_transform);
    static std::string get_account_id(AFortPlayerController* target_player_controller);
    static std::string get_account_id_unsafe(AFortPlayerController* target_player_controller);
    static std::string get_account_id_from_unique_id(FUniqueNetIdRepl& UniqueId);
    static bool is_source_tag_enabled(AFortPlayerController* controller, std::wstring Tag);
    static AFortPlayerController* get_player_by_account_id(std::wstring in_account_id);
    static AFortPlayerController* get_player_by_quest_manager(UFortQuestManager* quest_manager);
    static void handle_player_damaged(AFortPlayerPawn* HitActor, AFortPlayerController* Instigator, AActor* DamageCauser, int Damage, FName BoneName);
    static uint8_t convert_quick_chat_index_to_message(uint8_t Index);
    static void initialize_player_banner_icon(AFortPlayerController* TargetPlayerController, std::string BannerIcon);
    static void initialize_player_banner_level(AFortPlayerController* TargetPlayerController, int Level);
    static void initialize_player_banner_color(AFortPlayerController* TargetPlayerController, std::string BannerColor);
    static EDamageCauser get_damage_causer(FGameplayTagContainer& Tags);
    static void apply_variant(FFortAthenaLoadout* Loadout, nlohmann::json VariantData, UObject* ItemVariantIsUsedFor);
    static void apply_wraps(FFortAthenaLoadout* Loadout, nlohmann::json WrapsData);
    static void apply_bonus_pellets(UFortGameplayAbility_RangedWeapon* RangedWeaponAbility, FGameplayAbilityTargetDataHandle& InData);
    static bool get_bonus_pellet_data(UFortGameplayAbility_RangedWeapon* RangedWeaponAbility, FGameplayAbilityTargetDataHandle& InData, FEraPelletInfo& PelletInfo);
    
    static inline std::vector<std::wstring> EncryptionTokenList;
    static inline std::map<std::string, std::string> Matches;
    static inline std::map<std::string, std::string> Platforms;
    static inline int InvalidTokensReceived;
    static inline int16 global_player_id = 1;
    static inline int starting_players = 0;
    static inline void** HealthProperty;
};

namespace player_component_hooks
{
    // Called to set acknowledged pawn
    DEFINE_FUNCTION_HOOK(void, ServerAcknowledgePossession, (APlayerController* PlayerController, FFrame& StackFrame));
    
    // Called when a player joins the server
    DEFINE_FUNCTION_HOOK(void, PostLogin, (AFortGameModeAthena* FortGameModeAthena, APlayerController* NewPlayer));

    // Manipulated to override the hero type
    DEFINE_FUNCTION_HOOK(void, ApplyHomebaseEffectsOnPlayerSetup, (AFortGameState* Self, UObject* Unk, UFortMcpProfileAccount* Profile, UObject* ASCInterface, UObject* Hero, char A6, uint8_t A7));

    DEFINE_FUNCTION_HOOK(UObject*, GetOrInitializeHero, (AFortPlayerController* Controller));
    
    // Called when a player attempts to emote
    DEFINE_FUNCTION_HOOK(void, ServerPlayEmoteItem, (AFortPlayerController* FortPlayerController, FFrame& StackFrame));

    // Called when a player attempts to use a spray
    DEFINE_FUNCTION_HOOK(void, ServerPlaySprayItem, (AFortPlayerController* FortPlayerController, FFrame& StackFrame));
    
    // Called when a player attempts to emote
    DEFINE_FUNCTION_HOOK(void, MovingEmoteStopped, (AFortPawn* FortPawn));
    
    // Called when a player attempts to enter a ziplines
    DEFINE_FUNCTION_HOOK(void, ServerSendZiplineState, (AFortPlayerPawn* FortPlayerPawn, FFrame& StackFrame));

    // Called to set a CharacterPart, we hook it to null it
    DEFINE_FUNCTION_HOOK(void, ServerChoosePart, (AFortPlayerPawn*, FFrame&));
    
    // Called to set a different name, we hook it to null it
    DEFINE_FUNCTION_HOOK(void, ServerChangeName, (AFortPlayerController*, FFrame&));

    // Spawn Toy Instance
    DEFINE_FUNCTION_HOOK(AActor*, SpawnToyInstance, (AFortPlayerController* PlayerController, UObject* ToyClass, FTransform SpawnPosition));
    
    // Called when a player is disconnecting
    DEFINE_FUNCTION_HOOK(void, ServerReturnToMainMenu, (AFortPlayerController* FortPlayerController));

    // Called when a player is ready to drop the loading screen and play
    DEFINE_FUNCTION_HOOK(void, ServerReadyToStartMatch, (AFortPlayerController* FortPlayerController));
    
    // Called when a player has been spawned
    DEFINE_FUNCTION_HOOK(void, K2_OnRestartPlayer, (AFortGameMode* GameMode, AFortPlayerController* NewPlayer));

    // Called when a player attempts to create a map marker
    DEFINE_FUNCTION_HOOK(void, ServerAddMapMarker, (class UAthenaMarkerComponent* MarkerComponent, FFrame& StackFrame))

    // Reimplement hit-markers
    DEFINE_FUNCTION_HOOK(__int64, OnTargetDataReadyCallback, (UFortGameplayAbility_RangedWeapon* RangedWeaponAbility, FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag));

    // Reimplement quick chat
    DEFINE_FUNCTION_HOOK(void, ServerPlaySquadQuickChatMessage, (AFortPlayerControllerAthena* Self, FFrame& StackFrame));
    
    // OnDamageServer
    DEFINE_FUNCTION_HOOK(void, OnDamageServer, (UObject* Self, UObject* Function, void* Params));

    // Custom replication handling
    DEFINE_FUNCTION_HOOK(void, AFortPlayerStateAthena_PreReplication, (AFortPlayerStateAthena* Self, void*** ChangedPropertyTracker));

    // Track trick points
    DEFINE_FUNCTION_HOOK(void, ServerAwardVehicleTrickPoints, (AFortPlayerController* Self, FFrame& StackFrame))

    DEFINE_FUNCTION_HOOK(void, HandleStartingNewPlayer, (AGameModeBase* Self, FFrame& StackFrame))
    
    // Track shield for challenges
    DEFINE_FUNCTION_HOOK(void, GameplayCue_Shield_PotionConsumed, (UObject* Self, UObject* Function, void* Params));

    // Challenges
    DEFINE_FUNCTION_HOOK(void, GameplayCue, (UObject* Self, UObject* Function, void* Params));

    // Challenges
    DEFINE_FUNCTION_HOOK(void, ReceiveActorEndOverlap, (UObject* Self, UObject* Function, void* Params));
    
    // Reimplement function
    DEFINE_FUNCTION_HOOK(void, GiveItemToPlayer, (TScriptInterface<AFortPlayerController>* Interface, UFortItemDefinition* ItemDefinition, int Quantity))

    // Reimplement AGID anim bp functionality
    DEFINE_FUNCTION_HOOK(void, PushAnimBPOverride, (AFortPlayerPawn* Self, int Priority, UObject* AnimBPOverride))

    // Reimplement AGID anim bp functionality
    DEFINE_FUNCTION_HOOK(void, PopAnimBPOverride, (AFortPlayerPawn* Self, int Priority, UObject* AnimBPOverride))

    // Spray stuff
    DEFINE_FUNCTION_HOOK(void, AFortSprayDecalInstance_BeginPlay, (AFortSprayDecalInstance* Target));

    // For creative respawn bugs
    DEFINE_FUNCTION_HOOK(AActor*, AGameModeBase_FindPlayerStart_Implementation, (AGameModeBase* Self, AController* Controller, FString& IncomingName))
}

inline void (*UpdateZiplineState)(AFortPlayerPawn* FortPlayerPawn);
inline FFortWorldMarkerData* (*WorldMarkerDataCtr)(FFortWorldMarkerData*);
inline double (*GetPoiGridTagsForPawn)(AFortGameModeAthena*, AFortPawn*, FGameplayTagContainer&);