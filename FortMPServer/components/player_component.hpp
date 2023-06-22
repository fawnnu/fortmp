#pragma once

enum class EDamageCauser : uint8_t
{
    None,
    Pistol,
    Shotgun,
    AssaultRifle,
    Sniper,
    Grenade,
    SMG,
    GrenadeLauncher,
    RPG
};

struct DamageDealtMapValue
{
    DamageDealtMapValue() = default;

    int PistolDamage;
    int ShotgunDamage;
    int AssaultRifleDamage;
    int SniperDamage;
    int GrenadeDamage;
    int SMGDamage;
    int GrenadeLauncherDamage;
    int RPGDamage;
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
    }
    
    int Eliminations;
    int Assists;
    int Revives;
    int DamageTaken;
    int DistanceTraveled;
    FVector LastPos;
    int MaterialsGathered;
    int MaterialsUsed;
};

class player_component
{
public:
    player_component();

    static void OnPawnLandedImplementation(AFortPlayerPawn* Self, FFrame& StackFrame);
    static void GatherDistanceTraveled(AFortPlayerControllerAthena* Self, FFrame& StackFrame);
    
    static bool Login(const FUniqueNetIdRepl& PlayerID, const FString& Options);
    static void PostLogin(AFortPlayerController* FortPlayerController);
    static void Setup(AFortPlayerController* NewPlayer, const FString& Options);
    static bool is_live_spectator(APlayerController* player_controller);
    static void ApplyBackpack(AFortPlayerControllerAthena* NewPlayer);
    static void play_emote_item(AFortPlayerController* player_controller, UAthenaDanceItemDefinition* dance_asset);
    static AFortPlayerPawn* create_player(AFortPlayerController* player_controller, FTransform spawn_transform);
    static std::string get_account_id(AFortPlayerController* target_player_controller);
    static AFortPlayerController* get_player_by_account_id(std::wstring in_account_id);
    static void handle_player_damaged(AFortPlayerPawn* HitActor, AFortPlayerController* Instigator, AActor* DamageCauser, float Damage);
    static uint8_t convert_quick_chat_index_to_message(uint8_t Index);
    static void initialize_player_banner_icon(AFortPlayerController* TargetPlayerController, std::string BannerIcon);
    static void initialize_player_banner_level(AFortPlayerController* TargetPlayerController, int Level);
    static void initialize_player_banner_color(AFortPlayerController* TargetPlayerController, std::string BannerColor);
    static EDamageCauser get_damage_causer(FGameplayTagContainer& Tags);
    
    static inline std::vector<std::wstring> EncryptionTokenList;
    static inline std::map<std::wstring, nlohmann::json> Profiles;
    static inline std::map<std::wstring, AFortPlayerController*> Players;
    static inline std::map<AFortPlayerController*, DamageDealtMapValue> DamageDealtMap;
    static inline std::map<AFortPlayerController*, FPlayerStats> PlayerStatsMap;
    static inline int InvalidTokensReceived;
    static inline int global_player_id = 0;
    static inline int starting_players = 0;
};

namespace player_component_hooks
{
    // Called to set acknowledged pawn
    DEFINE_FUNCTION_HOOK(void, ServerAcknowledgePossession, (APlayerController* PlayerController, FFrame& StackFrame));
    
    // Called when a player joins the server
    DEFINE_FUNCTION_HOOK(void, PostLogin, (AFortGameModeAthena* FortGameModeAthena, APlayerController* NewPlayer));

    // Called to override the hero type
    DEFINE_FUNCTION_HOOK(UObject*, GetHeroDefinition, (UObject* Hero));

    // Called when a player attempts to emote
    DEFINE_FUNCTION_HOOK(void, ServerPlayEmoteItem, (AFortPlayerController* FortPlayerController, FFrame& StackFrame));

    // Called when a player attempts to use a spray
    DEFINE_FUNCTION_HOOK(void, ServerPlaySprayItem, (AFortPlayerController* FortPlayerController, FFrame& StackFrame));
    
    // Called when a player attempts to emote
    DEFINE_FUNCTION_HOOK(void, MovingEmoteStopped, (AFortPawn* FortPawn, FFrame& StackFrame));
    
    // Called when a player attempts to enter a ziplines
    DEFINE_FUNCTION_HOOK(void, ServerSendZiplineState, (AFortPlayerPawn* FortPlayerPawn, FFrame& StackFrame));

    // Called to set a CharacterPart, we hook it to null it
    DEFINE_FUNCTION_HOOK(void, ServerChoosePart, (AFortPlayerPawn*, FFrame&));

    // Called when a player is disconnecting
    DEFINE_FUNCTION_HOOK(void, ServerReturnToMainMenu, (AFortPlayerController* FortPlayerController));

    // Called when a player is ready to drop the loading screen and play
    DEFINE_FUNCTION_HOOK(void, ServerReadyToStartMatch, (AFortPlayerController* FortPlayerController));
    
    // Called when a player has been spawned
    DEFINE_FUNCTION_HOOK(void, K2_OnRestartPlayer, (UObject* Object, UObject* Function, void* Params));

    // Called when a player attempts to create a map marker
    DEFINE_FUNCTION_HOOK(void, ServerAddMapMarker, (class UAthenaMarkerComponent* MarkerComponent, FFrame& StackFrame))

    // Reimplement hit-markers
    DEFINE_FUNCTION_HOOK(__int64, OnTargetDataReadyCallback, (UFortGameplayAbility_RangedWeapon* RangedWeaponAbility, FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag));

    // Reimplement quick chat
    DEFINE_FUNCTION_HOOK(void, ServerPlaySquadQuickChatMessage, (AFortPlayerControllerAthena* Self, FFrame& StackFrame));

    // OnDamageServer
    DEFINE_FUNCTION_HOOK(void, OnDamageServer, (UObject* Self, UObject* Function, void* Params));
}

inline void (*UpdateZiplineState)(AFortPlayerPawn* FortPlayerPawn);
inline FFortWorldMarkerData* (*WorldMarkerDataCtr)(FFortWorldMarkerData*);