#include "include.hpp"
#include "storm_component.hpp"

#include "era_mutators_component.hpp"
#include "log_component.hpp"
#include "player_component.hpp"

storm_component::storm_component()
{
    storm_table_overrides = {
        {
            1, {
                {{0, 20},  {120, 140}},
                {{20, 30}, {130, 145}},
                {{30, 40}, {140, 150}},
                {{40, 50}, {150, 160}},
                {{50, 60}, {160, 165}},
                {{60, 70}, {170, 170}},
                {{70, 80}, {180, 180}},
                {{80, 90}, {190, 190}}
            }
        },
        {
            2, {
                {{0, 20}, {90, 100}},
                {{20, 30}, {100, 100}},
                {{30, 40}, {100, 105}},
                {{40, 50}, {100, 105}},
                {{50, 60}, {110, 110}},
                {{60, 70}, {110, 110}},
                {{70, 80}, {110, 110}},
                {{80, 90}, {120, 120}}
            }
        },
        {
            3, {
                {{0, 20}, {90, 60}},
                {{20, 30}, {100, 60}},
                {{30, 40}, {100, 70}},
                {{40, 50}, {100, 70}},
                {{50, 60}, {110, 80}},
                {{60, 70}, {110, 80}},
                {{70, 80}, {110, 80}},
                {{80, 90}, {120, 90}}
            }
        },
        {
            4, {
                {{0, 20}, {60, 40}},
                {{20, 30}, {70, 40}},
                {{30, 40}, {70, 45}},
                {{40, 50}, {70, 45}},
                {{50, 60}, {80, 50}},
                {{60, 70}, {80, 55}},
                {{70, 80}, {80, 55}},
                {{80, 90}, {80, 60}}
            }
        },
        {
            5, {
                {{0, 20}, {50, 25}},
                {{20, 30}, {60, 25}},
                {{30, 40}, {60, 30}},
                {{40, 50}, {70, 30}},
                {{50, 60}, {70, 30}},
                {{60, 70}, {70, 40}},
                {{70, 80}, {80, 40}},
                {{80, 90}, {80, 40}}
            }
        },
        {
            6, {
                {{0, 20}, {30, 15}},
                {{20, 30}, {30, 15}},
                {{30, 40}, {40, 20}},
                {{40, 50}, {40, 20}},
                {{50, 60}, {40, 20}},
                {{60, 70}, {50, 25}},
                {{70, 80}, {50, 25}},
                {{80, 90}, {50, 25}}
            }
        },
        {
            7, {
                {{0, 20}, {20, 10}},
                {{20, 30}, {20, 15}},
                {{30, 40}, {20, 15}},
                {{40, 50}, {30, 15}},
                {{50, 60}, {30, 20}},
                {{60, 70}, {30, 20}},
                {{70, 80}, {40, 20}},
                {{80, 90}, {40, 25}}
            }
        },
        {
            8, {
                {{0, 20}, {15, 10}},
                {{20, 30}, {15, 10}},
                {{30, 40}, {15, 15}},
                {{40, 50}, {15, 15}},
                {{50, 60}, {20, 20}},
                {{60, 70}, {20, 20}},
                {{70, 80}, {25, 20}},
                {{80, 90}, {25, 25}}
            }
        },
        {
            9, {
                {{0, 20}, {10, 10}},
                {{20, 30}, {10, 10}},
                {{30, 40}, {15, 15}},
                {{40, 50}, {15, 15}},
                {{50, 60}, {20, 20}},
                {{60, 70}, {20, 20}},
                {{70, 80}, {20, 20}},
                {{80, 90}, {20, 20}}
            }
        }
    };
    
    INIT_POINTER(StartSafeZone, memory_manager::find_function_by_string(L"AFortGameModeAthena::IsSafeZoneAllowed: Skipping SafeZone phase: bSkipSafeZones == true"));
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    // Only hook on builds where storm does not work
    REGISTER_FUNCTION_DETOUR(storm_component_hooks::SetSafezoneValues, memory_manager::find_function_by_string(L"FortGameModeAthena: No MegaStorm on SafeZone[%d]."), storm_component_hooks::SetSafezoneValues_Hook);
    if (!storm_component_hooks::SetSafezoneValues)
    {
        REGISTER_FUNCTION_DETOUR(storm_component_hooks::SetSafezoneValues, memory_manager::find_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 70 48 8B B9 ? ? ? ? 33 DB 0F 29 74 24 ? 48 8B F1 48 85 FF"), storm_component_hooks::SetSafezoneValues_Hook);
    }
    
    DetourTransactionCommit();
}

TMap<FName, Struct*> storm_component::get_game_data_table()
{
    // Get current playlist
    STATIC_HASPROPERTY((*GWorld)->GameState(), CurrentPlaylistData);
    static UFortPlaylistAthena* current_playlist = bExists_CurrentPlaylistData ?
        (*GWorld)->GameState()->CurrentPlaylistData() : (*GWorld)->GameState()->CurrentPlaylistInfo().BasePlaylist();
    
    // Get game data table
    static TMap<FName, Struct*> game_data_table = *reinterpret_cast<TMap<FName, Struct*>*>(__int64(current_playlist->GameData().Get()) + 0x30);
    
    return game_data_table;
}

std::pair<float, float> storm_component::get_storm_table_override(int phase, int amount_of_players, bool* success)
{
    if (storm_table_overrides.contains(phase))
    {
        for (auto overrides : storm_table_overrides[phase])
        {
            if (amount_of_players > overrides.first.first && amount_of_players <= overrides.first.second)
            {
                if (success)
                {
                    *success = true;
                }
            
                return overrides.second;
            }
        }
    }

    if (success)
    {
        *success = false;
    }
    return {};
}

void storm_component_hooks::SetSafezoneValues_Hook(AFortGameModeAthena* GameMode, int unk)
{
    // Call original function first
    SetSafezoneValues(GameMode, unk);

    if (GameMode->OnUpdateStorm() == true)
    {
        return;
    }

    if (era_mutators_component::IsActive("competitive"))
    {
        int safe_zone_phase = (*GWorld)->AuthorityGameMode()->SafeZonePhase();

        if (safe_zone_phase >= 4)
        {
            for (int i = 0; i < (*GWorld)->NetDriver()->ClientConnections().Num(); i++)
            {
                UNetConnection* ClientConnection = (*GWorld)->NetDriver()->ClientConnections()[i];

                if (!memory_manager::is_bad_read_pointer(ClientConnection) && ClientConnection)
                {
                    if (AFortPlayerController* PlayerController = ClientConnection->PlayerController(); !memory_manager::is_bad_read_pointer(PlayerController) && PlayerController)
                    {
                        if (AFortPlayerPawn* PlayerPawn = PlayerController->Pawn(); !memory_manager::is_bad_read_pointer(PlayerPawn) && PlayerPawn)
                        {
                            UKismetSystemLibrary::SetBoolPropertyByName(PlayerPawn, L"bIsInvulnerable", false);
                        }
                    }
                }
            }
        }
    }

    // Apply overrides
    if (era_mutators_component::IsActive("enable_storm_overrides"))
    {
        int safe_zone_phase = (*GWorld)->AuthorityGameMode()->SafeZonePhase();
        bool found_override = false;
        std::pair<float,float> override = storm_component::get_storm_table_override(safe_zone_phase, player_component::starting_players, &found_override);
        if (found_override)
        {
            GameMode->SafeZoneIndicator()->SafeZoneStartShrinkTime() = (*GWorld)->GameState()->ReplicatedWorldTimeSeconds() + override.first;
            GameMode->SafeZoneIndicator()->SafeZoneFinishShrinkTime() = GameMode->SafeZoneIndicator()->SafeZoneStartShrinkTime() + override.second;

            if (log_component::enable_debug_logs)
            {
                spdlog::info("storm_component_hooks::SetSafezoneValues_Hook: found overrides for phase {}, overriding with [WaitTime: {}, ShrinkTime: {}]",
                    safe_zone_phase,
                    GameMode->SafeZoneIndicator()->SafeZoneStartShrinkTime(),
                    GameMode->SafeZoneIndicator()->SafeZoneFinishShrinkTime());
            }
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::info("storm_component_hooks::SetSafezoneValues_Hook: keeping default safezone values for phase {}", safe_zone_phase);
            }
        }
    }
    
    if (GetEngineVersionEasy() > 425)
    {
        // Get game data table
        static TMap<FName, Struct*> game_data_table = storm_component::get_game_data_table();

        int safe_zone_phase = (*GWorld)->AuthorityGameMode()->SafeZonePhase();
        
        {
            // Amount of time to wait until zone starts
            float phase_wait_time = FSimpleCurve::GetClass() ?
                TArrayManager<FSimpleCurveKey>(&static_cast<FSimpleCurve*>(game_data_table[L"Default.SafeZone.WaitTime"].Value)->Keys(), FSimpleCurveKey::GetStructSize())[safe_zone_phase].Value() :
                TArrayManager<FRichCurveKey>(&static_cast<FRichCurve*>(game_data_table[L"Default.SafeZone.WaitTime"].Value)->Keys(), FRichCurveKey::GetStructSize())[safe_zone_phase].Value();

            // Amount of time that zone will take to shrink
            float phase_shrink_time = FSimpleCurve::GetClass() ?
                TArrayManager<FSimpleCurveKey>(&static_cast<FSimpleCurve*>(game_data_table[L"Default.SafeZone.ShrinkTime"].Value)->Keys(), FSimpleCurveKey::GetStructSize())[safe_zone_phase].Value() :
                TArrayManager<FRichCurveKey>(&static_cast<FRichCurve*>(game_data_table[L"Default.SafeZone.ShrinkTime"].Value)->Keys(), FRichCurveKey::GetStructSize())[safe_zone_phase].Value();
        
            // Update storm time
            GameMode->SafeZoneIndicator()->SafeZoneStartShrinkTime() = (*GWorld)->GameState()->ReplicatedWorldTimeSeconds() + phase_wait_time;
            GameMode->SafeZoneIndicator()->SafeZoneFinishShrinkTime() = GameMode->SafeZoneIndicator()->SafeZoneStartShrinkTime() + phase_shrink_time;
        }
    }
}


