#include "include.hpp"
#include "interact_component.hpp"

#include "looting_component.hpp"
#include "pickup_component.hpp"
#include "reviving_component.hpp"

interact_component::interact_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(interact_component_hooks::PlayerController_ServerAttemptInteract, L"/Script/FortniteGame.FortPlayerController.ServerAttemptInteract", interact_component_hooks::PlayerController_ServerAttemptInteract_Hook);
    REGISTER_UFUNCTION_DETOUR(interact_component_hooks::InteractionComponent_ServerAttemptInteract, L"/Script/FortniteGame.FortControllerComponent_Interaction.ServerAttemptInteract", interact_component_hooks::InteractionComponent_ServerAttemptInteract_Hook);

    DetourTransactionCommit();
}

void interact_component::handle_interaction(AFortPlayerController* player_controller, AActor* receiving_actor)
{
    static UObject* building_container_class = FindObject(L"/Script/FortniteGame.BuildingContainer");
    static UObject* supply_drop_class = FindObject(L"/Script/FortniteGame.FortAthenaSupplyDrop");
    static UObject* player_pawn_class = FindObject(L"/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");

    spdlog::info(L"handle_interaction: player_controller: {}, receiving_actor: {}",
        player_controller->GetName().c_str(),
        receiving_actor->GetName().c_str());
    
    if (UKismetMathLibrary::ClassIsChildOf(receiving_actor->Class, building_container_class))
    {
        // Handle any interactions with building containers
        handle_container_interaction(player_controller, reinterpret_cast<ABuildingContainer*>(receiving_actor));
    }
    else if (UKismetMathLibrary::ClassIsChildOf(receiving_actor->Class, player_pawn_class))
    {
        // Handle any interactions with player pawns
        handle_player_pawn_interaction(player_controller, reinterpret_cast<AFortPlayerPawn*>(receiving_actor));
    }
    else if (UKismetMathLibrary::ClassIsChildOf(receiving_actor->Class, supply_drop_class))
    {
        // Handle any interactions with supply drops
        handle_supply_drop_interaction(player_controller, reinterpret_cast<AFortAthenaSupplyDrop*>(receiving_actor));
    }
}

void interact_component::handle_container_interaction(AFortPlayerController* player_controller,
    ABuildingContainer* building_container)
{
    if (!building_container->bAlreadySearched())
    {
        FString SearchLootTierGroup = building_container->SearchLootTierGroup().ToString().c_str();

        std::wstring final_search_loot_tier_group;
        FString left_str;
        FString right_str;

        // Split the string
        UKismetStringLibrary::Split(SearchLootTierGroup, L"_", &left_str, &right_str, 1, 0); //Split, case insensitive (1), from start

        // Build the string
        final_search_loot_tier_group += left_str.c_str();
        final_search_loot_tier_group += L"_Athena";
        final_search_loot_tier_group += right_str.c_str();

        if (right_str == L"Ammo")
        {
            final_search_loot_tier_group += UKismetMathLibrary::RandomBool() ? L"Large" : L"Small";
        }

        // Get proper location for loot spawn
        FVector ContainerLocation = building_container->K2_GetActorLocation();
        FVector ContainerLocationOffset = building_container->LootSpawnLocation_Athena();
        
        // Sum actor location and loot spawn location to get location to spawn loot
        FVector LootSpawnLocation {};

        LootSpawnLocation.X = ContainerLocation.X += ContainerLocationOffset.X;
        LootSpawnLocation.Y = ContainerLocation.Y += ContainerLocationOffset.Y;
        LootSpawnLocation.Z = ContainerLocation.Z += ContainerLocationOffset.Z;
        
        // Generate and spawn the loot
        std::map<UFortItemDefinition*, int> loot_to_spawn = looting_component::spawn_loot(final_search_loot_tier_group.c_str());

        for (const auto& [ item_definition, count ] : loot_to_spawn)
        {
            // Spawn pickup
            pickup_component::spawn_pickup(item_definition, LootSpawnLocation, count, looting_component::get_default_loaded_ammo(item_definition));
        }

        // Mark the container as open
        UKismetSystemLibrary::SetBoolPropertyByName(building_container, L"bAlreadySearched", true);

        // Update on server
        building_container->OnRep_bAlreadySearched();
    }
}

void interact_component::handle_player_pawn_interaction(AFortPlayerController* player_controller,
    AFortPlayerPawn* player_pawn)
{
    // Revive player
    reviving_component::revive_player(player_controller, player_pawn);
}

void interact_component::handle_supply_drop_interaction(AFortPlayerController* player_controller, AFortAthenaSupplyDrop* supply_drop)
{
    // Spawn loot for supply drop
    std::map<UFortItemDefinition*, int> loot_to_spawn = looting_component::spawn_loot(L"Loot_AthenaSupplyDrop");

    for (const auto& [ item_definition, count ] : loot_to_spawn)
    {
        // Spawn pickup
        pickup_component::spawn_pickup(item_definition, supply_drop->K2_GetActorLocation(), count, looting_component::get_default_loaded_ammo(item_definition));
    }
}

void interact_component_hooks::PlayerController_ServerAttemptInteract_Hook(AFortPlayerController* PlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(AActor*, ReceivingActor);

    if (ReceivingActor == nullptr)
    {
        // No actor to interact with provided
        return;
    }

    // Interact with actor
    interact_component::handle_interaction(PlayerController, ReceivingActor);
    
    return PlayerController_ServerAttemptInteract(PlayerController, StackFrame);
}

void interact_component_hooks::InteractionComponent_ServerAttemptInteract_Hook(UActorComponent* InteractionComponent, FFrame& StackFrame)
{
    GET_PARAMETER(AActor*, ReceivingActor);

    if (ReceivingActor == nullptr)
    {
        // No actor to interact with provided
        return;
    }
    
    AFortPlayerController* player_controller = reinterpret_cast<AFortPlayerController*>(InteractionComponent->GetOwner());

    if (player_controller == nullptr)
    {
        // No owner of the component provided
        return;
    }

    // Interact with actor
    interact_component::handle_interaction(player_controller, ReceivingActor);
    
    return InteractionComponent_ServerAttemptInteract(InteractionComponent, StackFrame);
}