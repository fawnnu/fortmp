#pragma once

class modules_component
{
public:
    modules_component();

    inline static std::vector<FString> disabled_modules;
};

namespace modules_component_hooks
{
    DEFINE_FUNCTION_HOOK(void*, LoadModuleWithFailureReason, (void* ModuleManager, FName ModuleName, int& OutError));
}
