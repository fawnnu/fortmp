#pragma once

#include <detours.h>

class hook
{
public:
    hook(std::string hook_name, void** function_pointer, void* function_detour);
    
    std::string function_name;
    void** function_pointer;
    void* function_detour;
};

class hook_manager
{
public:
    hook_manager();

    //add a hook to the hooks vector
    void add_hook(hook* new_hook);
    
    std::vector<hook*> hooks;
};

inline hook_manager* global_hook_manager; 