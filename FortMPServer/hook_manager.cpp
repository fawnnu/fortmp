#include "include.hpp"
#include "hook_manager.hpp"

hook::hook(std::string hook_name, void** function_pointer, void* function_detour) :
    function_name(hook_name),
    function_pointer(function_pointer),
    function_detour(function_detour)
{
    global_hook_manager->add_hook(this);
}

hook_manager::hook_manager()
{
    global_hook_manager = this;
}

void hook_manager::add_hook(hook* new_hook)
{
    hooks.push_back(new_hook);
}
