#include "include.hpp"
#include "hook.hpp"

#include <fstream>

hook_manager::hook_manager()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(hook_manager_hooks::ProcessEvent, memory_manager::find_pattern("40 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 8D 6C 24 ? 48 89 9D ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C5 48 89 85 B0 ? ? ?"), hook_manager_hooks::ProcessEvent_Hook);

    DetourTransactionCommit();
}

void hook_manager_hooks::ProcessEvent_Hook(UObject* Object, UObject* Function, void* Params)
{
    while (!game_thread_queue.empty())
    {
        // Execute code on game thread
        game_thread_queue.front()();
        game_thread_queue.pop_front();
    }
    
    if (Object && Function)
    {
        if (hook_manager::hooked_functions[Function])
        {
            // Call hooked function
            hook_manager::hooked_functions[Function](Object, Function, Params);
        }
    }
    
    return ProcessEvent(Object, Function, Params);
}