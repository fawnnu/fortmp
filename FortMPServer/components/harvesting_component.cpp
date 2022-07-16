#include "include.hpp"
#include "harvesting_component.hpp"

#include "inventory_component.hpp"

harvesting_component::harvesting_component()
{
    REGISTER_PROCESS_EVENT_DETOUR(L"/Script/FortniteGame.FortPawn.NetMulticast_Athena_BatchedDamageCues", harvesting_component_hooks::NetMulticast_Athena_BatchedDamageCues_Hook);
    REGISTER_PROCESS_EVENT_DETOUR(L"/Script/FortniteGame.FortPlayerPawnAthena.NetMulticast_Athena_BatchedDamageCues", harvesting_component_hooks::NetMulticast_Athena_BatchedDamageCues_Old_Hook);
}

void harvesting_component::handle_building_damaged(ABuildingSMActor* building_actor, AFortPlayerController* instigated_by, AActor* damage_causer, float damage)
{
    static UObject* BuildingSMActor_Class = FindObject(L"/Script/FortniteGame.BuildingSMActor");
    static UObject* FortWeapon_Class = FindObject(L"/Script/FortniteGame.FortWeapon");
    static UObject* FortWeaponMeleeItemDefinition_Class = FindObject(L"/Script/FortniteGame.FortWeaponMeleeItemDefinition");
    
    if (UKismetMathLibrary::ClassIsChildOf(building_actor->Class, BuildingSMActor_Class))
    {
        // Make sure a player is damaging the building with their pickaxe
        if (building_actor->AttributeInitLevelSource() != 1 &&
            UKismetMathLibrary::ClassIsChildOf(damage_causer->Class, FortWeapon_Class) &&
            UKismetMathLibrary::ClassIsChildOf(static_cast<AFortWeapon*>(damage_causer)->WeaponData()->Class, FortWeaponMeleeItemDefinition_Class))
        {
            const bool bIsDestroyed = building_actor->GetHealth() - damage < 1;
            const bool bIsWeakSpot = damage == 100;
            
            if (!building_actor || !building_actor->BuildingResourceAmountOverride().CurveTable)
            {
                // Make sure objects are valid
                return;
            }

            int Count = 0;

            auto CurveTable = *reinterpret_cast<TMap<FName, Struct*>*>(__int64(building_actor->BuildingResourceAmountOverride().CurveTable) + 0x30);

            // Get the amount of materials to give from CurveTable
            Count = FSimpleCurve::GetClass() ?
                static_cast<FSimpleCurve*>(CurveTable[building_actor->BuildingResourceAmountOverride().RowName].Value)->Keys()[0].Value() :
                static_cast<FRichCurve*>(CurveTable[building_actor->BuildingResourceAmountOverride().RowName].Value)->Keys()[0].Value();
                
            Count = round(Count / (round(building_actor->GetMaxHealth() / damage)));

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
                    inventory_component::add_item(instigated_by, ResourceItemDefinition, Count);
                }
            }
        }
    }
}

void harvesting_component_hooks::NetMulticast_Athena_BatchedDamageCues_Old_Hook(UObject* Object, UObject* Function, void* Params)
{
    GET_PROCESS_EVENT_PARAMETER(FAthenaBatchedDamageGameplayCues, BatchData);

    // Get owner controller
    AFortPlayerController* player_controller = reinterpret_cast<AFortPlayerPawn*>(Object)->Controller();

    // Filter by building actors
    static UObject* building_actor_class = FindObject(L"/Script/FortniteGame.BuildingSMActor");
    
    if (BatchData.HitActor())
    {
        if (UKismetMathLibrary::ClassIsChildOf(BatchData.HitActor()->Class, building_actor_class))
        {
            if (player_controller->Pawn()->CurrentWeapon())
            {
                ABuildingSMActor* building_actor = reinterpret_cast<ABuildingSMActor*>(BatchData.HitActor());
        
                // Handle harvesting materials if we have a building actor and weapon
                harvesting_component::handle_building_damaged(building_actor, player_controller, player_controller->Pawn()->CurrentWeapon(), BatchData.Magnitude());
            }
        }
    }
}

void harvesting_component_hooks::NetMulticast_Athena_BatchedDamageCues_Hook(UObject* Object, UObject* Function, void* Params)
{
    GET_PROCESS_EVENT_PARAMETER(FAthenaBatchedDamageGameplayCues_Shared, SharedData);
    GET_PROCESS_EVENT_PARAMETER(FAthenaBatchedDamageGameplayCues_NonShared, NonSharedData);

    // Get owner controller
    AFortPlayerController* player_controller = reinterpret_cast<AFortPlayerPawn*>(Object)->Controller();

    // Filter by building actors
    static UObject* building_actor_class = FindObject(L"/Script/FortniteGame.BuildingSMActor");
    
    if (NonSharedData.HitActor())
    {
        if (UKismetMathLibrary::ClassIsChildOf(NonSharedData.HitActor()->Class, building_actor_class))
        {
            if (player_controller->Pawn()->CurrentWeapon())
            {
                ABuildingSMActor* building_actor = reinterpret_cast<ABuildingSMActor*>(NonSharedData.HitActor());
        
                // Handle harvesting materials if we have a building actor and weapon
                harvesting_component::handle_building_damaged(building_actor, player_controller, player_controller->Pawn()->CurrentWeapon(), SharedData.Magnitude());
            }
        }
    }
}