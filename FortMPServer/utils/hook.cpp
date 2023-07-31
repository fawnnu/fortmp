#include "include.hpp"
#include "hook.hpp"

#include <fstream>

// Process event hooks are by default only working for actors.
// In case a process event hook is needed for some object, you'll have to VTable hook PE for that actor's class only.

hook_manager::hook_manager()
{
    // GET ALL THE DATA WE NEED ABOUT PROCESSEVENT
    uintptr_t ProcessEvent_Address = memory_manager::find_pattern("40 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 8D 6C 24 ? 48 89 9D ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C5 48 89 85 B0 ? ? ?");

    int ProcessEvent_Index = 0;
    UObject* Default__Object = FindObject(L"/Script/CoreUObject.Default__Object");
    
    for (int i = 0; i < INT_MAX; i++)
    {
        if (Default__Object->VTable[i] == reinterpret_cast<LPVOID>(ProcessEvent_Address))
        {
            ProcessEvent_Index = i;
            
            break;
        }
    }
    //  
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(hook_manager_hooks::AActor_ProcessEvent, FindObject(L"/Script/Engine.Default__Actor")->VTable[ProcessEvent_Index], hook_manager_hooks::AActor_ProcessEvent_Hook);

    DetourTransactionCommit();

    REGISTER_VTABLE_HOOK(FindObject(L"/Script/AIModule.Default__EnvQueryInstanceBlueprintWrapper")->VTable, ProcessEvent_Index, hook_manager_hooks::UEnvQueryInstanceBlueprintWrapper_ProcessEvent);
}

void hook_manager_hooks::AActor_ProcessEvent_Hook(UObject* Object, UObject* Function, void* Params)
{
    if (Object && Function)
    {
        if (hook_manager::hooked_functions[Function])
        {
            // Call hooked function
            hook_manager::hooked_functions[Function](Object, Function, Params);
        }
    }

    if (Object && Function)
    {
        AActor_ProcessEvent(Object, Function, Params);

        if (hook_manager::post_hooked_functions[Function])
        {
            // Call hooked function
            hook_manager::post_hooked_functions[Function](Object, Function, Params);
        }
    }
}

void hook_manager_hooks::UEnvQueryInstanceBlueprintWrapper_ProcessEvent_Hook(UObject* Object, UObject* Function, void* Params)
{
    if (Object && Function)
    {
        if (hook_manager::hooked_functions[Function])
        {
            // Call hooked function
            hook_manager::hooked_functions[Function](Object, Function, Params);
        }
    }
    
    return UEnvQueryInstanceBlueprintWrapper_ProcessEvent(Object, Function, Params);
}
