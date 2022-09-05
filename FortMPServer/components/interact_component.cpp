#include "include.hpp"
#include "interact_component.hpp"

#include "inventory_component.hpp"
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
    static UObject* vending_machine_class = FindObject(L"/Script/FortniteGame.BuildingItemCollectorActor");
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
    else if (UKismetMathLibrary::ClassIsChildOf(receiving_actor->Class, vending_machine_class))
    {
        // Handle any interactions with vending machines
        handle_vending_machine_interaction(player_controller, reinterpret_cast<ABuildingItemCollectorActor*>(receiving_actor));
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

void interact_component::handle_vending_machine_interaction(AFortPlayerController* player_controller, ABuildingItemCollectorActor* vending_machine)
{
    UFortItemDefinition* deposit_item_definition = vending_machine->ActiveInputItem();
    
    // now get the item collection that takes the current deposit item
    auto ItemCollections = TArrayManager<FCollectorUnitInfo>(&vending_machine->ItemCollections(), FCollectorUnitInfo::GetStructSize());

    for (int i = 0; i < ItemCollections.Num(); i++)
    {
        if (ItemCollections[i].InputItem() == deposit_item_definition)
        {
            // Get the amount of materials to give from CurveTable
            int deposit_amount = vending_machine->GetDepositGoal(player_controller, deposit_item_definition);

            if (UFortWorldItem* deposit_item_instance = static_cast<UFortWorldItem*>(inventory_component::get_inventory_item_by_definition(player_controller, deposit_item_definition));
                deposit_item_instance != nullptr || deposit_amount == 0)
            {
                // Pay the item cost
                if (deposit_item_instance != nullptr && deposit_amount != 0)
                {
                    if (deposit_item_instance->ItemEntry().Count() < deposit_amount)
                    {
                        spdlog::warn(L"Player '{}' tried to buy from vending machines without enough materials.", player_controller->MyPlayerInfo()->PlayerName().c_str());
                            
                        return;
                    }
                }
                
                // Now spawn all the item entries
                auto OutputItemEntries = TArrayManager<FFortItemEntry>(&ItemCollections[i].OutputItemEntry(), FFortItemEntry::GetStructSize());

                for (FFortItemEntry& item_entry : OutputItemEntries)
                {
                    FVector LootSpawnLocation = vending_machine->K2_GetActorLocation();
                    LootSpawnLocation.Z += vending_machine->LootSpawnLocation().Z; // Change the Z axis not to have the pickup stuck in the floor
                    
                    AFortPickup* new_pickup = pickup_component::spawn_pickup(item_entry.ItemDefinition(), LootSpawnLocation, item_entry.Count(), looting_component::get_default_loaded_ammo(item_entry.ItemDefinition()), false);

                    if (new_pickup)
                    {
                        // Custom directional tossing
                        pickup_component::toss_pickup(new_pickup, vending_machine, vending_machine->LootTossDirection(), vending_machine->LootTossConeHalfAngle(), vending_machine->LootTossSpeed());
                    }
                }

                // If a new pickup has been spawned, charge the player with the price
                if (deposit_item_instance != nullptr && deposit_amount != 0)
                {
                    inventory_component::remove_item(player_controller, deposit_item_instance->ItemEntry().ItemGuid(), deposit_amount, false);
                }
            }
            
            break;
        }
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