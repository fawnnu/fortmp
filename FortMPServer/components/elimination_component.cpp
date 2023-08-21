#include "include.hpp"
#include "elimination_component.hpp"

#include "challenges_component.hpp"
#include "creative_component.hpp"
#include "era_mutators_component.hpp"
#include "function_component.hpp"
#include "inventory_component.hpp"
#include "looting_component.hpp"
#include "pickup_component.hpp"
#include "player_component.hpp"
#include "team_component.hpp"

elimination_component::elimination_component()
{
    if (GetEngineVersionEasy() < 421)
    {
        // Setup function with original arguments if below version 5.00
        INIT_POINTER(RemoveFromAlivePlayerList, memory_manager::find_function_by_string(L"FortGameModeAthena: Player [%s] removed"));
    }
    else
    {
        // Setup function with new arguments if on a newer build
        INIT_POINTER(RemoveFromAlivePlayers, memory_manager::find_function_by_string(L"FortGameModeAthena: Player [%s] removed"));
    }
    
    REGISTER_PROCESS_EVENT_DETOUR(L"/Script/FortniteGame.FortPlayerControllerZone.ClientOnPawnDied", elimination_component_hooks::ClientOnPawnDied_Hook);
    REGISTER_PROCESS_EVENT_DETOUR(L"/Script/FortniteGame.FortPlayerControllerAthena.ClientNotifyWon", elimination_component_hooks::ClientNotifyWon_Hook);
    REGISTER_PROCESS_EVENT_DETOUR(L"/Script/FortniteGame.FortPlayerControllerAthena.ClientNotifyTeamWon", elimination_component_hooks::ClientNotifyTeamWon_Hook);

    REGISTER_CUSTOM_UFUNCTION(SpectateKillerPawnOnDeath, AFortPlayerControllerAthena::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(SendEndMatchStats, AFortPlayerControllerAthena::StaticClass());
}

uint8_t elimination_component::to_death_cause(AFortPlayerStateAthena* player_state, FGameplayTagContainer tag_container, bool was_dbno)
{
    if (player_state->HasFunction(L"ToDeathCause"))
    {
        // Let the game decide the death cause if it can
        return player_state->ToDeathCause(tag_container, was_dbno);
    }

    // Enum containing all possible death reasons
    static UEnum* death_cause_enum = FindObject<UEnum>(L"/Script/FortniteGame.EDeathCause");

    static std::map<FName, uint8_t> death_cause_map =
    {
        { L"Weapon.Ranged.Shotgun", death_cause_enum->GetEnumIndex(L"Shotgun") },
        { L"Weapon.Ranged.Shotgun.Pump", death_cause_enum->GetEnumIndex(L"Shotgun") },
        { L"Weapon.Ranged.Shotgun.Tactical", death_cause_enum->GetEnumIndex(L"Shotgun") },
        { L"Weapon.Ranged.Assault", death_cause_enum->GetEnumIndex(L"Rifle") },
        { L"weapon.ranged.assault.burst", death_cause_enum->GetEnumIndex(L"Rifle") },
        { L"weapon.ranged.assault.standard", death_cause_enum->GetEnumIndex(L"Rifle") },
        { L"weapon.ranged.assault.silenced", death_cause_enum->GetEnumIndex(L"Rifle") },
        { L"Weapon.Ranged.LMG", death_cause_enum->GetEnumIndex(L"LMG") },
        { L"Weapon.Ranged.Pistol", death_cause_enum->GetEnumIndex(L"Pistol") },
        { L"Weapon.Ranged.Sniper", death_cause_enum->GetEnumIndex(L"Sniper") },
        { L"weapon.ranged.sniper.bolt", death_cause_enum->GetEnumIndex(L"Sniper") },
        { L"Weapon.Ranged.SMG", death_cause_enum->GetEnumIndex(L"SMG") },
        { L"Weapon.Ranged.Crossbow", death_cause_enum->GetEnumIndex(L"Bow") },
        { L"Weapon.Ranged.Minigun", death_cause_enum->GetEnumIndex(L"Minigun") },
        { L"Gameplay.Damage.Environment.Falling", death_cause_enum->GetEnumIndex(L"FallDamage") },
        { L"Gameplay.Damage.OutsideSafeZone", death_cause_enum->GetEnumIndex(L"OutsideSafeZone") },
        { L"Gameplay.Damage.DBNOBleedOut", death_cause_enum->GetEnumIndex(L"DBNOTimeout") },
        { L"Weapon.Melee.Massive.Charge", death_cause_enum->GetEnumIndex(L"MassiveMelee") },
        { L"weapon.melee.Massive", death_cause_enum->GetEnumIndex(L"MassiveMelee") },
        { L"Weapon.Melee.Massive.Divebomb", death_cause_enum->GetEnumIndex(L"MassiveDiveBomb") },
        { L"Weapon.Melee.Impact.Pickaxe", death_cause_enum->GetEnumIndex(L"Melee") },
        { L"Weapon.Ranged.Massive", death_cause_enum->GetEnumIndex(L"MassiveRanged") },
        { L"weapon.ranged.heavy.rocket_launcher",  death_cause_enum->GetEnumIndex(L"RocketLauncher")},
        { L"weapon.ranged.heavy.grenade_launcher",  death_cause_enum->GetEnumIndex(L"GrenadeLauncher")},
        { L"Weapon.Ranged.Grenade.Gas", death_cause_enum->GetEnumIndex(L"GasGrenade")},
        { L"Weapon.ranged.heavy.grenade", death_cause_enum->GetEnumIndex(L"Grenade")},
        { L"trap.floor", death_cause_enum->GetEnumIndex(L"Trap") },
        { L"trap.wall", death_cause_enum->GetEnumIndex(L"Trap") },
        { L"trap.ceiling", death_cause_enum->GetEnumIndex(L"Trap") },
        { L"trap.hostile", death_cause_enum->GetEnumIndex(L"Trap") }
    };

    for (FGameplayTag gameplay_tag : tag_container.GameplayTags)
    {
        if (death_cause_map[gameplay_tag.TagName])
        {
            // Return a death cause if we have it mapped
            return death_cause_map[gameplay_tag.TagName];
        }
    }

    return death_cause_enum->GetEnumIndex(L"Unspecified");
}

int elimination_component::get_placement(AFortPlayerController* PlayerController)
{
    // HANDLE PLACEMENT FOR SUMMARY EVENT
    if (AFortTeamInfo* TeamInfo = team_component::find_team(PlayerController))
    {
        bool bIsTeamAlive = false;

        for (auto TeamMember : TeamInfo->TeamMembers())
        {
            if ((*GWorld)->AuthorityGameMode()->AlivePlayers().Find(static_cast<AFortPlayerController*>(TeamMember)))
            {
                bIsTeamAlive = true;
                break;
            }
        }

        // If everyone on the team is dead, we are ready to calculate the placement
        if (bIsTeamAlive == false)
        {
            // Get alive teams to calculate placement
            return team_component::get_alive_teams() + 1;
        }
    }
    
    return 0;
}

void elimination_component::SpectateKillerPawnOnDeath(AFortPlayerControllerAthena* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self == nullptr)
    {
        return;
    }

    if (AFortPlayerStateAthena* FinisherOrDowner = static_cast<AFortPlayerStateAthena*>(Self->PlayerState()->DeathInfo().FinisherOrDowner()))
    {
        if (FinisherOrDowner->Owner())
        {
            if (AFortPlayerPawn* Pawn = reinterpret_cast<AFortPlayerController*>(FinisherOrDowner->Owner())->Pawn())
            {
                if (STATIC_HASPROPERTY(Self, PlayerToSpectateOnDeath))
                {
                    Self->PlayerToSpectateOnDeath() = Pawn;

                    Self->SpectateOnDeath();
                    
                    return;
                }
            }
        }
    }

    // If no Target pawn was found, just call SpectateOnDeath
    Self->SpectateOnDeath();
}

void elimination_component_hooks::ClientOnPawnDied_Hook(UObject* Object, UObject* Function, void* Params)
{
    GET_PROCESS_EVENT_PARAMETER(FFortPlayerDeathReport, DeathReport);

    // Victim player controller taken from function
    AFortPlayerController* VictimPlayerController = reinterpret_cast<AFortPlayerController*>(Object);

    // Get playlist
    UFortPlaylistAthena* current_playlist = GetPlaylistEasy();

    // Do nothing if playerstate is a bad ptr
    if (memory_manager::is_bad_read_pointer(VictimPlayerController->PlayerState()))
    {
        spdlog::error(L"ClientOnPawnDied_Hook: bad pointer playerstate");
        
        return;
    }

    // Check if anything is a bad pointer but not a nullptr
    if (memory_manager::is_bad_read_pointer(DeathReport.KillerPlayerState(), false) && DeathReport.KillerPlayerState())
    {
        DeathReport.KillerPlayerState() = nullptr;
    }
    if (memory_manager::is_bad_read_pointer(DeathReport.DamageCauser(), false) && DeathReport.DamageCauser())
    {
        DeathReport.DamageCauser() = nullptr;
    }
    if (memory_manager::is_bad_read_pointer(DeathReport.KillerPawn(), false) && DeathReport.KillerPawn())
    {
        DeathReport.KillerPawn() = nullptr;
    }

    if (!memory_manager::is_bad_read_pointer(DeathReport.KillerPlayerState()) &&
        !memory_manager::is_bad_read_pointer(VictimPlayerController->PlayerState()))
    {
        if (DeathReport.KillerPlayerState())
        {
            if (DeathReport.KillerPlayerState()->TeamIndex() != VictimPlayerController->PlayerState()->TeamIndex())
            {
                // Credit elimination if not on the same team
                DeathReport.KillerPlayerState()->ClientReportKill(VictimPlayerController->PlayerState());
                DeathReport.KillerPlayerState()->KillScore()++;
            }

            if (DeathReport.KillerPawn())
            {
                // Siphon
                siphon_mutator_params* SiphonParams = new siphon_mutator_params;
                SiphonParams->Target = DeathReport.KillerPawn()->Controller();
                SiphonParams->Victim = VictimPlayerController;
                era_mutators_component::Execute("siphon", SiphonParams);
                                
                FGameplayEventData* GameplayEventData = FGameplayEventData::Create();
                GameplayEventData->Instigator() = DeathReport.KillerPawn()->Controller();
                GameplayEventData->Target() = DeathReport.KillerPawn();
                GameplayEventData->OptionalObject() = DeathReport.KillerPawn();
                GameplayEventData->OptionalObject2() = DeathReport.KillerPawn();
                GameplayEventData->EventTag().TagName = FName(L"Event.EarnedElimination");
                GameplayEventData->EventMagnitude() = 1.f;

                UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(DeathReport.KillerPawn(), FGameplayTag(FName(L"Event.EarnedElimination")), GameplayEventData);
            
                if (FindPropertyByName(VictimPlayerController->PlayerState()->DeathInfo().GetClass(), L"Distance"))
                {
                    // Get distance from killer pawn to eliminated pawn
                    if (!memory_manager::is_bad_read_pointer(VictimPlayerController->Pawn()) && VictimPlayerController->Pawn())
                    {
                        VictimPlayerController->PlayerState()->DeathInfo().Distance() = VictimPlayerController->Pawn()->GetDistanceTo(DeathReport.KillerPawn());
                    }
                }

                // AntiCheat
                if (!memory_manager::is_bad_read_pointer(VictimPlayerController->Pawn()) && VictimPlayerController->Pawn())
                {
                    if (DeathReport.DamageCauser())
                    {
                        if (DeathReport.DamageCauser()->IsA(AFortWeapon::StaticClass()))
                        {
                            const float Range = static_cast<AFortWeapon*>(DeathReport.DamageCauser())->GetRange();
                    
                            if (Range > 0 && VictimPlayerController->Pawn()->GetDistanceTo(DeathReport.KillerPawn()) > Range)
                            {
                                spdlog::warn(L"Player '{}' eliminated an enemy with a too long distance. Kicking...", DeathReport.KillerPawn()->Controller()->MyPlayerInfo()->PlayerName().c_str());

                                DeathReport.KillerPawn()->Controller()->ClientReturnToMainMenu(TEXT(""));
                            }

                            if (Range <= 0)
                            {
                                if (VictimPlayerController->Pawn()->GetDistanceTo(DeathReport.KillerPawn()) > 1000)
                                {
                                    spdlog::warn(L"Player '{}' eliminated an enemy with a too long distance. Kicking...", DeathReport.KillerPawn()->Controller()->MyPlayerInfo()->PlayerName().c_str());

                                    DeathReport.KillerPawn()->Controller()->ClientReturnToMainMenu(TEXT(""));
                                }
                            }
                        }
                    }
                }
            }

            // Update finisher/downer in kill feed
            VictimPlayerController->PlayerState()->DeathInfo().FinisherOrDowner() = DeathReport.KillerPlayerState();
        }
    }

    // Make sure PlayerState is valid
    if (!memory_manager::is_bad_read_pointer(VictimPlayerController->PlayerState()))
    {
        // Update DBNO status
        if (!memory_manager::is_bad_read_pointer(VictimPlayerController->Pawn()) && VictimPlayerController->Pawn())
        {
            VictimPlayerController->PlayerState()->DeathInfo().bDBNO() = VictimPlayerController->Pawn()->IsDBNO();
        }
    
        // Get death cause
        if (!memory_manager::is_bad_read_pointer(VictimPlayerController->Pawn()) && VictimPlayerController->Pawn())
        {
            VictimPlayerController->PlayerState()->DeathInfo().DeathCause() = elimination_component::to_death_cause(VictimPlayerController->PlayerState(), DeathReport.Tags(), VictimPlayerController->Pawn()->IsDBNO());
        }
    
        // Mark death report as initialized
        if (FindPropertyByName(VictimPlayerController->PlayerState()->DeathInfo().GetClass(), L"bInitialized"))
        {
            VictimPlayerController->PlayerState()->DeathInfo().bInitialized() = true;
        }

        // Add death tags
        if (FindPropertyByName(VictimPlayerController->PlayerState()->DeathInfo().GetClass(), L"DeathTags"))
        {
            // copy from deathreport.tags()
            for (FGameplayTag GameplayTag : DeathReport.Tags().GameplayTags)
            {
                VictimPlayerController->PlayerState()->DeathInfo().DeathTags().GameplayTags.Add(GameplayTag);
            }
            for (FGameplayTag ParentTag : DeathReport.Tags().ParentTags)
            {
                VictimPlayerController->PlayerState()->DeathInfo().DeathTags().ParentTags.Add(ParentTag);
            }
        }

        // Set the deathlocation
        if (FindPropertyByName(VictimPlayerController->PlayerState()->DeathInfo().GetClass(), L"DeathLocation"))
        {
            if (!memory_manager::is_bad_read_pointer(VictimPlayerController->Pawn()) && VictimPlayerController->Pawn())
            {
                VictimPlayerController->PlayerState()->DeathInfo().DeathLocation() = VictimPlayerController->Pawn()->K2_GetActorLocation();
            }
        }

        if (STATIC_HASPROPERTY(VictimPlayerController->PlayerState(), PawnDeathLocation))
        {
            if (!memory_manager::is_bad_read_pointer(VictimPlayerController->Pawn()) && VictimPlayerController->Pawn())
            {
                VictimPlayerController->PlayerState()->PawnDeathLocation() = VictimPlayerController->Pawn()->K2_GetActorLocation();
            }
        }
    
        // Spawn dead player loot
        if (VictimPlayerController->WorldInventory())
        {
            if(creative_component::is_creative_enabled())
            {
                if (STATIC_HASPROPERTY(VictimPlayerController, OwnedPortal))
                {
                    if (static_cast<AFortPlayerControllerAthena*>(VictimPlayerController)->OwnedPortal() != nullptr)
                    {
                        if (auto Mutator = (*GWorld)->AuthorityGameMode()->GetMutatorByClass(static_cast<AFortPlayerControllerAthena*>(VictimPlayerController)->OwnedPortal(), AFortAthenaMutator_InventoryOverride::StaticClass()))
                        {
                            AFortAthenaMutator_InventoryOverride* mutator_inventory_override = static_cast<AFortAthenaMutator_InventoryOverride*>(Mutator);
                            if (mutator_inventory_override->DropAllItemsOverride() != 2)
                            {
                                inventory_component::clear_primary_inventory(VictimPlayerController, true);
                            }
                        }   
                    }
                }   
            }
            else if (elimination_component::keep_inventory_after_death == false)
            {
                // Clear primary inventory as always if creative is enabled and there's no special mutators
                inventory_component::clear_primary_inventory(VictimPlayerController, true);   
            }
        }

        // Current weapon list crash fix
        if (!memory_manager::is_bad_read_pointer(VictimPlayerController->Pawn()) && VictimPlayerController->Pawn())
        {
            static void* currentweaponlist_property = FindPropertyByName(VictimPlayerController->Pawn()->Class, L"CurrentWeaponList");

            if (currentweaponlist_property && VictimPlayerController->Pawn()->CurrentWeaponList().Num() > 0)
            {
                for (auto i = VictimPlayerController->Pawn()->CurrentWeaponList().Num() - 1; i >= 0; i--)
                {
                    // Clear the array
                    GenericArray_Remove(&VictimPlayerController->Pawn()->CurrentWeaponList(), currentweaponlist_property, i);
                }
            }
        }
    }
    
    // only proceed if Respawn isn't enabled
    if (current_playlist == nullptr ||
        current_playlist->HasProperty(L"RespawnType") == false ||
        current_playlist->HasProperty(L"RespawnType") == true &&
        current_playlist->RespawnType() == 0)
    {
        if (RemoveFromAlivePlayerList)
        {
            // Remove player from alive list if function exists
            RemoveFromAlivePlayerList((*GWorld)->AuthorityGameMode(), VictimPlayerController, 0);
        }
        
        if (RemoveFromAlivePlayers)
        {
            // Remove player from alive list if function exists
            RemoveFromAlivePlayers((*GWorld)->AuthorityGameMode(), VictimPlayerController, 0, 0, 0, 1, 0);
        }

#ifdef _RELEASE
        // Summary event
        if (int placement = elimination_component::get_placement(VictimPlayerController); placement > 0)
        {
            // Send placement for the whole team
            if (AFortTeamInfo* TeamInfo = team_component::find_team(VictimPlayerController))
            {
                for (AController* TeamMember : TeamInfo->TeamMembers())
                {
                    // Send summary
                    challenges_component::SendSummary(static_cast<AFortPlayerController*>(TeamMember), placement);

                    // Set placement
                    player_component::PlayerStatsMap[static_cast<AFortPlayerController*>(TeamMember)].Placement = placement;
                }
            }
        }

        // Make sure PlayerState is valid
        if (!memory_manager::is_bad_read_pointer(VictimPlayerController->PlayerState()))
        {
            // Send stats
            elimination_component::DelaySendEndMatchStats(VictimPlayerController);

            // Send elimination event to the killer
            if (DeathReport.KillerPawn())
            {
                if (AFortPlayerController* KillerPlayerController = DeathReport.KillerPawn()->Controller())
                {
                    challenges_component::SendElimination(KillerPlayerController, VictimPlayerController, VictimPlayerController->PlayerState()->DeathInfo().DeathCause(), DeathReport.Tags());
                }
            }
        }
        
        // Notify Era Services
        gameserver_client::Client->GameServer_Status.number_of_players = (*GWorld)->AuthorityGameMode()->AlivePlayers().Num();
        gameserver_client::Client->SendUpdate();
#endif
        
        // only spectate where spectating is stripped
        if (GetEngineVersionEasy() < 422)
        {
            // Start spectating after 5 seconds
            UKismetSystemLibrary::K2_SetTimer(VictimPlayerController, L"SpectateKillerPawnOnDeath", 5, false);

            // Spectate on other clients
            for (int i = 0; i < (*GWorld)->NetDriver()->ClientConnections().Num(); i++)
            {
                UNetConnection* Connection = (*GWorld)->NetDriver()->ClientConnections()[i];

                if (Connection == nullptr || Connection->PlayerController() == nullptr)
                {
                    continue;
                }

                if (!(*GWorld)->AuthorityGameMode()->AlivePlayers().Find(Connection->PlayerController()) &&
                    Connection->PlayerController() != VictimPlayerController)
                {
                    // If the player is dead, update spectating
                    UKismetSystemLibrary::K2_SetTimer(Connection->PlayerController(), L"SpectateOnDeath", 5, false);
                }
            }
        }
    }
}

void elimination_component::NotifyWon(UObject* Object, UObject* Function, void* Params)
{
    AFortPlayerControllerAthena* Controller = reinterpret_cast<AFortPlayerControllerAthena*>(Object);

    if (Controller)
    {
        if (FindPropertyByName(Function, TEXT("FinisherPawn")) &&
            FindPropertyByName(Function, TEXT("FinishingWeapon")))
        {
            GET_PROCESS_EVENT_PARAMETER(AFortPawn*, FinisherPawn);
            GET_PROCESS_EVENT_PARAMETER(UObject*, FinishingWeapon);
        
            FinisherPawn = Controller->Pawn();

            if (FinisherPawn && FinisherPawn->CurrentWeapon())
            {
                FinishingWeapon = FinisherPawn->CurrentWeapon()->WeaponData();
            }
        }

        // send a summary event for the winner
        CreateThread(nullptr, 0, [](LPVOID TeamMember) -> DWORD
        {
            Sleep(1000);
                    
            challenges_component::SendSummary(static_cast<AFortPlayerController*>(TeamMember), 1);
                        
            return 0;
        }, Controller, 0, nullptr);
    }   
}

void elimination_component::DelaySendEndMatchStats(AFortPlayerController* PlayerController)
{
    if (PlayerController)
    {
        // Send stats with a delay
        UKismetSystemLibrary::K2_SetTimer(PlayerController, L"SendEndMatchStats", 1, false);
    }
}

void elimination_component::SendEndMatchStats(AFortPlayerControllerAthena* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (FAthenaMatchStats::GetClass())
    {
        // Tell the client that the match ended
        FAthenaMatchStats* NewMatchStats = new FAthenaMatchStats;

        // Stats
        if (player_component::PlayerStatsMap.contains(Self))
        {
            if (FindPropertyByName(FAthenaMatchStats::GetClass(), TEXT("Stats")))
            {
                if (!memory_manager::is_bad_read_pointer(Self->PlayerState()) && Self->PlayerState())
                {
                    NewMatchStats->Stats()[3] = Self->PlayerState()->KillScore();
                }
                NewMatchStats->Stats()[7] = player_component::PlayerStatsMap[Self].Revives;
                NewMatchStats->Stats()[8] = player_component::PlayerStatsMap[Self].DamageTaken;
                NewMatchStats->Stats()[9] = player_component::PlayerStatsMap[Self].DistanceTraveled;
                NewMatchStats->Stats()[11] = player_component::PlayerStatsMap[Self].MaterialsGathered;
                NewMatchStats->Stats()[13] = player_component::PlayerStatsMap[Self].MaterialsUsed;
            }
            
            // Weapon stats
            if (FAthenaWeaponStats::GetClass())
            {
                FAthenaWeaponStats* NewWeaponStats = new FAthenaWeaponStats;
                if (FindPropertyByName(FAthenaWeaponStats::GetClass(), TEXT("Stats")))
                {
                    NewWeaponStats->Stats()[3] = player_component::DamageDealtMap[Self].GetPlayerDamage();
                    NewWeaponStats->Stats()[5] = player_component::DamageDealtMap[Self].StructureDamage;
                    NewWeaponStats->Stats()[8] = player_component::PlayerStatsMap[Self].Hits;
                    NewWeaponStats->Stats()[11] = player_component::PlayerStatsMap[Self].HeadShots;
                    if (FindPropertyByName(FAthenaMatchStats::GetClass(), TEXT("WeaponStats")))
                    {
                        TArrayManager<FAthenaWeaponStats>(&NewMatchStats->WeaponStats(), FAthenaWeaponStats::GetStructSize()).Add(NewWeaponStats);
                    }
                    free(NewWeaponStats);
                }
            }
        }
        Self->ClientSendMatchStatsForPlayer(NewMatchStats);
        free(NewMatchStats);
    }
}

void elimination_component_hooks::ClientNotifyWon_Hook(UObject* Object, UObject* Function, void* Params)
{
    elimination_component::NotifyWon(Object, Function, Params);
    elimination_component::DelaySendEndMatchStats(reinterpret_cast<AFortPlayerControllerAthena*>(Object));
}

void elimination_component_hooks::ClientNotifyTeamWon_Hook(UObject* Object, UObject* Function, void* Params)
{
    elimination_component::NotifyWon(Object, Function, Params);
}
