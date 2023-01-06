#pragma once

class interact_component
{
public:
    interact_component();

    static void handle_interaction(AFortPlayerController* player_controller, AActor* receiving_actor);

    static void handle_container_interaction(AFortPlayerController* player_controller, ABuildingContainer* building_container);
    static void handle_player_pawn_interaction(AFortPlayerController* player_controller, AFortPlayerPawn* player_pawn);
    static void handle_vending_machine_interaction(AFortPlayerController* player_controller, ABuildingItemCollectorActor* vending_machine);
    static void handle_vehicle_interaction(AFortPlayerController* player_controller, AFortAthenaVehicle* vehicle);
};

namespace interact_component_hooks
{
    // Called when a player interacts with an actor
    DEFINE_FUNCTION_HOOK(void, PlayerController_ServerAttemptInteract, (AFortPlayerController* PlayerController, FFrame& StackFrame));

    // Called when a player interacts with an actor
    DEFINE_FUNCTION_HOOK(void, InteractionComponent_ServerAttemptInteract, (UActorComponent* InteractionComponent, FFrame& StackFrame));
}
