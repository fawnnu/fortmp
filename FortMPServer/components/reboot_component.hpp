#pragma once

class reboot_component
{
public:
    reboot_component();

    static void start_rebooting(AFortPlayerControllerAthena* player_controller, ABuildingGameplayActorSpawnMachine* spawn_machine);
    
    static void spawn_reboot_card(AFortPlayerControllerAthena* player_controller);
    static void reboot_player(AFortPlayerControllerAthena* player_controller, ABuildingGameplayActorSpawnMachine* spawn_machine);

    static bool should_spawn_reboot_card(AFortPlayerControllerAthena* player_controller);
};

// Add back to alive players list
inline void (*AddToAlivePlayerList)(UObject* GameMode, UObject* PlayerController);