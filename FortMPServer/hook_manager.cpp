#include "include.hpp"
#include "hook_manager.hpp"

hook::hook(std::string hook_name, void** function_pointer, void* function_detour) :
    function_pointer(function_pointer),
    function_detour(function_detour)
{
    if (DetourAttach(function_pointer, function_detour) == 0)
    {
        spdlog::info("Hooked: '{}'",
            hook_name);
    }
    else
    {
        // Failed to hook, most likely due to invalid pointer or bad arguments
        spdlog::error("Failed to hook: '{}'",
            hook_name);
    }
}