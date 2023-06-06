#include "include.hpp"
#include "interact_component.hpp"

#include "challenges_component.hpp"
#include "inventory_component.hpp"
#include "looting_component.hpp"
#include "pickup_component.hpp"
#include "reviving_component.hpp"
#include "vehicles_component.hpp"

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
    static UObject* vending_machine_class = FindObject(L"/Script/FortniteGame.BuildingItemCollectorActor");
    static UObject* player_pawn_class = FindObject(L"/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");

    if (receiving_actor->IsA(building_container_class))
    {
        // Handle any interactions with building containers
        handle_container_interaction(player_controller, reinterpret_cast<ABuildingContainer*>(receiving_actor));
    }
    else if (receiving_actor->IsA(player_pawn_class))
    {
        // Handle any interactions with player pawns
        handle_player_pawn_interaction(player_controller, reinterpret_cast<AFortPlayerPawn*>(receiving_actor));
    }
    else if (receiving_actor->IsA(vending_machine_class))
    {
        // Handle any interactions with vending machines
        handle_vending_machine_interaction(player_controller, reinterpret_cast<ABuildingItemCollectorActor*>(receiving_actor));
    }
    else if (receiving_actor->IsA(AFortAthenaVehicle::StaticClass()))
    {
        // Handle any interactions with vending machines
        handle_vehicle_interaction(player_controller, reinterpret_cast<AFortAthenaVehicle*>(receiving_actor));
    }
}

void interact_component::handle_container_interaction(AFortPlayerController* player_controller,
    ABuildingContainer* building_container)
{
    if (!building_container->bAlreadySearched())
    {
        STATIC_HASPROPERTY(building_container, LootSpawnLocation_Athena);
        
        // Get proper location for loot spawn
        FVector LootSpawnLocation = building_container->K2_GetActorLocation();
        LootSpawnLocation.Z += bExists_LootSpawnLocation_Athena ? building_container->LootSpawnLocation_Athena().Z : building_container->LootSpawnLocation().Z;
        
        // Generate and spawn the loot
        std::map<UFortItemDefinition*, int> loot_to_spawn = looting_component::spawn_loot(looting_component::redirect_tier_group(building_container->SearchLootTierGroup()));

        for (const auto& [ item_definition, count ] : loot_to_spawn)
        {
            // Spawn pickup
            pickup_component::spawn_pickup(item_definition, LootSpawnLocation, count, looting_component::get_default_loaded_ammo(item_definition));
        }

        // Mark the container as open
        UKismetSystemLibrary::SetBoolPropertyByName(building_container, L"bAlreadySearched", true);
		
		// Container opening effects
        building_container->SearchBounceData().SearchAnimationCount()++;

        // Update on server
        building_container->OnRep_bAlreadySearched();

        // Send progression for this challenge
        challenges_component::SendChestEvent(player_controller, building_container);
    }
}

void interact_component::handle_player_pawn_interaction(AFortPlayerController* player_controller,
    AFortPlayerPawn* player_pawn)
{
    // Revive player
    reviving_component::revive_player(player_controller, player_pawn);
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

                // Destroy vending if needed
                STATIC_HASPROPERTY(vending_machine, NumInteractionsAllowed);
                STATIC_HASPROPERTY(vending_machine, NumInteractions);

                if (bExists_NumInteractions && bExists_NumInteractionsAllowed)
                {
                    if (vending_machine->NumInteractions() >= vending_machine->NumInteractionsAllowed())
                    {
                        static_cast<AB_Athena_VendingMachine_C*>(vending_machine)->DoVendDeath();
                        vending_machine->K2_DestroyActor();
                    }
                }
            }
            
            break;
        }
    }
}

void interact_component::handle_vehicle_interaction(AFortPlayerController* player_controller, AFortAthenaVehicle* vehicle)
{
    // Make sure the component class exists in this version
    if (UFortVehicleSeatWeaponComponent::StaticClass())
    {
        if (player_controller->Pawn()->IsInVehicle())
        {
            unsigned char SeatIndex = player_controller->Pawn()->GetVehicleSeatIndex();

            if (UFortVehicleSeatWeaponComponent* SeatWeaponComponent = vehicle->GetSeatWeaponComponent(SeatIndex))
            {
                UFortWeaponItemDefinition* VehicleWeapon = nullptr;
                TArrayManager WeaponSeatDefinitions = TArrayManager<FWeaponSeatDefinition>(&SeatWeaponComponent->WeaponSeatDefinitions(), FWeaponSeatDefinition::GetStructSize());
                
                for (FWeaponSeatDefinition& SeatDefinition : WeaponSeatDefinitions)
                {
                    if (SeatDefinition.SeatIndex() == SeatIndex)
                    {
                        VehicleWeapon = SeatDefinition.VehicleWeapon();
                    }
                }
                
                vehicles_component::equip_vehicle_weapon(player_controller, VehicleWeapon);
            }
        }
    }
}   

void interact_component_hooks::PlayerController_ServerAttemptInteract_Hook(AFortPlayerController* PlayerController, FFrame& StackFrame)
{
    if (PlayerController == nullptr || PlayerController->MyFortPawn() == nullptr)
    {
        return;
    }

    if (memory_manager::is_bad_read_pointer(PlayerController->MyFortPawn()))
    {
        spdlog::error(L"PlayerController->MyFortPawn is bad read pointer.");
        
        return;
    }
    
    // Call native function first
    PlayerController_ServerAttemptInteract(PlayerController, StackFrame);
    
    GET_PARAMETER(AActor*, ReceivingActor);

    if (ReceivingActor == nullptr)
    {
        // No actor to interact with provided
        return;
    }

    // Interact with actor
    interact_component::handle_interaction(PlayerController, ReceivingActor);
}

void interact_component_hooks::InteractionComponent_ServerAttemptInteract_Hook(UActorComponent* InteractionComponent, FFrame& StackFrame)
{
    // Call native function first
    InteractionComponent_ServerAttemptInteract(InteractionComponent, StackFrame);
    
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
}