#pragma once

class elimination_component
{
public:
    elimination_component();

    static uint8_t to_death_cause(AFortPlayerStateAthena* player_state, FGameplayTagContainer gameplay_tags, bool was_dbno);

    static void spectate_on_death();

    inline static std::vector<TWeakObjectPtr<AFortPlayerController>> spectators_array;
};

namespace elimination_component_hooks
{
    // Called when the player is eliminated
    DEFINE_FUNCTION_HOOK(void, ClientOnPawnDied, (UObject* Object, UObject* Function, void* Params));
}

// Remove player from alive player list
inline void (*RemoveFromAlivePlayers)(UObject*, UObject*, uintptr_t a3, uintptr_t a4, uintptr_t a5, char a6, char a7);
inline void (*RemoveFromAlivePlayerList)(UObject*, UObject*, uintptr_t);