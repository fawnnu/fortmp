#pragma once

class storm_component
{
public:
    storm_component();

    static TMap<FName, Struct*> get_game_data_table();
    static std::pair<float, float> get_storm_table_override(int phase, int amount_of_players, bool* success);

    static inline std::map<int, std::vector<std::pair<std::pair<int,int>, std::pair<float, float>>>> storm_table_overrides;
};

namespace storm_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, SetSafezoneValues, (AFortGameModeAthena* FortGameModeAthena, int unk));
}

inline char (*StartSafeZone)(AFortGameModeAthena* Self, char, char);
