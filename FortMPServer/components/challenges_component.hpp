#pragma once

class challenges_component
{
public:
    challenges_component();

    static void SendChestEvent(AFortPlayerController* InteractingController, ABuildingContainer* TargetContainer);
    static void SendSupplyDropEvent(AFortPlayerController* InteractingController, AFortAthenaSupplyDrop* SupplyDrop);
    static void SendAmmoEvent(AFortPlayerController* InteractingController, ABuildingContainer* TargetContainer);
    static void SendLandingEvent(AFortPlayerController* OwnerPC, AFortPlayerPawn* Self);
    static void SendLlamaEvent(AFortPlayerController* InteractingController, AFortAthenaSupplyDrop* Llama);
    static void SendSummary(AFortPlayerController* PlayerController, int Placement);
    static void SendElimination(AFortPlayerController* KillerPlayerController, AFortPlayerController* VictimPlayerController, uint8_t DeathCause, FGameplayTagContainer GameplayTags);
    static void SendRevive(AFortPlayerController* ExecutingPlayerController, AFortPlayerController* VictimPlayerController);
    static void SendLetterInteraction(AFortPlayerController* InteractingController, AProp_QuestInteractable_Letter_Parent_C* Letter, FName Objective);
    static void SendGnomeInteraction(AFortPlayerController* InteractingController, AProp_Quest_GniceGnomes_Parent_C* Gnome, FName Objective);
    static void SendBattleStarInteraction(AFortPlayerController* InteractingController, AProp_QuestObject_Interactable_Parent_C* BattleStar, FName Objective);
    static void SendPortAFortInteraction(AFortPlayerController* InteractingController);
    static void SendHopRockInteraction(AFortPlayerController* InteractingController, ACBGA_LowGravity_UsingJump_C* HopRock);
    static void SendAppleInteraction(AFortPlayerController* InteractingController, ACBGA_HealthSmall_C* HealthSmall);
    static void SendMushroomInteraction(AFortPlayerController* InteractingController, ACBGA_ShieldsSmall_C* ShieldsSmall);
    static void SendStormCenterInteraction(AFortPlayerController* InteractingController, ABA_QuestInteractable_StormCenter_Single* StormCenter);
    static void SendDanceInteraction(AFortPlayerController* InteractingController);
    static void SendSprayInteraction(AFortPlayerController* InteractingController);
    static void SendRubberDuckInteraction(AFortPlayerController* InteractingController, AProp_QuestInteractable_RubberDuckies_Parent_C* RubberDuck, FName Objective);
    static void SendVendingMachineInteraction(AFortPlayerController* InteractingController, AB_Athena_VendingMachine_C* VendingMachine);
    static void SendDiscoBallInteraction(AFortPlayerController* InteractingController);
    static void SendPitchGoalInteraction(AFortPlayerController* InteractingController, FName BackendObjectiveName);

    static void HandleSummaryReply(std::string Channel, nlohmann::json Payload);
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
}
