#include "include.hpp"
#include "era_mutators_component.hpp"

#include "config_manager.hpp"
#include "inventory_component.hpp"

era_mutator::era_mutator(std::string in_mutator_name, ExecuteCallback in_callback)
{
    mutator_name = in_mutator_name;
    callback = in_callback;
    era_mutators_component::era_mutators.push_back(this);
}

era_mutators_component::era_mutators_component()
{
    new era_mutator("rocket", [](LPVOID)
    {
        if (GetCL() == 4159770)
        {
            if (ABP_GeodeScripting_C::StaticClass(true))
            {
                TArray<AActor*> geode_scripters = UGameplayStatics::GetAllActorsOfClass((*GWorld), ABP_GeodeScripting_C::StaticClass());
                                
                for (auto actor : geode_scripters)
                {
                    static_cast<ABP_GeodeScripting_C*>(actor)->LaunchSequence();
                }
            }
        }
    });
    
    new era_mutator("rift_s4_finale_no_cracks", [](LPVOID Parameters)
    {
        if (ABP_Athena_Event_Components_C::StaticClass(true))
        {
            // Enable the crack in the sky
            TArray<ABP_Athena_Event_Components_C*> Event_Components = UGameplayStatics::GetAllActorsOfClass<ABP_Athena_Event_Components_C>(*GWorld, ABP_Athena_Event_Components_C::StaticClass());

            // Set properties on all Event Components
            for (ABP_Athena_Event_Components_C* Event_Component : Event_Components)
            {
                Event_Component->AreEndSequenceCrackVisible() = 1;
            }
        }
    });
    
    new era_mutator("rift_s4_finale", [](LPVOID Parameters)
    {
        if (ABP_Athena_Event_Components_C::StaticClass(true))
        {
            // Enable the crack in the sky
            TArray<ABP_Athena_Event_Components_C*> Event_Components = UGameplayStatics::GetAllActorsOfClass<ABP_Athena_Event_Components_C>(*GWorld, ABP_Athena_Event_Components_C::StaticClass());

            // Set properties on all Event Components
            for (ABP_Athena_Event_Components_C* Event_Component : Event_Components)
            {
                Event_Component->AreEndSequenceCrackVisible() = 1;
                Event_Component->AreCracksVisible() = 1;
                Event_Component->CrackProgression() = 1.0;
            }
        }
    });

    new era_mutator("cube_s5", [](LPVOID Parameters)
    {
        if (ACUBE_C::StaticClass(true))
        {
            int Starting_Cube_Path_Index = 0;
            // Get the Cube actor
            TArray<AActor*> cube_scripter = UGameplayStatics::GetAllActorsOfClass((*GWorld), ACUBE_C::StaticClass());
        
            for (auto actor : cube_scripter)
            {
                // Spawn the cube at a specific step
                auto cube = static_cast<ACUBE_C*>(actor);
                cube->New(Starting_Cube_Path_Index-1);
                cube->Next(Starting_Cube_Path_Index);
            }
        }
    });

    new era_mutator("birthday_bus", [](LPVOID Parameters)
    {
        (*GWorld)->GameState()->ChangeBattleBusCosmetic(FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Cosmetics/BattleBuses/BBID_BirthdayBus.BBID_BirthdayBus", true, false));
    });

    new era_mutator("winter_bus", [](LPVOID Parameters)
    {
        (*GWorld)->GameState()->ChangeBattleBusCosmetic(FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Cosmetics/BattleBuses/BBID_WinterBus.BBID_WinterBus", true, false));
    });
    
    new era_mutator("siphon", [](LPVOID Parameters)
    {
        siphon_mutator_params* SiphonParameters = static_cast<siphon_mutator_params*>(Parameters);
        AFortPlayerController* TargetPlayerController = SiphonParameters->Target;
        AFortPlayerController* VictimPlayerController = SiphonParameters->Victim;

        // Add the items to the victim's inventory
        inventory_component::add_item(VictimPlayerController, FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/WoodItemData.WoodItemData"), 50);
        inventory_component::add_item(VictimPlayerController, FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/StoneItemData.StoneItemData"), 50);
        inventory_component::add_item(VictimPlayerController, FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/MetalItemData.MetalItemData"), 50);

        // Health To Apply
        float HealthToApply = 50.0;

        // Health & Shield
        float CurrentHealth = TargetPlayerController->Pawn()->GetHealth();
        float CurrentShield = TargetPlayerController->Pawn()->GetShield();

        CurrentHealth += HealthToApply;
        if (CurrentHealth >= 100)
        {
            HealthToApply = CurrentHealth - 100;
            CurrentHealth = 100;

            if (HealthToApply > 0)
            {
                CurrentShield += HealthToApply;
                if (CurrentShield >= 100)
                {
                    CurrentShield = 100;
                }
            }
        }

        // Apply
        TargetPlayerController->Pawn()->SetHealth(CurrentHealth);
        TargetPlayerController->Pawn()->SetShield(CurrentShield);
    });

    // See what mutators are enabled
    parse_loaded_mutators();
}

void era_mutators_component::parse_loaded_mutators()
{
    auto config = config_manager::get_server_config();

    for (auto& mutator_name_element : config["loaded_mutators"].items())
    {
        if (mutator_name_element.value().is_string())
        {
            std::string mutator_name = mutator_name_element.value();

            enabled_era_mutators.push_back(mutator_name);
        }
    }
}

void era_mutators_component::Execute(std::string Name, void* Params)
{
    if (const auto Iterator = std::find(era_mutators.begin(), era_mutators.end(), Name); Iterator != era_mutators.end())
    {
        // make sure it's enabled
        if (std::find(enabled_era_mutators.begin(), enabled_era_mutators.end(), Name) != enabled_era_mutators.end())
        {
            (*Iterator)->callback(Params);
        }
    }
}
