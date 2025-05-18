#include "include.hpp"
#include "harvesting_component.hpp"

#include "challenges_component.hpp"
#include "era_mutators_component.hpp"
#include "inventory_component.hpp"
#include "log_component.hpp"
#include "player_component.hpp"

harvesting_component::harvesting_component()
{

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(harvesting_component_hooks::OnBuildingDamaged, memory_manager::find_function_by_string(L"Player %s reported hitting weak spot on building %s, but none found!"), harvesting_component_hooks::OnBuildingDamaged_Hook);
    
    DetourTransactionCommit();
}

void harvesting_component_hooks::OnBuildingDamaged_Hook(ABuildingActor* BuildingActor, float Damage, __int64 a3, __int64 a4, UObject* Instigatedby, AActor* DamagedBy)
{
    // Call the original func
    OnBuildingDamaged(BuildingActor, Damage, a3, a4, Instigatedby, DamagedBy);

    // Safety checks
    if (Instigatedby == nullptr || memory_manager::is_bad_read_pointer(Instigatedby) || Instigatedby->Cast<AFortPlayerController>() == nullptr || DamagedBy == nullptr ||  memory_manager::is_bad_read_pointer(DamagedBy))
        return;

    harvesting_component::handle_building_damaged(BuildingActor->Cast<ABuildingSMActor>(), Instigatedby->Cast<AFortPlayerController>(), DamagedBy, Damage);
}

void harvesting_component::handle_building_damaged(ABuildingSMActor* building_actor, AFortPlayerController* instigated_by, AActor* damage_causer, float damage)
{
    if (instigated_by)
    {
        if (player_component::is_live_spectator(instigated_by))
        {
            return;
        }
    }
    
    if (building_actor != nullptr)
    {
        if (building_actor->IsA(AFortAthenaSupplyDropBalloon::StaticClass()))
        {
            if (auto root_component = building_actor->RootComponent())
            {
                if (auto attach_parent = root_component->AttachParent())
                {
                    if (AAthenaSupplyDrop_C* supply_drop = attach_parent->GetOwner()->Cast<AAthenaSupplyDrop_C>())
                    {
                        if (supply_drop->K2_GetActorLocation().Z - supply_drop->GroundLocation().Z > 100)
                        {
                            static_cast<AFortPlayerControllerAthena*>(instigated_by)->EraPlayerInfo().DamageDealtTracker.SupplyDropBalloon += damage;
                            if (UKismetSystemLibrary::K2_IsTimerActive(instigated_by, L"SupplyDropDamageCallback"))
                            {
                                UKismetSystemLibrary::K2_ClearTimer(instigated_by, L"SupplyDropDamageCallback");
                            }
                            UKismetSystemLibrary::K2_SetTimer(instigated_by, L"SupplyDropDamageCallback", 5.0, false);   
                        }
                    }
                }
            }
        }
        else if (building_actor->IsA(ABuildingSMActor::StaticClass()))
        {
            // Track damage inflicted to building actors
            static_cast<AFortPlayerControllerAthena*>(instigated_by)->EraPlayerInfo().DamageDealtTracker.StructureDamage += damage;

            const bool bIsPlayerBuiltStructure = building_actor->AttributeInitLevelSource() == 1;
            
            // If the building is player built & owned by another team, count EnemyStructureDamage
            if (bIsPlayerBuiltStructure)
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
            
            const bool bIsDestroyed = building_actor->GetHealth() < 1;

            if (bIsDestroyed)
            {
                if (building_actor->IsA(AProp_QuestInteractable_GoldBalloon_Parent_C::StaticClass()))
                {
                    if (instigated_by->Pawn())
                    {
                        challenges_component::SendCustomEventWithObjective(instigated_by, "interaction:pop-golden-balloon", building_actor->Cast<AProp_QuestInteractable_GoldBalloon_Parent_C>()->ObjBackendName());
                    }
                }   
            }
            
            // Make sure a player is damaging the building with their pickaxe
            if (building_actor->AttributeInitLevelSource() != 1 &&
                damage_causer->IsA(AFortWeapon::StaticClass()) &&
                static_cast<AFortWeapon*>(damage_causer)->WeaponData()->IsA(UFortWeaponMeleeItemDefinition::StaticClass()))
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

                if (era_mutators_component::IsActive("competitive"))
                {
                    // 30% boost for competitive
                    Count += round(0.3 * Count);
                }
                
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
                        inventory_component::add_item(instigated_by, ResourceItemDefinition, Count, 0, {}, false);

                        // update gathered materials
                        inventory_component::safe_update_materials_gathered(instigated_by, ResourceItemDefinition, Count);
                    }
                }
            }
        }
    }
}