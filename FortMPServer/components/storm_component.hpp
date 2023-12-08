#pragma once

class storm_component
{
public:
    storm_component();

    static TMap<FName, Struct*> get_game_data_table();
};

namespace storm_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, SetSafezoneValues, (AFortGameModeAthena* FortGameModeAthena, int unk));
}

inline char (*StartSafeZone)(AFortGameModeAthena* Self, char, char);
