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

    static void SendChestEvent(AFortPlayerController* InteractingController, ABuildingContainer* TargetContainer);
    static void SendSupplyDropEvent(AFortPlayerController* InteractingController, AFortAthenaSupplyDrop* SupplyDrop);
    static void SendAmmoEvent(AFortPlayerController* InteractingController, ABuildingContainer* TargetContainer);
    static void SendLandingEvent(AFortPlayerController* OwnerPC, AFortPlayerPawn* Self);
    static void SendLlamaEvent(AFortPlayerController* InteractingController, AFortAthenaSupplyDrop* Llama);
    static void SendSummary(AFortPlayerController* PlayerController, int Placement);
    static void SendElimination(AFortPlayerController* KillerPlayerController, AFortPlayerController* VictimPlayerController, uint8_t DeathCause, FGameplayTagContainer GameplayTags, std::string rarity);
    static void SendRevive(AFortPlayerController* ExecutingPlayerController, AFortPlayerController* VictimPlayerController);
    static void SendLetterInteraction(AFortPlayerController* InteractingController, AActor* Letter, FName Objective);
    static void SendGnomeInteraction(AFortPlayerController* InteractingController, AProp_Quest_GniceGnomes_Parent_C* Gnome, FName Objective);
    static void SendBattleStarInteraction(AFortPlayerController* InteractingController, AProp_QuestObject_Interactable_Parent_C* BattleStar, FName Objective);
    static void SendBattleStarInteraction(AFortPlayerController* InteractingController, AProp_QuestInteractable_HiddenStar_Parent_C* BattleStar, FName Objective);
    static void SendLocationInteraction(AFortPlayerController* InteractingController, AProp_MountainSummit_QuestInteractable_Parent* LocationProp, FName Objective);
    static void SendSkyDiveRingInteraction(AFortPlayerController* InteractingController, AProp_SkydiveRings_QuestInteractable* SkydiveRing);
    static void SendLaunchPadInteraction(AFortPlayerController* InteractingController, ATrap_Floor_Player_Launch_Pad_C* LaunchPad);
    static void SendPortAFortInteraction(AFortPlayerController* InteractingController);
    static void SendHopRockInteraction(AFortPlayerController* InteractingController, ACBGA_LowGravity_UsingJump_C* HopRock);
    static void SendAppleInteraction(AFortPlayerController* InteractingController, ACBGA_HealthSmall_C* HealthSmall);
    static void SendMushroomInteraction(AFortPlayerController* InteractingController, ACBGA_ShieldsSmall_C* ShieldsSmall);
    static void SendStormCenterInteraction(AFortPlayerController* InteractingController, ABA_QuestInteractable_StormCenter_Single* StormCenter);
    static void SendDanceInteraction(AFortPlayerController* InteractingController);
    static void SendCandyCaneVisitInteraction(AFortPlayerController* InteractingController, FName CurrentEraPOI);
    static void SendSprayInteraction(AFortPlayerController* InteractingController);
    static void SendRubberDuckInteraction(AFortPlayerController* InteractingController, AProp_QuestInteractable_RubberDuckies_Parent_C* RubberDuck, FName Objective);
    static void SendLightningBoltInteraction(AFortPlayerController* InteractingController, AProp_QuestActor_FloatingLightningBolt_Parent_C* LightningBolt, FName Objective);
    static void SendVendingMachineInteraction(AFortPlayerController* InteractingController, AB_Athena_VendingMachine_C* VendingMachine);
    static void SendDiscoBallInteraction(AFortPlayerController* InteractingController);
    static void SendShoppingCartInteraction(AFortPlayerController* InteractingController);
    static void SendPitchGoalInteraction(AFortPlayerController* InteractingController, FName BackendObjectiveName);
    static void SendBasketBallHoopInteraction(AFortPlayerController* InteractingController, FName BackendObjectiveName);
    static void SendClayPidgeonInteraction(AFortPlayerController* InteractingController, FName BackendObjectiveName);
    static void SendGoldenRingBiplaneInteraction(AFortPlayerController* InteractingController, Afortmp_golden_ring_quest* SkydiveRing, FName Objective);
    static void SendPOIInteraction(AFortPlayerController* InteractingController, FName BackendObjectiveName);
    static void SendJewelInteraction(AFortPlayerController* InteractingController);
    static void HandleItemAdded(AFortPlayerController* InteractingPlayerController, UFortItemDefinition* ItemDefinition);
    static void SendFlamingHoopInteraction(AFortPlayerController* InteractingController, AActor_QuestObject_Touch_FlamingHoops_Parent* FlamingHoop, FName Objective);
    static void SendGooseEggInteraction(AFortPlayerController* InteractingController, AProp_FMP_GooseEgg_Interactable* GooseEgg, FName Objective);
    static void SendGolfInteraction(AFortPlayerController* InteractingController, FName BackendObjectiveName);
    static void SendRiftInteraction(AFortPlayerController* InteractingController, ABGA_RiftPortal_Athena* RiftPortal);
    static void SendTrapInteraction(AFortPlayerController* InteractingController);
    static void SendJigsawPuzzleInteraction(AFortPlayerController* InteractingController, AProp_QuestInteractable_Jigsaw_Parent* JigsawPuzzle, FName Objective);
    static void SendSpotlightDanceInteraction(AFortPlayerController* InteractingController, AStreet_Light_Spotlight_AthenaQuest_C* StreetLight, FName Objective);
    static void SendPickupInteraction(AFortPlayerController* InteractingPlayerController, AFortPickup* Pickup);
    static void SendCorruptedAreaInteraction(AFortPlayerController* InteractingPlayerController, FGameplayTag POI);
    static void SendNomsSignInteraction(AFortPlayerController* InteractingPlayerController, FGameplayTag POI);
    static void SendSpookyMistInteraction(AFortPlayerController* InteractingPlayerController);
    static void SendTomatoInteraction(AFortPlayerController* InteractingPlayerController, float distance);
    static void SendSnowballInteraction(AFortPlayerController* InteractingPlayerController);
    static void SendDoorbellInteraction(AFortPlayerController* InteractingPlayerController);
    static void SendDoorbellPOIInteraction(AFortPlayerController* InteractingPlayerController, FName Objective);
    static void SendThankBusDriverInteraction(AFortPlayerControllerAthena* InteractingPlayerController);
    static void SendZiplineInteraction(AFortPlayerControllerAthena* InteractingPlayerController);
    static void SendPlaneInteraction(AFortPlayerControllerAthena* InteractingPlayerController);
    static void SendBoogieBombInteraction(AFortPlayerControllerAthena* InteractingPlayerController);
    static void SendFireworksInteraction(AFortPlayerControllerAthena* InteractingPlayerController, ABP_FrozenFireworkLauncher_Parent* firework, FName Objective);
    static void SendIcePuckInteraction(AFortPlayerControllerAthena* InteractingPlayerController);
    
    static void HandleSummaryReply(std::string Channel, nlohmann::json Payload);

    static inline std::map<UFortQuestItemDefinition*, std::string> QuestToInteractionMatch;
    static inline std::map<std::vector<FGameplayTag>, std::string> DestroyTagsToInteractionMatch;
};

struct progression_entry
{
    std::string challenge;
    std::string objective;
    int value;
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
