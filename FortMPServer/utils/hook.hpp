#pragma once

#include <deque>

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
    { void* native_to_hook = FindObject<UFunction>(FunctionObject) ?                                                                                                                                                \
        FindObject<UFunction>(FunctionObject)->get_native_function() : 0;        \
                                                                                                                                                                                                                    \
    REGISTER_FUNCTION_DETOUR(FunctionName, native_to_hook, FunctionDetour); }

#define REGISTER_PROCESS_EVENT_DETOUR(FunctionObject, FunctionDetour)                                                           \
    { UObject* function_to_hook = FindObject<UObject>(FunctionObject) ?                                                         \
        FindObject<UObject>(FunctionObject) : FindObject<UObject>(FunctionObject, false, true);                                 \
                                                                                                                                \
    hook_manager::hooked_functions[function_to_hook] = std::function<void(UObject*, UObject*, void*)>(&FunctionDetour); }

#define DEFINE_FUNCTION_HOOK(ReturnType, FunctionName, FunctionDefinition)  \
    inline ReturnType (* FunctionName##) FunctionDefinition;                \
                                                                            \
    ReturnType FunctionName##_Hook FunctionDefinition;

class hook_manager
{
public:
    hook_manager();

    // Map of functions to hook
    inline static std::map<UObject*, std::function<void(UObject*, UObject*, void*)>> hooked_functions;
};

// Queue of code to execute on game thread
inline std::deque<std::function<void()>> game_thread_queue;

namespace hook_manager_hooks
{
    // Called every time that ProcessEvent is called
    DEFINE_FUNCTION_HOOK(void, ProcessEvent, (UObject* Object, UObject* Function, void* Params));
}
