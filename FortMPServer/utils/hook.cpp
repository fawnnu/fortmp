#include "include.hpp"
#include "hook.hpp"

#include <fstream>

// Process event hooks are by default only working for actors.
// In case a process event hook is needed for some object, you'll have to VTable hook PE for that actor's class only.

void VectoredExceptionHook::StartHook(unsigned __int64 oldfunction, unsigned __int64 hookfunction)
{
    oldfunc = oldfunction;
    hookfunc = hookfunction;

    Handle = AddVectoredExceptionHandler(true, (PVECTORED_EXCEPTION_HANDLER)Handler);

    VirtualProtect((LPVOID)oldfunction, 1, PAGE_EXECUTE_READ | PAGE_GUARD, &Protection);
}

void VectoredExceptionHook::DisableHook()
{
    DWORD old;

    VirtualProtect((LPVOID)oldfunc, 1, Protection, &old);
    RemoveVectoredExceptionHandler(Handle);
}
    
long __stdcall VectoredExceptionHook::Handler(EXCEPTION_POINTERS* Info)
{
    if (Info->ExceptionRecord->ExceptionCode == STATUS_GUARD_PAGE_VIOLATION)
    {
        if (Info->ContextRecord->Rip == (unsigned __int64)oldfunc) Info->ContextRecord->Rip = (unsigned __int64)hookfunc;
        Info->ContextRecord->EFlags |= 0x100;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    if (Info->ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP)
    {
        DWORD dwOld;
        VirtualProtect((LPVOID)oldfunc, 1, PAGE_EXECUTE_READ | PAGE_GUARD, &dwOld);
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

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
    REGISTER_FUNCTION_DETOUR(hook_manager_hooks::ProcessInternal, memory_manager::find_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 30 48 8B 6A 10 49 8B F8 4C 8B 42 28 48 8B DA 48 8B 01"), hook_manager_hooks::ProcessInternal_Hook);
    REGISTER_FUNCTION_DETOUR(hook_manager_hooks::ProcessLocalScriptFunction, memory_manager::find_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 70 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 48 8B 42 20 48 8D 35 ? ? ? ? 49 8B F8 48 8B DA 80 38 04 74 34"), hook_manager_hooks::ProcessLocalScriptFunction_Hook);
    //REGISTER_FUNCTION_DETOUR(hook_manager_hooks::ProcessScriptFunction, memory_manager::find_pattern("40 55 53 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 8D 6C 24 ? 48 8B 05 ? ? ? ? 48 33 C5 48 89 85 ? ? ? ? 48 8B 85 ? ? ? ? 4C 8B E1"), hook_manager_hooks::ProcessScriptFunction_Hook);

    DetourTransactionCommit();

    // Creative saving etc...
    if (static UObject* Default__EnvQueryInstanceBlueprintWrapper = FindObject(L"/Script/AIModule.Default__EnvQueryInstanceBlueprintWrapper"))
    {
        REGISTER_VTABLE_HOOK(Default__EnvQueryInstanceBlueprintWrapper->VTable, ProcessEvent_Index, hook_manager_hooks::UEnvQueryInstanceBlueprintWrapper_ProcessEvent);
    }
    if (static UFortLevelSaveComponent* Default__FortLevelSaveComponent = CreateDefaultObject<UFortLevelSaveComponent>())
    {
        REGISTER_VTABLE_HOOK(Default__FortLevelSaveComponent->VTable, ProcessEvent_Index, hook_manager_hooks::UFortLevelSaveComponent_ProcessEvent);
    }
    if (static UPlaysetLevelStreamComponent* Default__PlaysetLevelStreamComponent = CreateDefaultObject<UPlaysetLevelStreamComponent>())
    {
        REGISTER_VTABLE_HOOK(Default__PlaysetLevelStreamComponent->VTable, ProcessEvent_Index, hook_manager_hooks::UPlaysetLevelStreamComponent_ProcessEvent);
    }
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


void hook_manager_hooks::UFortLevelSaveComponent_ProcessEvent_Hook(UObject* Object, UObject* Function, void* Params)
{
    if (Object && Function)
    {
        if (hook_manager::hooked_functions[Function])
        {
            // Call hooked function
            hook_manager::hooked_functions[Function](Object, Function, Params);
        }
    
        UFortLevelSaveComponent_ProcessEvent(Object, Function, Params);

        if (hook_manager::post_hooked_functions[Function])
        {
            // Call hooked function
            hook_manager::post_hooked_functions[Function](Object, Function, Params);
        }
    }
}

void hook_manager_hooks::UPlaysetLevelStreamComponent_ProcessEvent_Hook(UObject* Object, UObject* Function, void* Params)
{
    if (hook_manager::hooked_functions[Function])
    {
        hook_manager::hooked_functions[Function](Object, Function, Params);
    }

    UPlaysetLevelStreamComponent_ProcessEvent(Object, Function, Params);

    if (hook_manager::post_hooked_functions[Function])
    {
        // Call hooked function
        hook_manager::post_hooked_functions[Function](Object, Function, Params);
    }
}

void hook_manager_hooks::ProcessInternal_Hook(UObject* Context, FFrame& StackFrame, void* ReturnValue)
{
    if (hook_manager::process_internal_hooks.contains(StackFrame.Node))
    {
        hook_manager::process_internal_hooks[StackFrame.Node].first(Context, StackFrame, ReturnValue);

        if (hook_manager::process_internal_hooks[StackFrame.Node].second)
        {
            return;
        }
    }

    ProcessInternal(Context, StackFrame, ReturnValue);   
}

void hook_manager_hooks::ProcessLocalScriptFunction_Hook(UObject* Context, FFrame& StackFrame, void* ReturnValue)
{
    if (hook_manager::process_internal_hooks.contains(StackFrame.Node))
    {
        hook_manager::process_internal_hooks[StackFrame.Node].first(Context, StackFrame, ReturnValue);

        if (hook_manager::process_internal_hooks[StackFrame.Node].second)
        {
            return;
        }
    }

    ProcessLocalScriptFunction(Context, StackFrame, ReturnValue);   
}

void hook_manager_hooks::ProcessScriptFunction_Hook(UObject* Context, UFunction* Function, FFrame& Stack, void* RESULT_DECL, void* Exec)
{
    if (hook_manager::process_internal_hooks.contains(Function))
    {
        spdlog::info(L"Using {} for {}", Exec, Function->GetName().c_str());
    }

    ProcessScriptFunction(Context, Function, Stack, RESULT_DECL, Exec);
}
