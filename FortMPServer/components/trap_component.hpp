#pragma once

class trap_component
{
public:
    trap_component();
};

namespace trap_component_hooks
{
    // Called when a player tries to place a trap
    DEFINE_FUNCTION_HOOK(void, ServerSpawnDeco, (AFortDecoTool* FortDecoTool, FFrame& StackFrame));
}