#include "include.hpp"
#include "hook.hpp"

hook_manager::hook_manager()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(hook_manager_hooks::ProcessEvent, FindObject(L"/Script/CoreUObject.Default__Object")->VTable[0x40], hook_manager_hooks::ProcessEvent_Hook);

    DetourTransactionCommit();
}

void hook_manager_hooks::ProcessEvent_Hook(UObject* Object, UObject* Function, void* Params)
{
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