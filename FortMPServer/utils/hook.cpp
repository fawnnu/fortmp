#include "include.hpp"
#include "hook.hpp"

hook::hook(void**& function_pointer, void* function_detour)
{
    this->function_pointer = function_pointer;
    this->function_detour = function_detour;
}

hook::hook(void**& function_pointer, UObject* function, void* function_detour)
{
    // Update function pointer
    function_pointer = reinterpret_cast<void**>(__int64(function) + 0xB0);

    this->function_pointer = function_pointer;
    this->function_detour = function_detour;
}

bool hook::enable_hook()
{
    return DetourAttach(this->function_pointer, this->function_detour) == NO_ERROR;
}

bool hook::disable_hook()
{
    return DetourDetach(this->function_pointer, this->function_detour) == NO_ERROR;
}

void hook_manager::register_hook(hook* hook_to_add)
{
    hooks.push_back(hook_to_add);
}

void hook_manager::enable_all_hooks()
{
    for (hook* hook : hooks)
    {
        if (hook->enable_hook())
        {
            spdlog::info("Enabled hook at {:p}",
                *hook->function_pointer);
        }
        else
        {
            // Usually caused by bad pointer or invalid arguments
            spdlog::error("Failed to enable hook at {:p}",
                *hook->function_pointer);
        }
    }
}

