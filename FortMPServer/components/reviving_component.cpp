#include "include.hpp"
#include "reviving_component.hpp"

reviving_component::reviving_component()
{
}

void reviving_component::revive_player(AFortPlayerController* player_controller, AFortPlayerPawn* player_to_revive)
{
    if (player_controller == nullptr || player_to_revive == nullptr)
    {
        return;
    }

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

    if (player_to_revive->IsDBNO())
    {
        // Null deathinfo
        ZeroMemory(&player_to_revive->PlayerState()->DeathInfo(), FDeathInfo::GetStructSize());
        
        // Using SetBoolPropertyByName as property changes from boolean -> bit
        UKismetSystemLibrary::SetBoolPropertyByName(player_to_revive, L"bPlayedDying", false);
        UKismetSystemLibrary::SetBoolPropertyByName(player_to_revive, L"bIsDBNO", false);

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

        // Send gameplay event
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(player_to_revive, player_to_revive->EventReviveTag(), NewPayload);

        // Set health
        player_to_revive->SetHealth(30.0f);
    }
}
