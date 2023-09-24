#pragma once

class fort_mutators_component
{
public:

    // Init
    fort_mutators_component();

    static void on_game_phase_updated(uint8_t GamePhase);
    static void ExitCraftOnTimerFinished(AFortAthenaExitCraftSpawner* ExitCraftSpawner, FFrame& StackFrame);

    // Getters
    static AFortGameplayMutator* get_mutator_of_class(UObject* mutator_class);
};

namespace fort_mutators_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, AFortAthenaMutator_Barrier_BeginPlay, (AFortAthenaMutator_Barrier* Target));
    DEFINE_FUNCTION_HOOK(void, AFortAthenaExitCraftSpawner_BeginPlay, (AFortAthenaExitCraftSpawner* Target));
}