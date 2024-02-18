#include "include.hpp"
#include "harvesting_component.hpp"

#include "challenges_component.hpp"
#include "inventory_component.hpp"
#include "log_component.hpp"
#include "player_component.hpp"

harvesting_component::harvesting_component()
{
    REGISTER_PROCESS_EVENT_DETOUR( FindObject(L"/Script/FortniteGame.FortPawn.NetMulticast_Athena_BatchedDamageCues") ? 
        L"/Script/FortniteGame.FortPawn.NetMulticast_Athena_BatchedDamageCues" :
        L"/Script/FortniteGame.FortPlayerPawnAthena.NetMulticast_Athena_BatchedDamageCues" , harvesting_component_hooks::NetMulticast_Athena_BatchedDamageCues_Hook);
}

void harvesting_component::handle_building_damaged(ABuildingSMActor* building_actor, AFortPlayerController* instigated_by, AActor* damage_causer, float damage, FVector hit_location)
{
    if (building_actor != nullptr)
    {
        static UObject* BuildingSMActor_Class = FindObject(L"/Script/FortniteGame.BuildingSMActor");
        static UObject* FortWeapon_Class = FindObject(L"/Script/FortniteGame.FortWeapon");
        static UObject* FortWeaponMeleeItemDefinition_Class = FindObject(L"/Script/FortniteGame.FortWeaponMeleeItemDefinition");
    
        if (building_actor->IsA(BuildingSMActor_Class))
        {
            // Track damage inflicted to building actors
            static_cast<AFortPlayerControllerAthena*>(instigated_by)->EraPlayerInfo().DamageDealtTracker.StructureDamage += damage;

            // If the building is player built & owned by another team, count EnemyStructureDamage
            if (building_actor->AttributeInitLevelSource() == 1)
            {
                // Check PlayerState
                if (instigated_by->PlayerState())
                {
                    // Check team
                    if (building_actor->Team() != instigated_by->PlayerState()->TeamIndex())
                    {
                        static_cast<AFortPlayerControllerAthena*>(instigated_by)->EraPlayerInfo().DamageDealtTracker.EnemyStructureDamage += damage;
                        if (UKismetSystemLibrary::K2_IsTimerActive(instigated_by, L"EnemyStructureDamageCallback"))
                        {
                            UKismetSystemLibrary::K2_ClearTimer(instigated_by, L"EnemyStructureDamageCallback");
                        }
                        UKismetSystemLibrary::K2_SetTimer(instigated_by, L"EnemyStructureDamageCallback", 5.0, false);
                    }
                }
            }
            
            const bool bIsDestroyed = building_actor->GetHealth() - damage < 1;

            if (bIsDestroyed)
            {
                if (building_actor->IsA(AProp_QuestInteractable_GoldBalloon_Parent_C::StaticClass()))
                {
                    if (instigated_by->Pawn())
                    {
                        std::string account_id = player_component::get_account_id(instigated_by);

                        if (account_id.size() > 0)
                        {
                            nlohmann::json Payload{
                                                            {"x", instigated_by->Pawn()->K2_GetActorLocation().X},
                                                            {"y", instigated_by->Pawn()->K2_GetActorLocation().Y}
                            };

                            if (log_component::enable_debug_logs)
                            {
                                spdlog::info("harvesting_component::handle_building_damaged sending interaction:pop-golden-balloon");
                            }

                            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:pop-golden-balloon", Payload);
                        }
                    }
                }   
            }
            
            // Make sure a player is damaging the building with their pickaxe
            if (building_actor->AttributeInitLevelSource() != 1 &&
                damage_causer->IsA(FortWeapon_Class) &&
                static_cast<AFortWeapon*>(damage_causer)->WeaponData()->IsA(FortWeaponMeleeItemDefinition_Class))
            {
                const bool bIsWeakSpot = damage == 100;

                if (bIsDestroyed)
                {
                    // Check if we're tracking any "damage prop" challenges
                    for (auto& [key, val] : challenges_component::DestroyTagsToInteractionMatch)
                    {
                        for (FGameplayTag TargetTag : key)
                        {
                            for (FGameplayTag StaticTag : building_actor->StaticGameplayTags().GameplayTags)
                            {
                                if (StaticTag == TargetTag)
                                {
                                    if (instigated_by->Pawn())
                                    {
                                        std::string account_id = player_component::get_account_id(instigated_by);

                                        if (account_id.size() > 0)
                                        {
                                            nlohmann::json Payload{
                                                {"x", instigated_by->Pawn()->K2_GetActorLocation().X},
                                                {"y", instigated_by->Pawn()->K2_GetActorLocation().Y}
                                            };

                                            if (log_component::enable_debug_logs)
                                            {
                                                spdlog::info("harvesting_component::handle_building_damaged sending {}.", val.c_str());
                                            }

                                            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, val, Payload);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                
                if (!building_actor || !building_actor->BuildingResourceAmountOverride().CurveTable)
                {
                    // Make sure objects are valid
                    return;
                }

                int Count = 0;

                UFortPlaylistAthena* current_playlist = GetPlaylistEasy();

                // Get resource rates table
                UCurveTable* OverrideResourceRates = nullptr;

                if (current_playlist != nullptr)
                {
                    if (STATIC_HASPROPERTY(current_playlist, ResourceRates))
                    {
                        OverrideResourceRates = current_playlist->ResourceRates().Get();
                    }
                }

                // Make sure noone is cheating by calling the func on a far away object
                if (AFortPlayerPawn* damage_causing_pawn = instigated_by->Pawn())
                {
                    FVector building_actor_location = building_actor->K2_GetActorLocation();
                    FVector pawn_location = damage_causing_pawn->K2_GetActorLocation();
                    
                    float distance_from_hit_location = sqrtf((hit_location.X - pawn_location.X) * (hit_location.X - pawn_location.X) +
                                                (hit_location.Y - pawn_location.Y) * (hit_location.Y - pawn_location.Y) +
                                                (hit_location.Z - pawn_location.Z) * (hit_location.Z - pawn_location.Z));
                    
                    if (distance_from_hit_location > 1000)
                    {
                        return;
                    }
                }
                
                auto CurveTable = *reinterpret_cast<TMap<FName, Struct*>*>(__int64(OverrideResourceRates ?
                    OverrideResourceRates :
                    building_actor->BuildingResourceAmountOverride().CurveTable) + 0x30);

                // Get the amount of materials to give from CurveTable
                Count = FSimpleCurve::GetClass() ?
                    static_cast<FSimpleCurve*>(CurveTable[building_actor->BuildingResourceAmountOverride().RowName].Value)->Keys()[0].Value() :
                    static_cast<FRichCurve*>(CurveTable[building_actor->BuildingResourceAmountOverride().RowName].Value)->Keys()[0].Value();
                
                // Don't give mats for weakspot if damage > health before actor death (Weakspots give a 2x bonus so divide by 2)
                if (damage > building_actor->GetHealth() + damage) damage = building_actor->GetHealth() + damage;

                // Calculate the amount of materials to give per hit
                Count = round((Count / building_actor->GetMaxHealth()) * damage);
                
                // Bonus varying from 0% to 15%
                Count += round(UKismetMathLibrary::RandomFloatInRange(0, 0.15)*Count);
                
                if (Count <= 0) return;
                
                if (Count > 0)
                {
                    // Store ResourceType in a variable for performance purpose
                    const uint8_t ResourceType = building_actor->ResourceType();

                    // Notify client with the resources they have harvested
                    instigated_by->ClientReportDamagedResourceBuilding(building_actor, ResourceType, Count, bIsDestroyed, bIsWeakSpot);

                    UFortItemDefinition* ResourceItemDefinition = nullptr;

                    static UFortItemDefinition* WoodItemData = FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
                    static UFortItemDefinition* StoneItemData = FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
                    static UFortItemDefinition* MetalItemData = FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

                    // Convert ResourceType to ItemDefinition
                    if (ResourceType == 0) ResourceItemDefinition = WoodItemData;
                    if (ResourceType == 1) ResourceItemDefinition = StoneItemData;
                    if (ResourceType == 2) ResourceItemDefinition = MetalItemData;

                    if (ResourceItemDefinition)
                    {
                        // Give the resource to the player
                        inventory_component::add_item(instigated_by, ResourceItemDefinition, Count, 0);

                        // update gathered materials
                        inventory_component::safe_update_materials_gathered(instigated_by, ResourceItemDefinition, Count);
                    }
                }
            }
        }
    }
}

void harvesting_component_hooks::NetMulticast_Athena_BatchedDamageCues_Hook(UObject* Object, UObject* Function, void* Params)
{
    AActor* HitActor = nullptr;
    float Magnitude = 0.f;
    FVector HitLocation = FVector();
    
    if (FindPropertyByName(Function, L"BatchData"))
    {
        GET_PROCESS_EVENT_PARAMETER(FAthenaBatchedDamageGameplayCues, BatchData);

        HitActor = BatchData.HitActor();
        Magnitude = BatchData.Magnitude();
        HitLocation = BatchData.Location();
    }
    else
    {
        GET_PROCESS_EVENT_PARAMETER(FAthenaBatchedDamageGameplayCues_Shared, SharedData);
        GET_PROCESS_EVENT_PARAMETER(FAthenaBatchedDamageGameplayCues_NonShared, NonSharedData);

        HitActor = NonSharedData.HitActor();
        Magnitude = SharedData.Magnitude();
        HitLocation = SharedData.Location();
    }
    
    // Get owner controller
    AFortPlayerController* player_controller = reinterpret_cast<AFortPlayerPawn*>(Object)->Controller();

    // Safety check
    if (memory_manager::is_bad_read_pointer(player_controller) || player_controller == nullptr)
    {
        spdlog::error(L"harvesting_component_hooks::NetMulticast_Athena_BatchedDamageCues_Hook: Invalid PlayerController");
        return;
    }

    // Filter by building actors
    static UObject* building_actor_class = FindObject(L"/Script/FortniteGame.BuildingSMActor");
    
    if (HitActor)
    {
        if (HitActor->IsA(building_actor_class))
        {
            if (player_controller->Pawn())
            {
                if (player_controller->Pawn()->CurrentWeapon())
                {
                    ABuildingSMActor* building_actor = reinterpret_cast<ABuildingSMActor*>(HitActor);
        
                    // Handle harvesting materials if we have a building actor and weapon
                    harvesting_component::handle_building_damaged(building_actor, player_controller, player_controller->Pawn()->CurrentWeapon(), Magnitude, HitLocation);
                }
            }
        }
    }
}