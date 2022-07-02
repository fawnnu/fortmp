#include "include.hpp"
#include "elimination_component.hpp"

elimination_component::elimination_component()
{
    REGISTER_PROCESS_EVENT_HOOK(L"/Script/FortniteGame.FortPlayerControllerZone.ClientOnPawnDied", elimination_component_hooks::ClientOnPawnDied_Hook);
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
        { L"Weapon.Ranged.Assault", death_cause_enum->GetEnumIndex(L"Rifle") },
        { L"Weapon.Ranged.LMG", death_cause_enum->GetEnumIndex(L"Rifle") },
        { L"Weapon.Ranged.Pistol", death_cause_enum->GetEnumIndex(L"Pistol") },
        { L"Weapon.Ranged.Sniper", death_cause_enum->GetEnumIndex(L"Sniper") },
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
            // Get distance from killer pawn to eliminated pawn
            VictimPlayerController->PlayerState()->DeathInfo().Distance() = VictimPlayerController->Pawn()->GetDistanceTo(DeathReport.KillerPawn());
        }

        // Update finisher/downer in kill feed
        VictimPlayerController->PlayerState()->DeathInfo().FinisherOrDowner() = DeathReport.KillerPlayerState();
    }

    // Update DBNO status
    VictimPlayerController->PlayerState()->DeathInfo().bDBNO() = VictimPlayerController->Pawn()->IsDBNO();

    // Get death cause
    VictimPlayerController->PlayerState()->DeathInfo().DeathCause() = elimination_component::to_death_cause(VictimPlayerController->PlayerState(), DeathReport.Tags(), VictimPlayerController->Pawn()->IsDBNO());
}