#pragma once

#include <detours.h>

class hook
{
    hook(std::string hook_name, void** function_pointer, void* function_detour);
    
private:
    void** function_pointer;
    void* function_detour;
};

class hook_manager
{
public:
    hook_manager() = default;
    
private:
    std::vector<hook*> hooks;
};