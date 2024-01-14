#pragma once

class interact_component
{
public:
    interact_component();

    static void handle_interaction(AFortPlayerController* player_controller, AActor* receiving_actor, UPrimitiveComponent* interact_component, uint8_t interaction_type);

    static void handle_container_interaction(AFortPlayerController* player_controller, ABuildingContainer* building_container);
    static void handle_player_pawn_interaction(AFortPlayerController* player_controller, AFortPlayerPawn* player_pawn);
    static void handle_vending_machine_interaction(AFortPlayerController* player_controller, ABuildingItemCollectorActor* vending_machine);
    static void handle_vehicle_interaction(AFortPlayerController* player_controller, AFortAthenaVehicle* vehicle);
    static void handle_shoppingcart_interaction(AFortPlayerController* player_controller, AShoppingCartVehicleSK* vehicle);
    static void handle_plane_interaction(AFortPlayerController* player_controller, AFortAthenaFerretVehicle* vehicle);
    static void handle_llama_interaction(AFortPlayerController* player_controller, AFortAthenaSupplyDrop* supply_drop);
    static void handle_supplydrop_interaction(AFortPlayerController* player_controller, AFortAthenaSupplyDrop* supply_drop);
    static void handle_letter_interaction(AFortPlayerController* player_controller, AProp_QuestInteractable_Letter_Parent_C* letter);
    static void handle_battlestar_interaction(AFortPlayerController* player_controller, AProp_QuestObject_Interactable_Parent_C* battlestar);
    static void handle_battlestar_interaction(AFortPlayerController* player_controller, AProp_QuestInteractable_HiddenStar_Parent_C* battlestar);
    static void handle_gnome_interaction(AFortPlayerController* player_controller, AProp_Quest_GniceGnomes_Parent_C* gnome);
    static void handle_hoprock_interaction(AFortPlayerController* player_controller, ACBGA_LowGravity_UsingJump_C* hoprock);
    static void handle_apple_interaction(AFortPlayerController* player_controller, ACBGA_HealthSmall_C* apple);
    static void handle_mushroom_interaction(AFortPlayerController* player_controller, ACBGA_ShieldsSmall_C* mushroom);
    static void handle_rubberduck_interaction(AFortPlayerController* player_controller, AProp_QuestInteractable_RubberDuckies_Parent_C* rubberduck);
    static void handle_gooseegg_interaction(AFortPlayerController* player_controller, AProp_FMP_GooseEgg_Interactable* goose_egg);
    static void handle_lightningbolt_interaction(AFortPlayerController* player_controller, AProp_QuestActor_FloatingLightningBolt_Parent_C* lightningbolt);
    static void handle_jigsaw_puzzle_interaction(AFortPlayerController* player_controller, AProp_QuestInteractable_Jigsaw_Parent* jigsawpuzzle);
    static void handle_shadowstone_interaction(AFortPlayerController* player_controller, ACBGA_SpookyMist_C* spookymist);
    static void handle_doorbell_interaction(AFortPlayerController* player_controller, AProp_Athena_Doorbell_Interactable_C* doorbell);
    static void handle_noms_interaction(AFortPlayerController* player_controller, AProp_QuestInteractable_Noms_Base* noms);
    static void handle_fireworks_interaction(AFortPlayerController* player_controller, ABP_FrozenFireworkLauncher_Parent* firework);
};

namespace interact_component_hooks
{
    // Called when a player interacts with an actor
    DEFINE_FUNCTION_HOOK(void, PlayerController_ServerAttemptInteract, (AFortPlayerController* PlayerController, FFrame& StackFrame));

    // Called when a player interacts with an actor
    DEFINE_FUNCTION_HOOK(void, InteractionComponent_ServerAttemptInteract, (UActorComponent* InteractionComponent, FFrame& StackFrame));
}
