#pragma once

class pe_component
{
public:
    pe_component();

    inline static bool bPeHookValidated;
    inline static bool bCreated;
};

namespace pe_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, ProcessEvent, (UObject* Object, UObject* Function, void* Params));
}
