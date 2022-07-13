#pragma once

class storm_component
{
public:
    storm_component();
};

namespace storm_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, SetSafezoneValues, (AFortGameModeAthena* FortGameModeAthena, int unk));
}
