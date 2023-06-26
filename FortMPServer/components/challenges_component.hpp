#pragma once

class challenges_component
{
public:
    challenges_component();

    static void SendChestEvent(AFortPlayerController* InteractingController, ABuildingContainer* TargetContainer);
    static void SendAmmoEvent(AFortPlayerController* InteractingController, ABuildingContainer* TargetContainer);
    static void SendLandingEvent(AFortPlayerController* OwnerPC, AFortPlayerPawn* Self);
    static void SendLlamaEvent(AFortPlayerController* InteractingController, AFortAthenaSupplyDrop* Llama);
    static void SendSummary(AFortPlayerController* PlayerController, int Placement);
    static void SendElimination(AFortPlayerController* KillerPlayerController, AFortPlayerController* VictimPlayerController, uint8_t DeathCause);
    static void SendLetterInteraction(AFortPlayerController* InteractingController, AProp_QuestInteractable_Letter_Parent_C* Letter, FName Objective);
    static void SendBattleStarInteraction(AFortPlayerController* InteractingController, AProp_QuestObject_Interactable_Parent_C* BattleStar, FName Objective);
    static void SendPortAFortInteraction(AFortPlayerController* InteractingController);
    static void SendHopRockInteraction(AFortPlayerController* InteractingController, ACBGA_LowGravity_UsingJump_C* HopRock);

    static void HandleSummaryReply(std::string Channel, nlohmann::json Payload);
};

struct progression_entry
{
    std::string challenge;
    std::string objective;
    int value;
    std::string player;
};
