#pragma once

class loader_component
{
public:
    loader_component();

    static void start_match(UFortPlaylistAthena* in_playlist = nullptr);

    // List of all loaded levels
    inline static std::vector<FName> loaded_levels;

    // Amount of loaded levels required to consider the server loaded
    inline static constexpr int loaded_levels_threshold = 400;
};

namespace loader_component_hooks
{
    // Called when a streaming level is loaded
    DEFINE_FUNCTION_HOOK(void, SetLoadedLevel, (ULevelStreaming* LevelStreaming, UObject* Level));
}