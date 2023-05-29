#pragma once

class bots_component
{
public:
    bots_component();
    
    // Spawns an agent
    static AFortPlayerPawnAthena* spawn_agent(UFortAthenaAIBotCustomizationData* customization_data, FVector spawn_location, FRotator spawn_rotation);

    static void initialize_data();

    inline static AFortAthenaMutator_Bots* mutator_bots = nullptr;
};

namespace bots_component_hooks
{
    // Called when a bot dies
    DEFINE_FUNCTION_HOOK(void, OnPossesedPawnDied, (UObject* Object, UObject* Function, void* Params));
}
