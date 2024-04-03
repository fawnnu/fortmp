#include "include.hpp"
#include "reviving_component.hpp"

#include "ability_component.hpp"
#include "challenges_component.hpp"
#include "player_component.hpp"

reviving_component::reviving_component()
{
}

void reviving_component::revive_player(AFortPlayerController* player_controller, AFortPlayerPawn* player_to_revive, bool bForceRevive)
{
    if (player_controller == nullptr || player_to_revive == nullptr)
    {
        return;
    }

    if (bForceRevive == false)
    {
        if (player_controller->PlayerState()->TeamIndex() != player_to_revive->PlayerState()->TeamIndex())
        {
            // Only allow players to revive teammates
            return;
        }

        if (player_controller->Pawn() == player_to_revive)
        {
            // Do not allow players to revive themselves
            return;
        }
    }
    
    if (player_to_revive->IsDBNO())
    {
        // Above Season 6, completely clear deathinfo
        if (GetFortniteVersionEasy() >= 6)
        {
            ZeroMemory(&player_to_revive->PlayerState()->DeathInfo(), FDeathInfo::GetStructSize());
        }
    
        // Using SetBoolPropertyByName as property changes from boolean -> bit
        UKismetSystemLibrary::SetBoolPropertyByName(player_to_revive, L"bPlayedDying", false);
        UKismetSystemLibrary::SetBoolPropertyByName(player_to_revive, L"bIsDBNO", false);
        UKismetSystemLibrary::SetBoolPropertyByName(player_to_revive, L"bIsHiddenForDeath", false);
        UKismetSystemLibrary::SetBoolPropertyByName(player_to_revive, L"bIsDying", false);
        UKismetSystemLibrary::SetBoolPropertyByName(player_to_revive, L"bWasDBNOOnDeath", false);

        auto items = TArrayManager<FGameplayAbilitySpec>(&player_to_revive->AbilitySystemComponent()->ActivatableAbilities().Items(), FGameplayAbilitySpec::GetStructSize());
        
        for (FGameplayAbilitySpec& ability_spec : items)
        {
            for (UGameplayAbility* replicated_instance : ability_spec.ReplicatedInstances)
            {
                // Easier way to check instead of checking gameplay tags
                const bool is_dbno_ability = wcsstr(replicated_instance->GetName().c_str(), L"DBNO");

                if (is_dbno_ability)
                {
                    // Cancel ability if it matches
                    replicated_instance->K2_CancelAbility();
                }
            }
            
            for (UGameplayAbility* non_replicated_instance : ability_spec.NonReplicatedInstances)
            {
                // Easier way to check instead of checking gameplay tags
                const bool is_dbno_ability = wcsstr(non_replicated_instance->GetName().c_str(), L"DBNO");

                if (is_dbno_ability)
                {
                    // Cancel ability if it matches
                    non_replicated_instance->K2_CancelAbility();
                }
            }
        }

        // Create payload
        auto NewPayload = FGameplayEventData::Create();
        NewPayload->Instigator() = player_controller->Pawn();
        NewPayload->Target() = player_to_revive;
        NewPayload->EventTag() = player_to_revive->EventReviveTag();
        NewPayload->EventMagnitude() = 1.0f;

        // Send gameplay event
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(player_to_revive, player_to_revive->EventReviveTag(), NewPayload);

        if (bForceRevive == false)
        {
            // Send event to backend
            challenges_component::SendRevive(player_controller, player_to_revive->Controller());
        }
        
        // Update tag map
        FGameplayTag StatusTag = FGameplayTag(TEXT("Gameplay.Status.DBNOBeingRevived"));
        ability_component::UpdateTagMap(player_to_revive->AbilitySystemComponent(), StatusTag, -1);

        // Remove cue
        FGameplayTag CueTag = FGameplayTag(TEXT("GameplayCue.Athena.Player.BeingRevivedFromDBNO"));
        ability_component::RemoveGameplayCue(player_to_revive->AbilitySystemComponent(), CueTag);
        
        // Set health
        player_to_revive->SetHealth(30.0f);
        player_to_revive->SetMaxHealth(player_to_revive->GetMaxHealth());

        // Bump revives in player stats
        static_cast<AFortPlayerControllerAthena*>(player_controller)->EraPlayerInfo().PlayerStatsTracker.Revives++;

        if (FindPropertyByName(FDeathInfo::GetClass(), TEXT("DeathTags")))
        {
            // Free DeathTags arrays
            player_to_revive->PlayerState()->DeathInfo().DeathTags().GameplayTags.Free();
            player_to_revive->PlayerState()->DeathInfo().DeathTags().ParentTags.Free();

            // Reset DeathInfo
            ZeroMemory(&player_to_revive->PlayerState()->DeathInfo(), FDeathInfo::GetStructSize());
        }
    }
}
