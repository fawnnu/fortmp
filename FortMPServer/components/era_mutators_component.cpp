#include "include.hpp"
#include "era_mutators_component.hpp"

#include "challenges_component.hpp"
#include "config_manager.hpp"
#include "creative_component.hpp"
#include "director_client.hpp"
#include "inventory_component.hpp"
#include "log_component.hpp"
#include "player_component.hpp"
#include "storm_component.hpp"

era_mutator::era_mutator(std::string in_mutator_name, ExecuteCallback in_callback)
{
    mutator_name = in_mutator_name;
    callback = in_callback;
    era_mutators_component::era_mutators.push_back(this);
}

era_mutators_component::era_mutators_component()
{
    new era_mutator("NoVehicle", [](LPVOID)
    {
    });

    new era_mutator("NoVending", [](LPVOID)
    {
        TArray<AB_Athena_VendingMachine_C*> vending_machine_actors = UGameplayStatics::GetAllActorsOfClass<AB_Athena_VendingMachine_C>(*GWorld, FindObject(L"B_Athena_VendingMachine_C", false, true));
        
        for (AB_Athena_VendingMachine_C* vending_machine_actor : vending_machine_actors)
        {
            vending_machine_actor->K2_DestroyActor();
        }
    });

    new era_mutator("siphon_new", [](LPVOID)
    {
    });
    
    new era_mutator("2024_nye", [](LPVOID)
    {
        CHECK_CREATIVE()
        
        TArray<AActor*> domino_scripters = UGameplayStatics::GetAllActorsOfClass((*GWorld), FindObject(L"/Game/fortmp/content_simple/athena/maps/bp_fortmp_map_loader.bp_fortmp_map_loader_C", true, false));
                            
        for (auto actor : domino_scripters)
        {
            static_cast<Abp_fortmp_map_loader*>(actor)->StartNYE();
        }
    });
    
    new era_mutator("no_cube", [](LPVOID)
    {
    });
    
    new era_mutator("season6_kevin", [](LPVOID)
    {
        if (auto EventScripter = ABP_IslandScripting_C::get_island_scripting())
        {
            EventScripter->CubecrackLeadup();
            EventScripter->CubeBP()->DrainAmount() = 1;
            EventScripter->CubeBP()->BigDrip(true);

            EventScripter->ButterflyStart();
        }
    });
    
    new era_mutator("era_festival_live", [](LPVOID)
    {
        TArray<AActor*> domino_scripters = UGameplayStatics::GetAllActorsOfClass((*GWorld), Adomino_scripting_C::StaticClass());
                                
        for (auto actor : domino_scripters)
        {
            static_cast<Adomino_scripting_C*>(actor)->StartEvent();
        }
    });

    new era_mutator("era_festival_close", [](LPVOID)
    {
        gameserver_client::Client->GameServer_Status.accepting_players = false;
        gameserver_client::Client->SendUpdate();

        player_component::starting_players = gameserver_client::Client->GameServer_Status.number_of_players;

        /*
         * Send a match_started event to director
         */
        if (director_client::client)
        {
            director_client::client->Send(eudp_packet({
                {"type", "match_started"},
                {"payload", {
                    {"starting_players", player_component::starting_players}
                }}
            }));
        }
    });
    
    new era_mutator("butterfly_begin", [](LPVOID)
    {
        // 1 hour storm formation
        (*GWorld)->GameState()->MapInfo()->SafeZoneStartDelay().Value() = 60;
    });

    new era_mutator("domino_event", [](LPVOID)
    {
        // Do nothing
    });
    
    new era_mutator("butterfly_end", [](LPVOID)
    {
        (*GWorld)->GameState()->MapInfo()->SafeZoneStartDelay().Value() = 1;

        // Start safe zone again
        if (StartSafeZone)
        {
            StartSafeZone((*GWorld)->AuthorityGameMode(), 1, 1);
        }
        
        // Remove god from everyone
        for (int i = 0; i < (*GWorld)->NetDriver()->ClientConnections().Num(); i++)
        {
            UNetConnection* ClientConnection = (*GWorld)->NetDriver()->ClientConnections()[i];

            if (!memory_manager::is_bad_read_pointer(ClientConnection) && ClientConnection)
            {
                if (AFortPlayerController* PlayerController = ClientConnection->PlayerController(); !memory_manager::is_bad_read_pointer(PlayerController) && PlayerController)
                {
                    if (AFortPlayerPawn* PlayerPawn = PlayerController->Pawn(); !memory_manager::is_bad_read_pointer(PlayerPawn) && PlayerPawn)
                    {
                        UKismetSystemLibrary::SetBoolPropertyByName(PlayerPawn, L"bIsInvulnerable", false);
                    }
                }
            }
        }
    });
    
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
    
    new era_mutator("cube", [](LPVOID)
    {
        if (GetCL() == 4363240)
        {
            TArray<AActor*> cube_scripter = UGameplayStatics::GetAllActorsOfClass(*GWorld, ACUBE_C::StaticClass());
                                
            for (auto actor : cube_scripter)
            {
                if (cube_mutator::step < 563)
                {
                    static_cast<ACUBE_C*>(actor)->NextStepDynamic();
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

    new cube_mutator("cube_spawn", [](LPVOID Parameters)
    {
        if (ACUBE_C::StaticClass(true))
        {
            // Get the Cube actor
            TArray<AActor*> cube_scripter = UGameplayStatics::GetAllActorsOfClass((*GWorld), ACUBE_C::StaticClass());
        
            for (auto actor : cube_scripter)
            {
                auto cube = static_cast<ACUBE_C*>(actor);
                
                // Spawn the cube at a specific step
                if (cube_mutator::step < 563)
                {
                    cube->New(cube_mutator::step-1);
                    cube->Next(cube_mutator::step);
                }
                else 
                {
                    cube->SetupLake();
                }
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
    
    new era_mutator("holiday_supplydrop", [](LPVOID Parameters)
    {
        (*GWorld)->GameState()->ChangeSupplyDropCosmetic(FindObject<UFortItemDefinition>(L"/Game/Athena/SupplyDrops/AthenaSupplyDrop_Holiday.AthenaSupplyDrop_Holiday_C", true, false));
    });

    new era_mutator("halloween_bus", [](LPVOID Parameters)
    {
        (*GWorld)->GameState()->ChangeBattleBusCosmetic(FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Cosmetics/BattleBuses/BBID_HalloweenBus.BBID_HalloweenBus", true, false));
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

        // Free memory
        delete SiphonParameters;
    });

    // See what mutators are enabled
    parse_loaded_mutators();
}

void era_mutators_component::InitializeHooks()
{
    // Hook
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(era_mutator_hooks::EvaluateCurveTableRow, memory_manager::find_pattern("4C 8B DC 49 89 5B 20 56 48 83 EC 70 45 33 C0 0F 29 74 24 ? 44 38 05 ? ? ? ? 48 8D 05 ? ? ? ? 49 8B F1 49 89 4B C8 49 0F 45 C0 49 89 53 D0 49 89 43 08 0F 28 F2 48 85 C9"), era_mutator_hooks::EvaluateCurveTableRow_Hook);

    if (!era_mutator_hooks::EvaluateCurveTableRow)
    {
        REGISTER_FUNCTION_DETOUR(era_mutator_hooks::EvaluateCurveTableRow, memory_manager::find_pattern("40 53 48 83 EC 30 4C 8B 44 24 ? 49 8B D9 4C 8B 4C 24 ? 0F 28 CA 48 89 4C 24 ? 48 8D 4C 24 ? 48 89 54 24 ? E8 ? ? ? ? 84 C0 0F 94 C0 88 03 48 83 C4 30 5B C3"), era_mutator_hooks::EvaluateCurveTableRow_Hook);
    }
    DetourTransactionCommit();
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


bool era_mutators_component::IsActive(std::string Name)
{
    if (const auto Iterator = std::find(era_mutators.begin(), era_mutators.end(), Name); Iterator != era_mutators.end())
    {
        return std::find(enabled_era_mutators.begin(), enabled_era_mutators.end(), Name) != enabled_era_mutators.end();
    }

    return false;
}

void era_mutator_hooks::EvaluateCurveTableRow_Hook(UCurveTable* CurveTable, FName RowName, float InXY, uint8* OutResult, float* OutXY, FString& ContextString)
{
    static FName SiphonRowName = FName(TEXT("Default.Siphon.Enabled"));

    if (GetFortniteVersionEasy() >= 8.0)
    {
        static FName SiphonMatsRowName = FName(TEXT("Default.Siphon.GrantMatsOnKillEnabled"));
        static FName SiphonWoodRowName = FName(TEXT("Default.Siphon.GrantWoodOnKill"));
        static FName SiphonStoneRowName = FName(TEXT("Default.Siphon.GrantStoneOnKill"));
        static FName SiphonMetalRowName = FName(TEXT("Default.Siphon.GrantMetalOnKill"));
        
        if (RowName == SiphonMatsRowName)
        {
            if (era_mutators_component::IsActive("siphon_new"))
            {
                if (OutResult)
                {
                    *OutResult = 0;
                }
                if (OutXY)
                {
                    *OutXY = 1.0;
                }

                return;
            }
        }
        if (RowName == SiphonWoodRowName)
        {
            if (era_mutators_component::IsActive("siphon_new"))
            {
                if (OutResult)
                {
                    *OutResult = 0;
                }
                if (OutXY)
                {
                    *OutXY = 50.0;
                }

                return;
            }
        }
        if (RowName == SiphonStoneRowName)
        {
            if (era_mutators_component::IsActive("siphon_new"))
            {
                if (OutResult)
                {
                    *OutResult = 0;
                }
                if (OutXY)
                {
                    *OutXY = 50.0;
                }

                return;
            }
        }
        if (RowName == SiphonMetalRowName)
        {
            if (era_mutators_component::IsActive("siphon_new"))
            {
                if (OutResult)
                {
                    *OutResult = 0;
                }
                if (OutXY)
                {
                    *OutXY = 50.0;
                }

                return;
            }
        }
    }
    if (RowName == SiphonRowName)
    {
        if (era_mutators_component::IsActive("siphon_new"))
        {
            if (OutResult)
            {
                *OutResult = 0;
            }
            if (OutXY)
            {
                *OutXY = 1.0;
            }

            if (log_component::enable_debug_logs)
            {
                spdlog::info(L"era_mutator_hooks::EvaluateCurveTableRow_Hook: Overridden 'Default.Siphon.Enabled' [OutResult: {}, OutXY: {}].", OutResult ? *OutResult : 2, OutXY ? *OutXY : -1.0);
            }
            
            return;
        }
    }

    EvaluateCurveTableRow(CurveTable, RowName, InXY, OutResult, OutXY, ContextString);
}