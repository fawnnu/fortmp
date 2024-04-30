#pragma once

#define CHECK_PAST_WARMUP()\
    static UEnum* GamePhaseEnum = FindObject<UEnum>(TEXT("/Script/FortniteGame.EAthenaGamePhase"));\
    static uint8_t Warmup = GamePhaseEnum->GetEnumIndex(TEXT("Warmup"));\
    \
    if (*GWorld)\
    {\
    if ((*GWorld)->GameState())\
    {\
    if ((*GWorld)->GameState()->GamePhase() == Warmup)\
    {\
    return;\
    }\
    }\
    }\
    
#define CHECK_CREATIVE()\
    \
    if (creative_component::is_creative_enabled())\
    {\
    return;\
    }\
    

class challenges_component
{
public:
    challenges_component();
    static void Initialize();
    
    static void SendSummary(AFortPlayerController* PlayerController, int Placement);
    static void SendElimination(AFortPlayerController* KillerPlayerController, AFortPlayerController* VictimPlayerController, uint8_t DeathCause, FGameplayTagContainer GameplayTags, std::string rarity);
    static void SendRevive(AFortPlayerController* ExecutingPlayerController, AFortPlayerController* VictimPlayerController);
    static void SendDanceInteraction(AFortPlayerController* InteractingController, UAthenaDanceItemDefinition* EmoteAsset);
    static void SendPOIInteraction(AFortPlayerController* InteractingController, FName BackendObjectiveName);
    static void HandleItemAdded(AFortPlayerController* InteractingPlayerController, UFortItemDefinition* ItemDefinition);
    static void SendCustomEvent(AFortPlayerController* InteractingController, std::string event_type);
    static void SendCustomEventWithObjective(AFortPlayerController* InteractingController, std::string event_type, FName ObjBackendName);
    static void SendCustomEventWithCount(AFortPlayerController* InteractingController, std::string event_type, int Count);
    static void SendPickupInteraction(AFortPlayerController* InteractingPlayerController, AFortPickup* Pickup);
    static void SendVisitInteraction(AFortPlayerController* InteractingPlayerController, FGameplayTag POI);
    static void SendTomatoInteraction(AFortPlayerController* InteractingPlayerController, float distance);
    static void SendGrenadeHitInteraction(AFortPlayerControllerAthena* InteractingPlayerController,std::string type);
    static void SendPlaceEvent(AFortPlayerController* InteractingPlayerController, std::string Type);
    static void SendHealInteraction(AFortPlayerControllerAthena* InteractingPlayerController, std::string source, int count);
    
    static void HandleSummaryReply(std::string Channel, nlohmann::json Payload);

    static inline std::map<UFortQuestItemDefinition*, std::string> QuestToInteractionMatch;
    static inline std::map<std::vector<FGameplayTag>, std::string> DestroyTagsToInteractionMatch;
};

struct progression_meta
{
    bool assisted;
    std::string assisted_by;
};

struct progression_entry
{
    std::string challenge;
    std::string objective;
    int value;
    int delta;
    int stage;
    progression_meta meta;
    std::string player;
};

namespace challenges_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, Rising__FinishedFunc, (UObject* Object, UObject* Function, void* Params))
    DEFINE_FUNCTION_HOOK(void, BndEvt__Goal_02_Overlap_K2Node_ComponentBoundEvent_0_ComponentBeginOverlapSignature__DelegateSignature, (UObject* Object, UObject* Function, void* Params))
    DEFINE_FUNCTION_HOOK(void, BndEvt__Goal_01_Overlap_K2Node_ComponentBoundEvent_1_ComponentBeginOverlapSignature__DelegateSignature, (UObject* Object, UObject* Function, void* Params))
    DEFINE_FUNCTION_HOOK(void, BndEvt__HoopTriggerBox_K2Node_ComponentBoundEvent_0_ComponentBeginOverlapSignature__DelegateSignature, (UObject* Object, UObject* Function, void* Params))
    DEFINE_FUNCTION_HOOK(void, PidgeonShot, (UObject* Object, UObject* Function, void* Params))
    DEFINE_FUNCTION_HOOK(void, KillPuck, (UObject* Object, UObject* Function, void* Params))
    DEFINE_FUNCTION_HOOK(void, BndEvt__HoleCollision_K2Node_ComponentBoundEvent_0_ComponentBeginOverlapSignature__DelegateSignature, (UObject* Object, UObject* Function, void* Params));
    DEFINE_FUNCTION_HOOK(void, Athena_GiveQuestUpdateToPlayers, (UObject* Context, FFrame& StackFrame, void* Param))
}
