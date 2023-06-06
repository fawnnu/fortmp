#pragma once

class challenges_component
{
public:
    challenges_component();

    static void SendChestEvent(AFortPlayerController* InteractingController, ABuildingContainer* TargetContainer);
    static void SendAmmoEvent(AFortPlayerController* InteractingController, ABuildingContainer* TargetContainer);
    static void SendLandingEvent(AFortPlayerController* OwnerPC, AFortPlayerPawn* Self);
    static void SendLlamaEvent(AFortPlayerController* InteractingController, AFortAthenaSupplyDrop* Llama);
};
