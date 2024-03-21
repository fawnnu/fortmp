#include "include.hpp"
#include "reboot_component.hpp"

#include "function_component.hpp"
#include "player_component.hpp"

reboot_component::reboot_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    INIT_POINTER(AddToAlivePlayerList, memory_manager::find_function_by_string(L"FortGameModeAthena: Player [%s] PostLog", { 0xE8, 0x100, 0x100, 0x100, 0xFF}, 1, false));
    
    if (const uintptr_t TargetInstruction = memory_manager::find_pattern("48 8D 05 ? ? ? ? 33 F6 48 89 44 24 ? 49 8B CE 49 8B 06 89 74 24 60 FF 90 ? ? ? ? 4C 8B A4 24 ? ? ? ? 48 8B 88 ? ? ? ? 48 85 C9 74 09 "))
    {
        memory_manager::lea_hook(TargetInstruction, reinterpret_cast<uintptr_t>(reboot_component_hooks::SpawnPawnsOnReboot_Hook));
    }

    REGISTER_CUSTOM_UFUNCTION(RebootPlayerWithDelay, AFortPlayerControllerAthena::StaticClass())

    DetourTransactionCommit();
}

void reboot_component_hooks::SpawnPawnsOnReboot_Hook(ABuildingGameplayActorSpawnMachine* SpawnMachine)
{
	auto InteractController = SpawnMachine->InstigatorPC().Get();
	if (!InteractController) return;

	auto InteractPawn = InteractController->Pawn();
	if (!InteractPawn) return;

	auto FortPlayerStart = AFortGameMode::GetClosestPlayerStart(InteractPawn->K2_GetActorLocation());
	if (!FortPlayerStart) return;

    // List of players to reboot
    std::vector<AFortPlayerControllerAthena*> ControllersToReboot;
    
    for (UNetConnection* NetConnection : (*GWorld)->NetDriver()->ClientConnections())
    {
        auto PlayerState = NetConnection->PlayerController()->PlayerState();
        if (!PlayerState) continue;

        auto Controller = PlayerState->Owner()->Cast<AFortPlayerControllerAthena>();
        if (!Controller) continue;

        if (PlayerState->SquadId() == SpawnMachine->SquadId())
        {
            if (PlayerState->bResurrectingNow() && !PlayerState->PawnPrivate())
            {
                Controller->EraPlayerInfo().CurrentSpawnMachine = SpawnMachine;
                ControllersToReboot.push_back(Controller);
            }
        }
    }

    // reboot the first player
    if (ControllersToReboot.size() > 0)
    {
        ControllersToReboot[0]->RebootPlayerWithDelay();
    }

    // Reboot the other players each 2 seconds
    for (int i = 1; i < ControllersToReboot.size(); i++)
    {
        UKismetSystemLibrary::K2_SetTimer(ControllersToReboot[i], L"RebootPlayerWithDelay", 2.0 * i, false);

        if (i + 1 == ControllersToReboot.size())
        {
            auto BP_OnResurrectionCompleted = FindObject<UFunction>(L"/Game/Athena/Items/EnvironmentalItems/SCMachine/BGA_Athena_SCMachine.BGA_Athena_SCMachine_C.OnResurrectionCompleted");
            ProcessEvent(SpawnMachine,BP_OnResurrectionCompleted, nullptr);
        }
    }
}

void reboot_component::start_rebooting(AFortPlayerControllerAthena* player_controller,
                                       ABuildingGameplayActorSpawnMachine* spawn_machine)
{
    // Wait 2s before rebooting
    Sleep(2000);

    // List of players to resurrect
    TArrayManager controller_player_ids = TArrayManager<FUniqueNetIdRepl>(&player_controller->ResurrectionComponent()->PlayerIdsForResurrection(), sizeof(FUniqueNetIdRepl));
    TArrayManager spawn_machine_player_ids = TArrayManager<FUniqueNetIdRepl>(&spawn_machine->PlayerIdsForResurrection(), sizeof(FUniqueNetIdRepl));

    for (int controller_player_idx = 0; controller_player_idx < controller_player_ids.Num(); controller_player_idx++)
    {
        FUniqueNetIdRepl controller_player_id = controller_player_ids[controller_player_idx];
        
        // Add player to resurrection list
        spawn_machine_player_ids.Add(&controller_player_id);
        
        for (UNetConnection*& client_connection : (*GWorld)->NetDriver()->ClientConnections())
        {
            if (UFortKismetLibrary::EqualEqual_UniqueNetIdReplUniqueNetIdRepl(client_connection->PlayerID(), controller_player_id))
            {
                game_thread_queue.push_back([ player_controller, spawn_machine ]
                {
                    // Reboot player
                    reboot_player(player_controller, spawn_machine);
                });
            }

            // Delay each spawn by 2.5 seconds
            Sleep(2000 * (controller_player_idx + 1));
        }

        // TODO: Support FFields
        controller_player_ids.Remove(controller_player_idx, FindObject(L"/Script/FortniteGame.AthenaResurrectionComponent.PlayerIdsForResurrection"));
    }

    // Wait 2.5s before changing state
    Sleep(2500);

    game_thread_queue.push_back([spawn_machine]
    {
        static uint8_t on_cooldown_enum = FindObject<UEnum>(L"/Script/FortniteGame.ESpawnMachineState")->GetEnumIndex(L"OnCooldown");
        
        // Set spawn machine into recharging state
        spawn_machine->SetSpawnMachineState(on_cooldown_enum);
    });
}

void reboot_component::spawn_reboot_card(AFortPlayerControllerAthena* player_controller)
{
    if (player_controller == nullptr)
    {
        return;
    }

    if (player_controller->Pawn() == nullptr)
    {
        // No pawn to spawn reboot card for
        return;
    }

    if (should_spawn_reboot_card(player_controller))
    {
        FVector reboot_card_location = player_controller->Pawn()->K2_GetActorLocation();
        FRotator reboot_card_rotation = player_controller->Pawn()->K2_GetActorRotation();

        // Actor class for reboot card
        static UObject* reboot_card_class = FindObject(L"/Game/Athena/Items/EnvironmentalItems/SCMachine/BGA_Athena_SCMachine_Pickup.BGA_Athena_SCMachine_Pickup_C", true);

        if (reboot_card_class)
        {
            // No actor class to spawn
            return;
        }
        
        // Spawn reboot card
        ABuildingGameplayActorSpawnChip* reboot_card_actor = reinterpret_cast<ABuildingGameplayActorSpawnChip*>(SpawnActor(*GWorld, reboot_card_class, &reboot_card_location, &reboot_card_rotation, FActorSpawnParameters {}));

        if (reboot_card_actor == nullptr)
        {
            // Reboot card failed to spawn
            return;
        }

        // Update reboot card actor
        reboot_card_actor->OwnerPlayerController() = player_controller;
        reboot_card_actor->OwnerPlayerState() = player_controller->PlayerState();
        reboot_card_actor->OwnerTeam() = player_controller->PlayerState()->TeamIndex();
        reboot_card_actor->OwnerPlayerId() = player_controller->NetConnection()->PlayerID();

        // Update reboot card replicated state
        reboot_card_actor->RebootCardReplicatedState().PlayerState() = player_controller->PlayerState();
        reboot_card_actor->RebootCardReplicatedState().ChipExpirationServerStartTime() = 300.0f;

        // TODO: Might need to OnRep to load on server
    }
}

void reboot_component::reboot_player(AFortPlayerControllerAthena* player_controller, ABuildingGameplayActorSpawnMachine* spawn_machine)
{
    if (player_controller == nullptr || spawn_machine == nullptr)
    {
        return;
    }

    AFortPlayerPawn* new_player = player_component::create_player(player_controller, spawn_machine->ResurrectLocation()->GetTransform());

    if (new_player == nullptr)
    {
        // Failed to spawn player
        return;
    }

    // Add player back to alive player list
    AddToAlivePlayerList((*GWorld)->AuthorityGameMode(), player_controller);

    // Change to 'Playing' state
    player_controller->ClientGotoState(L"Playing");
}

bool reboot_component::should_spawn_reboot_card(AFortPlayerControllerAthena* player_controller)
{
    // TODO: Do not spawn a reboot card if bIsDisconnecting
    
    int alive_team_players = 0;
    
    for (AFortPlayerController* alive_player : (*GWorld)->AuthorityGameMode()->AlivePlayers())
    {
        if (alive_player->PlayerState()->TeamIndex() == player_controller->PlayerState()->TeamIndex())
        {
            // Count alive player as part of same team
            alive_team_players++;
        }
    }

    return alive_team_players > 1;
}

void reboot_component::RebootPlayerWithDelay(AFortPlayerControllerAthena* Controller, FFrame& StackFrame)
{
    if (Controller->EraPlayerInfo().CurrentSpawnMachine &&
        Controller->PlayerState())
    {
        const auto SpawnMachine = Controller->EraPlayerInfo().CurrentSpawnMachine;
        
        UKismetSystemLibrary::SetBoolPropertyByName(Controller, L"bPlayerIsWaiting", true); // TODO: Check if this bool is needed
        Controller->ClientClearDeathNotification();
        bool bRebootPlayersInAir = SpawnMachine->RebootPlayersInAir(); // ?
        Controller->RespawnPlayerAfterDeath(bRebootPlayersInAir);

        auto RespawnedPawn = Controller->Pawn()->Cast<AFortPlayerPawnAthena>();
        if (!RespawnedPawn) return;

        Controller->PlayerState()->RebootCounter()++;
        Controller->PlayerState()->OnRep_RebootCounter();

        AddToAlivePlayerList((*GWorld)->AuthorityGameMode(), Controller);

        auto BP_OnPlayerPawnResurrected = FindObject<UFunction>(L"/Game/Athena/Items/EnvironmentalItems/SCMachine/BGA_Athena_SCMachine.BGA_Athena_SCMachine_C.OnPlayerPawnResurrected");
                
        ProcessEvent(SpawnMachine,BP_OnPlayerPawnResurrected, &RespawnedPawn);
    }
}
