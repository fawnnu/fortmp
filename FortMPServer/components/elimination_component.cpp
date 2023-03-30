#include "include.hpp"
#include "elimination_component.hpp"

#include "creative_component.hpp"
#include "function_component.hpp"
#include "inventory_component.hpp"
#include "looting_component.hpp"
#include "pickup_component.hpp"

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

    REGISTER_CUSTOM_UFUNCTION(SpectateKillerPawnOnDeath, AFortPlayerControllerAthena::StaticClass());
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
        { L"Weapon.Ranged.LMG", death_cause_enum->GetEnumIndex(L"Rifle") },
        { L"Weapon.Ranged.Pistol", death_cause_enum->GetEnumIndex(L"Pistol") },
        { L"Weapon.Ranged.Sniper", death_cause_enum->GetEnumIndex(L"Sniper") },
        { L"weapon.ranged.sniper.bolt", death_cause_enum->GetEnumIndex(L"Sniper") },
        { L"Weapon.Ranged.SMG", death_cause_enum->GetEnumIndex(L"SMG") },
        { L"Weapon.Ranged.Crossbow", death_cause_enum->GetEnumIndex(L"Bow") },
        { L"Weapon.Ranged.Minigun", death_cause_enum->GetEnumIndex(L"Minigun") },
        { L"Gameplay.Damage.Environment.Falling", death_cause_enum->GetEnumIndex(L"FallDamage") },
        { L"Gameplay.Damage.OutsideSafeZone", death_cause_enum->GetEnumIndex(L"OutsideSafeZone") },
        { L"Gameplay.Damage.DBNOBleedOut", death_cause_enum->GetEnumIndex(L"DBNOTimeout") }, 
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
                VictimPlayerController->PlayerState()->DeathInfo().Distance() = VictimPlayerController->Pawn()->GetDistanceTo(DeathReport.KillerPawn());
            }

            // AntiCheat
            if (DeathReport.DamageCauser())
            {
                if (DeathReport.DamageCauser()->IsA(AFortProjectileAthena::StaticClass()))
                {
                    if (STATIC_HASPROPERTY(DeathReport.DamageCauser(), FireStartLoc))
                    {
                        auto FireStartLoc = static_cast<AFortProjectileAthena*>(DeathReport.DamageCauser())->FireStartLoc();
                        auto ImpactLoc = VictimPlayerController->Pawn()->K2_GetActorLocation();

                        // Distance
                        auto Distance = sqrtf((FireStartLoc.X - ImpactLoc.X) * (FireStartLoc.X - ImpactLoc.X) +
                              (FireStartLoc.Y - ImpactLoc.Y) * (FireStartLoc.Y - ImpactLoc.Y) +
                              (FireStartLoc.Z - ImpactLoc.Z) * (FireStartLoc.Z - ImpactLoc.Z));

                        // Log distance as meters
                        spdlog::info(L"Player '{}' eliminated opponent using {}. [Distance: {}, Owner: {}]", DeathReport.KillerPawn()->Controller()->MyPlayerInfo()->PlayerName().c_str(), DeathReport.DamageCauser()->GetName().c_str(), Distance * 100, DeathReport.DamageCauser()->GetOwner() ? DeathReport.DamageCauser()->GetOwner()->Class->GetName().c_str() : TEXT("NONE"));

                        if (auto Owner = DeathReport.DamageCauser()->GetOwner())
                        {
                            if (Owner->IsA(AFortWeapon::StaticClass()))
                            {
                                const float Range = static_cast<AFortWeapon*>(DeathReport.DamageCauser()->GetOwner())->GetRange();

                                if (Distance > Range)
                                {
                                    spdlog::warn(L"Player '{}' eliminated an enemy with a too long distance. Kicking...", DeathReport.KillerPawn()->Controller()->MyPlayerInfo()->PlayerName().c_str());

                                    DeathReport.KillerPawn()->Controller()->ClientReturnToMainMenu(TEXT(""));
                                }
                            }
                        }
                    }
                }
                
                if (DeathReport.DamageCauser()->IsA(AFortWeapon::StaticClass()))
                {
                    const float Range = static_cast<AFortWeapon*>(DeathReport.DamageCauser())->GetRange();
                    
                    if (Range > 0 && VictimPlayerController->Pawn()->GetDistanceTo(DeathReport.KillerPawn()) > Range)
                    {
                        spdlog::warn(L"Player '{}' eliminated an enemy with a too long distance. Kicking...", DeathReport.KillerPawn()->Controller()->MyPlayerInfo()->PlayerName().c_str());

                        DeathReport.KillerPawn()->Controller()->ClientReturnToMainMenu(TEXT(""));
                    }

                    if (auto ItemDefinition = static_cast<AFortWeapon*>(DeathReport.DamageCauser())->WeaponData())
                    {
                        if (DeathReport.LethalDamage() > looting_component::get_best_damage(ItemDefinition))
                        {
                            spdlog::warn(L"Player '{}' eliminated an enemy with a hotfixed item... [LethalDamage: {}, Best Possible Damage: {}, ItemDefinition: {}]", DeathReport.KillerPawn()->Controller()->MyPlayerInfo()->PlayerName().c_str(), DeathReport.LethalDamage(), looting_component::get_best_damage(ItemDefinition), ItemDefinition->GetName().c_str());

                            DeathReport.KillerPawn()->Controller()->ClientReturnToMainMenu(TEXT(""));
                        }
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

        // Update finisher/downer in kill feed
        VictimPlayerController->PlayerState()->DeathInfo().FinisherOrDowner() = DeathReport.KillerPlayerState();
    }

    // Update DBNO status
    VictimPlayerController->PlayerState()->DeathInfo().bDBNO() = VictimPlayerController->Pawn()->IsDBNO();

    // Get death cause
    VictimPlayerController->PlayerState()->DeathInfo().DeathCause() = elimination_component::to_death_cause(VictimPlayerController->PlayerState(), DeathReport.Tags(), VictimPlayerController->Pawn()->IsDBNO());

    // Mark death report as initialized
    if (FindPropertyByName(VictimPlayerController->PlayerState()->DeathInfo().GetClass(), L"bInitialized"))
    {
        VictimPlayerController->PlayerState()->DeathInfo().bInitialized() = true;
    }

    // Set the deathlocation
    if (FindPropertyByName(VictimPlayerController->PlayerState()->DeathInfo().GetClass(), L"DeathLocation"))
    {
        VictimPlayerController->PlayerState()->DeathInfo().DeathLocation() = VictimPlayerController->Pawn()->K2_GetActorLocation();
    }

    if(STATIC_HASPROPERTY(VictimPlayerController->PlayerState(), PawnDeathLocation))
    {
        VictimPlayerController->PlayerState()->PawnDeathLocation() = VictimPlayerController->Pawn()->K2_GetActorLocation();
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
        else
        {
            // Clear primary inventory as always if creative is enabled and there's no special mutators
            inventory_component::clear_primary_inventory(VictimPlayerController, true);   
        }
    }

    // Current weapon list crash fix
    if (VictimPlayerController->Pawn())
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

void elimination_component_hooks::ClientNotifyWon_Hook(UObject* Object, UObject* Function, void* Params)
{
    if (FindPropertyByName(Function, TEXT("FinisherPawn")) &&
        FindPropertyByName(Function, TEXT("FinishingWeapon")))
    {
        GET_PROCESS_EVENT_PARAMETER(AFortPawn*, FinisherPawn);
        GET_PROCESS_EVENT_PARAMETER(UObject*, FinishingWeapon);

        FinisherPawn = reinterpret_cast<AFortPlayerControllerAthena*>(Object)->Pawn();

        if (FinisherPawn && FinisherPawn->CurrentWeapon())
        {
            FinishingWeapon = FinisherPawn->CurrentWeapon()->WeaponData();
        }
    }
}
