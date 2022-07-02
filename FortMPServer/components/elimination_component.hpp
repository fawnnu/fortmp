#pragma once

class elimination_component
{
public:
    elimination_component();

    static uint8_t to_death_cause(AFortPlayerStateAthena* player_state, FGameplayTagContainer gameplay_tags, bool was_dbno);
};

namespace elimination_component_hooks
{
    // Called when the player is eliminated
    DEFINE_FUNCTION_HOOK(void, ClientOnPawnDied, (UObject* Object, UObject* Function, void* Params));
}