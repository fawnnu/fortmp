#pragma once

#define REGISTER_FUNCTION_DETOUR(FunctionName, FunctionPointer, FunctionDetour)        \
    INIT_POINTER(FunctionName, FunctionPointer);                                       \
                                                                                       \
    if (FunctionName)                                                                  \
    {                                                                                  \
        DetourAttach(&reinterpret_cast<PVOID&>(FunctionName), FunctionDetour);         \
    }                                                                                  \
    else                                                                               \
    {                                                                                  \
        spdlog::error("Failed to setup hook '{}', invalid function pointer provided",  \
            #FunctionName);                                                            \
    }

#define REGISTER_UFUNCTION_DETOUR(FunctionName, FunctionObject, FunctionDetour)                                                                                                                                     \
    { void* native_to_hook = FindObject<UFunction>(FunctionObject) ?                                                                                                                                                  \
        FindObject<UFunction>(FunctionObject)->get_native_function() : (FindObject<UFunction>(FunctionObject, false, true) ? FindObject<UFunction>(FunctionObject, false, true)->get_native_function() : 0);        \
                                                                                                                                                                                                                    \
    REGISTER_FUNCTION_DETOUR(FunctionName, native_to_hook, FunctionDetour); }

#define DEFINE_FUNCTION_HOOK(ReturnType, FunctionName, FunctionDefinition)  \
    inline ReturnType (* FunctionName##) FunctionDefinition;                \
                                                                            \
    ReturnType FunctionName##_Hook FunctionDefinition;

class hook
{
public:
    hook(void**& function_pointer, void* function_detour);

    bool enable_hook();
    bool disable_hook();
    
    void** function_pointer;
    void* function_detour;
};

class hook_manager
{
public:
    static void register_hook(hook* hook_to_add);
    static void enable_all_hooks();

    inline static std::vector<hook*> hooks;
};
