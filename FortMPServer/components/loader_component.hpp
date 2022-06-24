#pragma once

class loader_component
{
public:
    loader_component();

    static void start_match(UObject* in_playlist = nullptr);
    static void load_level(FName package_name);

    inline static std::vector<FName> loaded_levels;
};

namespace loader_component_hooks
{
    inline void (*ReadyToStartMatch)(AGameMode* GameMode, FFrame& StackFrame);

    // Called when GameMode is ready to start match
    void ReadyToStartMatch_Hook(AGameMode* GameMode, FFrame& StackFrame);;
}