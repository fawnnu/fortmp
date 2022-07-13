#include "include.hpp"
#include "storm_component.hpp"

storm_component::storm_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (GetEngineVersionEasy() > 425)
    {
        // Only hook on builds where storm does not work
        REGISTER_FUNCTION_DETOUR(storm_component_hooks::SetSafezoneValues, memory_manager::find_function_by_string(L"FortGameModeAthena: No MegaStorm on SafeZone[%d]."), storm_component_hooks::SetSafezoneValues_Hook);
    }
    
    DetourTransactionCommit();
}

TMap<FName, Struct*> storm_component::get_game_data_table()
{
    // Get current playlist
    static UFortPlaylistAthena* current_playlist = (*GWorld)->GameState()->HasProperty("CurrentPlaylistData") ?
        (*GWorld)->GameState()->CurrentPlaylistData() : (*GWorld)->GameState()->CurrentPlaylistInfo().BasePlaylist();
    
    // Get game data table
    static TMap<FName, Struct*> game_data_table = *reinterpret_cast<TMap<FName, Struct*>*>(__int64(UKismetSystemLibrary::Conv_SoftObjectReferenceToObject(current_playlist->GameData())) + 0x30);
    
    return game_data_table;
}

void storm_component_hooks::SetSafezoneValues_Hook(AFortGameModeAthena* GameMode, int unk)
{
    // Call original function first
    SetSafezoneValues(GameMode, unk);
    
    // Get game data table
    static TMap<FName, Struct*> game_data_table = storm_component::get_game_data_table();

    {
        int safe_zone_phase = (*GWorld)->AuthorityGameMode()->SafeZonePhase();

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


