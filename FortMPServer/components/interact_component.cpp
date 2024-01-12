#include "include.hpp"
#include "interact_component.hpp"

#include "challenges_component.hpp"
#include "inventory_component.hpp"
#include "log_component.hpp"
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

void interact_component::handle_interaction(AFortPlayerController* player_controller, AActor* receiving_actor, UPrimitiveComponent* interact_component, uint8_t interaction_type)
{
    static UObject* building_container_class = FindObject(L"/Script/FortniteGame.BuildingContainer");
    static UObject* vending_machine_class = FindObject(L"/Script/FortniteGame.BuildingItemCollectorActor");
    static UObject* player_pawn_class = FindObject(L"/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");

    if (log_component::enable_debug_logs)
    {
        // Log receiving actor
        spdlog::info(L"handle_interaction called on receiving_actor: {}", UKismetSystemLibrary::GetPathName(receiving_actor).c_str());
    }

    if (receiving_actor->IsA(building_container_class))
    {
        // Handle any interactions with building containers
        handle_container_interaction(player_controller, reinterpret_cast<ABuildingContainer*>(receiving_actor));
    }
    else if (receiving_actor->IsA(player_pawn_class))
    {
        // 2 = LongInteraction, might stop insta-revive cheats considering they probably call AttemptInteract with interaction type as 0 (NoInteraction).
        if (interaction_type == 2)
        {
            // Handle any interactions with player pawns
            handle_player_pawn_interaction(player_controller, reinterpret_cast<AFortPlayerPawn*>(receiving_actor));
        }
        else if (reinterpret_cast<AFortPlayerPawn*>(receiving_actor)->IsDBNO())
        {
            // # THE MESSAGE HAS NOTHING TO DO WITH THE CODE AND IT IS A TRAP FOR CHEATERS
            // This is supposed to mislead the hackers when they try to cheat. If someone gets kicked for this reason, that means they attempted to use an insta-revive cheat.
            player_controller->ClientReturnToMainMenu(L"AFortPlayerPawn::DBNOInteractionCollisionProfile : No DBNOInteractionCollisionProfile. Failed to get finished hero soundwave.");
        }
    }
    else if (receiving_actor->IsA(vending_machine_class))
    {
        // Handle any interactions with vending machines
        handle_vending_machine_interaction(player_controller, reinterpret_cast<ABuildingItemCollectorActor*>(receiving_actor));
    }
    else if (receiving_actor->IsA(AFortAthenaVehicle::StaticClass(log_component::enable_debug_logs)))
    {
        // Handle any interactions with vending machines
        handle_vehicle_interaction(player_controller, reinterpret_cast<AFortAthenaVehicle*>(receiving_actor));

        // Shopping carts
        if (receiving_actor->IsA(AShoppingCartVehicleSK::StaticClass(log_component::enable_debug_logs)))
        {
            handle_shoppingcart_interaction(player_controller, static_cast<AShoppingCartVehicleSK*>(receiving_actor));
        }
        else if (receiving_actor->IsA(AFortAthenaFerretVehicle::StaticClass(log_component::enable_debug_logs)))
        {
            handle_plane_interaction(player_controller, static_cast<AFortAthenaFerretVehicle*>(receiving_actor));
        }
    }
    else if (receiving_actor->IsA(AAthenaSupplyDrop_Llama_C::StaticClass(log_component::enable_debug_logs)))
    {
        // Handle any interactions with llamas, mainly for challenges
        handle_llama_interaction(player_controller, reinterpret_cast<AFortAthenaSupplyDrop*>(receiving_actor));
    }
    else if (receiving_actor->IsA(AFortAthenaSupplyDrop::StaticClass(log_component::enable_debug_logs)))
    {
        // Handle any interactions with supply drops, for challenges
        handle_supplydrop_interaction(player_controller, reinterpret_cast<AFortAthenaSupplyDrop*>(receiving_actor));
    }
    else if (receiving_actor->IsA(AProp_QuestActor_FloatingLightningBolt_Parent_C::StaticClass(log_component::enable_debug_logs)))
    {
        // Handle any interaction with lightning bolts for challenges
        handle_lightningbolt_interaction(player_controller, reinterpret_cast<AProp_QuestActor_FloatingLightningBolt_Parent_C*>(receiving_actor));
    }
    else if (receiving_actor->IsA(AProp_QuestInteractable_HiddenStar_Parent_C::StaticClass(log_component::enable_debug_logs)))
    {
        // Handle any interactions with battle stars
        handle_battlestar_interaction(player_controller, reinterpret_cast<AProp_QuestInteractable_HiddenStar_Parent_C*>(receiving_actor));
    }
    else if (receiving_actor->IsA(ACBGA_HealthSmall_C::StaticClass(log_component::enable_debug_logs)))
    {
        // Handle any interaction with apples, for challenges
        handle_apple_interaction(player_controller, reinterpret_cast<ACBGA_HealthSmall_C*>(receiving_actor));
    }
    else if (receiving_actor->IsA(ACBGA_ShieldsSmall_C::StaticClass(log_component::enable_debug_logs)))
    {
        // Handle any interaction with mushrooms, for challenges
        handle_mushroom_interaction(player_controller, reinterpret_cast<ACBGA_ShieldsSmall_C*>(receiving_actor));
    }
    else if (receiving_actor->IsA(AProp_QuestInteractable_Jigsaw_Parent::StaticClass()))
    {
        // Handle any interaction with jigsaw puzzle
        handle_jigsaw_puzzle_interaction(player_controller, reinterpret_cast<AProp_QuestInteractable_Jigsaw_Parent*>(receiving_actor));
    }
    else if (receiving_actor->IsA(AProp_Athena_Doorbell_Interactable_C::StaticClass()))
    {
        handle_doorbell_interaction(player_controller, reinterpret_cast<AProp_Athena_Doorbell_Interactable_C*>(receiving_actor));
    }
    else if (receiving_actor->IsA(AProp_FMP_GooseEgg_Interactable::StaticClass()))
    {
        handle_gooseegg_interaction(player_controller, reinterpret_cast<AProp_FMP_GooseEgg_Interactable*>(receiving_actor));
    }
}

void interact_component::handle_container_interaction(AFortPlayerController* player_controller,
    ABuildingContainer* building_container)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"interact_component::handle_container_interaction is sending container interaction.");
    }
    
    if (!building_container->bAlreadySearched())
    {
        STATIC_HASPROPERTY(building_container, LootSpawnLocation_Athena);
        
        // Get proper location for loot spawn
        FVector LootSpawnLocation = building_container->CalculateLootSpawnLocation();
        
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
        if (building_container->SearchLootTierGroup() == L"Loot_Treasure")
        {
            challenges_component::SendChestEvent(player_controller, building_container);
        }
        else if (building_container->SearchLootTierGroup() == L"Loot_Ammo")
        {
            challenges_component::SendAmmoEvent(player_controller, building_container);
        }
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
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"interact_component::handle_vending_machine_interaction is sending vending_machine interaction.");
    }
    
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

                // Send event
                challenges_component::SendVendingMachineInteraction(player_controller, static_cast<AB_Athena_VendingMachine_C*>(vending_machine));

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

void interact_component::handle_shoppingcart_interaction(AFortPlayerController* player_controller, AShoppingCartVehicleSK* vehicle)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"interact_component::handle_shoppingcart_interaction is sending shopping cart interaction.");
    }
    
    challenges_component::SendShoppingCartInteraction(player_controller);
}

void interact_component::handle_plane_interaction(AFortPlayerController* player_controller,
    AFortAthenaFerretVehicle* vehicle)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"interact_component::handle_plane_interaction is sending plane interaction.");
    }
    
    challenges_component::SendPlaneInteraction(reinterpret_cast<AFortPlayerControllerAthena*>(player_controller));
}

void interact_component::handle_llama_interaction(AFortPlayerController* player_controller, AFortAthenaSupplyDrop* supply_drop)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"interact_component::handle_llama_interaction is sending llama interaction.");
    }
    
    challenges_component::SendLlamaEvent(player_controller, supply_drop);
}

void interact_component::handle_supplydrop_interaction(AFortPlayerController* player_controller, AFortAthenaSupplyDrop* supply_drop)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"interact_component::handle_supplydrop_interaction is sending supplydrop interaction.");
    }
    
    challenges_component::SendSupplyDropEvent(player_controller, supply_drop);
}

void interact_component::handle_letter_interaction(AFortPlayerController* player_controller, AProp_QuestInteractable_Letter_Parent_C* letter)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"interact_component::handle_letter_interaction is sending letter interaction.");
    }
    
    if (letter->HasProperty(L"QuestBackendObjectiveName"))
    {
        // Send interaction event
        challenges_component::SendLetterInteraction(player_controller, letter, letter->QuestBackendObjectiveName());
    }
}

void interact_component::handle_gnome_interaction(AFortPlayerController* player_controller, AProp_Quest_GniceGnomes_Parent_C* gnome)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"interact_component::handle_gnome_interaction is sending gnome interaction.");
    }
    
    if (gnome->HasProperty(L"QuestBackendObjectiveName"))
    {
        // Send interaction event
        challenges_component::SendGnomeInteraction(player_controller, gnome, gnome->QuestBackendObjectiveName());
    }
}

void interact_component::handle_battlestar_interaction(AFortPlayerController* player_controller, AProp_QuestObject_Interactable_Parent_C* battlestar)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"interact_component::handle_battlestar_interaction is sending battlestar interaction.");
    }
    
    if (battlestar->HasProperty(L"QuestBackendObjectiveName"))
    {
        // Send interaction event
        challenges_component::SendBattleStarInteraction(player_controller, battlestar, battlestar->QuestBackendObjectiveName());
    }
}

void interact_component::handle_battlestar_interaction(AFortPlayerController* player_controller, AProp_QuestInteractable_HiddenStar_Parent_C* battlestar)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"interact_component::handle_battlestar_interaction is sending battlestar interaction.");
    }
    
    if (battlestar->HasProperty(L"QuestBackendObjectiveName"))
    {
        // Send interaction event
        challenges_component::SendBattleStarInteraction(player_controller, battlestar, battlestar->QuestBackendObjectiveName());
    }
}

void interact_component::handle_hoprock_interaction(AFortPlayerController* player_controller, ACBGA_LowGravity_UsingJump_C* hoprock)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"interact_component::handle_hoprock_interaction is sending hoprock interaction.");
    }
    
    // Send interaction event
    challenges_component::SendHopRockInteraction(player_controller, hoprock);
}

void interact_component::handle_apple_interaction(AFortPlayerController* player_controller, ACBGA_HealthSmall_C* apple)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"interact_component::handle_apple_interaction is sending apple interaction.");
    }
    
    // Send interaction event
    challenges_component::SendAppleInteraction(player_controller, apple);
}

void interact_component::handle_mushroom_interaction(AFortPlayerController* player_controller, ACBGA_ShieldsSmall_C* mushroom)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"interact_component::handle_mushroom_interaction is sending mushroom interaction.");
    }
    
    // Send interaction event
    challenges_component::SendMushroomInteraction(player_controller, mushroom);
}

void interact_component::handle_rubberduck_interaction(AFortPlayerController* player_controller, AProp_QuestInteractable_RubberDuckies_Parent_C* rubberduck)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"interact_component::handle_rubberduck_interaction is sending rubberduck interaction.");
    }
    
    if (rubberduck->HasProperty(L"QuestBackendObjectiveName"))
    {
        // Send interaction event
        challenges_component::SendRubberDuckInteraction(player_controller, rubberduck, rubberduck->QuestBackendObjectiveName());
    }
}

void interact_component::handle_gooseegg_interaction(AFortPlayerController* player_controller, AProp_FMP_GooseEgg_Interactable* goose_egg)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"interact_component::handle_gooseegg_interaction is sending goose egg interaction.");
    }
    
    if (goose_egg->HasProperty(L"QuestBackendObjectiveName"))
    {
        // Send interaction event
        challenges_component::SendGooseEggInteraction(player_controller, goose_egg, goose_egg->QuestBackendObjectiveName());
    }
}

void interact_component::handle_lightningbolt_interaction(AFortPlayerController* player_controller, AProp_QuestActor_FloatingLightningBolt_Parent_C* lightningbolt)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"interact_component::handle_lightningbolt_interaction is sending lightning bolt interaction.");
    }
    
    if (lightningbolt->HasProperty(L"QuestBackendObjectiveName"))
    {
        // Send interaction event
        challenges_component::SendLightningBoltInteraction(player_controller, lightningbolt, lightningbolt->QuestBackendObjectiveName());
    }
}


void interact_component::handle_jigsaw_puzzle_interaction(AFortPlayerController* player_controller, AProp_QuestInteractable_Jigsaw_Parent* jigsawpuzzle)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"interact_component::handle_jigsaw_puzzle_interaction is sending jigsaw puzzle interaction.");
    }
    
    if (jigsawpuzzle->HasProperty(L"QuestBackendObjectiveName"))
    {
        // Send interaction event
        challenges_component::SendJigsawPuzzleInteraction(player_controller, jigsawpuzzle, jigsawpuzzle->QuestBackendObjectiveName());
    } 
}

void interact_component::handle_shadowstone_interaction(AFortPlayerController* player_controller,
    ACBGA_SpookyMist_C* spookymist)
{
    return; // Deprecated
    inventory_component::add_item(player_controller, FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Gameplay/SpookyMist/AGID_SpookyMist.AGID_SpookyMist"), 1);
}

void interact_component::handle_doorbell_interaction(AFortPlayerController* player_controller, AProp_Athena_Doorbell_Interactable_C* doorbell)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"interact_component::handle_doorbell_interaction: sending interaction:doorbell-poi with objective {}.", doorbell->ObjBackendName().ToString().c_str());
    }
    
    challenges_component::SendDoorbellPOIInteraction(player_controller, doorbell->ObjBackendName());
    
    if (UBoxComponent* BoxComponent = doorbell->InsideHouseOverlap())
    {
        FVector ComponentLocation = BoxComponent->K2_GetComponentLocation();
        FVector UnscaledBoxExtent = BoxComponent->GetUnscaledBoxExtent();
        
        // Log
        if (log_component::enable_debug_logs)
        {
            spdlog::info(L"interact_component::handle_doorbell_interaction: Checking overlap for BoxComponent [{}].", UKismetSystemLibrary::GetPathName(BoxComponent).c_str());
            spdlog::info(L"interact_component::handle_doorbell_interaction: {} [Location: [{}, {}, {}], UnscaledBoxExtent: [{}, {}, {}]].", BoxComponent->GetName().c_str(), ComponentLocation.X, ComponentLocation.Y, ComponentLocation.Z, UnscaledBoxExtent.X, UnscaledBoxExtent.Y, UnscaledBoxExtent.Z);
        }
        
        // Construct parameters for BoxOverlapActors
        TArray<AActor*> ActorsToIgnore{};
        TArray<uint8_t> ObjectTypes;
        UEnum* EObjectTypeQuery = FindObject<UEnum>(L"/Script/Engine.EObjectTypeQuery");
        for (uint8_t i = 0; i < EObjectTypeQuery->GetEnumIndex(L"ObjectTypeQuery_MAX"); i++)
        {
            ObjectTypes.Add(i);
        }
        TArray<AActor*> Result;

        // BoxOverlapActors
        UKismetSystemLibrary::BoxOverlapActors(*GWorld, ComponentLocation, UnscaledBoxExtent, ObjectTypes, AFortPawn::StaticClass(), ActorsToIgnore, &Result);

        // Send interaction event for instigator
        if (Result.Num() > 0)
        {
            challenges_component::SendDoorbellInteraction(player_controller);
        }
    }
}

void interact_component_hooks::PlayerController_ServerAttemptInteract_Hook(AFortPlayerController* PlayerController, FFrame& StackFrame)
{
    if (PlayerController == nullptr || PlayerController->MyFortPawn() == nullptr)
    {
        spdlog::error(L"interact_component_hooks::PlayerController_ServerAttemptInteract_Hook: invalid PlayerController or MyFortPawn.");
        
        return;
    }

    if (memory_manager::is_bad_read_pointer(PlayerController->MyFortPawn()))
    {
        spdlog::error(L"PlayerController->MyFortPawn is bad read pointer.");
        
        return;
    }
    
    GET_PARAMETER(AActor*, ReceivingActor);
    AActor* CopiedReceivingActor = ReceivingActor;

    GET_PARAMETER(UPrimitiveComponent*, InteractComponent);
    
    GET_PARAMETER(uint8_t, InteractType);
    
    // Call native function first
    PlayerController_ServerAttemptInteract(PlayerController, StackFrame);
    
    if (CopiedReceivingActor == nullptr)
    {
        //
        spdlog::error(L"interact_component_hooks::PlayerController_ServerAttemptInteract_Hook: attempting interaction with invalid CopiedReceivingActor.");
        
        // No actor to interact with provided
        return;
    }

    // Interact with actor
    interact_component::handle_interaction(PlayerController, CopiedReceivingActor, InteractComponent, InteractType);
}

void interact_component_hooks::InteractionComponent_ServerAttemptInteract_Hook(UActorComponent* InteractionComponent, FFrame& StackFrame)
{
    // Call native function first
    InteractionComponent_ServerAttemptInteract(InteractionComponent, StackFrame);
    
    GET_PARAMETER(AActor*, ReceivingActor);

    GET_PARAMETER(UPrimitiveComponent*, InteractComponent);
    
    GET_PARAMETER(uint8_t, InteractType);
    
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
    interact_component::handle_interaction(player_controller, ReceivingActor, InteractComponent, InteractType);
}