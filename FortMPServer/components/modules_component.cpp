#include "include.hpp"
#include "modules_component.hpp"

modules_component::modules_component()
{
    // Initialize list of disabled modules
    disabled_modules =
    {
        L"UACClient",
        L"UACBase",
        L"UACServer",
        L"UACCommon"
    };

    // Hook loadmodule
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(modules_component_hooks::LoadModuleWithFailureReason, memory_manager::find_function_by_string(L"STAT_ModuleLoad"), modules_component_hooks::LoadModuleWithFailureReason_Hook);
    
    DetourTransactionCommit();
}

void* modules_component_hooks::LoadModuleWithFailureReason_Hook(void* ModuleManager, FName ModuleName, int& OutError)
{
    FString ModuleNameStr = FString(ModuleName.ToString().c_str());

    for (FString& Module : modules_component::disabled_modules)
    {
        if (Module==ModuleNameStr)
        {
            return nullptr;
        }
    }

    return LoadModuleWithFailureReason(ModuleManager, ModuleName, OutError);
}