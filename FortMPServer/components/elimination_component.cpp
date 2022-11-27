#include "include.hpp"
#include "elimination_component.hpp"

#include "inventory_component.hpp"
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

void elimination_component_hooks::ClientOnPawnDied_Hook(UObject* Object, UObject* Function, void* Params)
{
    GET_PROCESS_EVENT_PARAMETER(FFortPlayerDeathReport, DeathReport);

    // Victim player controller taken from function
    AFortPlayerController* VictimPlayerController = reinterpret_cast<AFortPlayerController*>(Object);

    // Get playlist
    UFortPlaylistAthena* current_playlist = (*GWorld)->GameState()->HasProperty("CurrentPlaylistData") ?
        (*GWorld)->GameState()->CurrentPlaylistData() :
        (*GWorld)->GameState()->HasProperty("CurrentPlaylistInfo") ?
        (*GWorld)->GameState()->CurrentPlaylistInfo().BasePlaylist() :
        nullptr;
    
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
            if (FindPropertyByName(VictimPlayerController->PlayerState()->DeathInfo().GetClass(), L"Distance"))
            {
                // Get distance from killer pawn to eliminated pawn
                VictimPlayerController->PlayerState()->DeathInfo().Distance() = VictimPlayerController->Pawn()->GetDistanceTo(DeathReport.KillerPawn());
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
    
    // Spawn dead player loot
    auto replicated_entries = TArrayManager<FFortItemEntry>(&VictimPlayerController->WorldInventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());

    static UEnum* EFortInventoryFilter = FindObject<UEnum>(L"/Script/FortniteGame.EFortInventoryFilter");

    if (VictimPlayerController->WorldInventory())
    {
        for (int i = replicated_entries.Num() - 1; i >= 0; i--)
        {
            FFortItemEntry& item_entry = replicated_entries[i];
            
            if (item_entry.ItemDefinition()->FilterOverride() == EFortInventoryFilter->GetEnumIndex(L"Invisible"))
            {
                continue;
            }

            // Remove item from dead player
            inventory_component::remove_item(VictimPlayerController, item_entry.ItemGuid(), -1);
        }
    }

    // only proceed if Respawn isn't enabled
    if (current_playlist->HasProperty(L"RespawnType") == false ||
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

        if ((*GWorld)->GameState()->TeamsLeft() == 1)
        {
        }
 
        if (static bool b_should_spectate = GetEngineVersionEasy() < 422)
        {
            // Start spectating after 5 seconds
            UKismetSystemLibrary::K2_SetTimer(VictimPlayerController, L"SpectateOnDeath", 5, false);

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
    GET_PROCESS_EVENT_PARAMETER(AFortPawn*, FinisherPawn);
    GET_PROCESS_EVENT_PARAMETER(UObject*, FinishingWeapon);
    GET_PROCESS_EVENT_PARAMETER(uint8_t, DeathCause);

    FinisherPawn = reinterpret_cast<AFortPlayerControllerAthena*>(Object)->Pawn();

    if (FinisherPawn && FinisherPawn->CurrentWeapon())
    {
        FinishingWeapon = FinisherPawn->CurrentWeapon()->WeaponData();
    }
}
