#pragma once

class loader_component
{
public:
    loader_component();

    static void start_match(UFortPlaylistAthena* in_playlist = nullptr);
};

namespace loader_component_hooks
{
    // Called when a streaming level is loaded
    DEFINE_FUNCTION_HOOK(bool, ReadyToStartMatch, (AFortGameModeAthena* GameMode));
    DEFINE_FUNCTION_HOOK(void, UpdateLevelStreaming, (UWorld* World));
}