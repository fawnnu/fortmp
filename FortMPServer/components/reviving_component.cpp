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

            // TODO: Check if NonReplicatedInstances is even needed?
            
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

        // TODO: Might need SendGameplayEventToActor

        // Set health
        player_to_revive->SetHealth(30.0f);
    }
}
