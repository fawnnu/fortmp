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
    { UObject* function_to_hook = FindObject<UObject>(FunctionObject);                                                          \
                                                                                                                                \
    if (function_to_hook) hook_manager::hooked_functions[function_to_hook] = std::function<void(UObject*, UObject*, void*)>(&FunctionDetour); }

#define REGISTER_POST_PROCESS_EVENT_DETOUR(FunctionObject, FunctionDetour)                                                           \
    { UObject* function_to_hook = FindObject<UObject>(FunctionObject);                                                          \
                                                                                                                                \
    if (function_to_hook) hook_manager::post_hooked_functions[function_to_hook] = std::function<void(UObject*, UObject*, void*)>(&FunctionDetour); }

#define REGISTER_CALL_DETOUR(FunctionObject, FunctionDetour) \
    { \
        UFunction* function_to_hook = FindObject<UFunction>(FunctionObject) ? \
        FindObject<UFunction>(FunctionObject) : FindObject<UFunction>(FunctionObject, false, true); \
        \
        if (function_to_hook != nullptr) \
        {\
            const auto all_calls_in_function = memory_manager::get_all_calls_in_function(reinterpret_cast<uintptr_t>(function_to_hook->get_native_function())); \
            \
            memory_manager::call_hook(all_calls_in_function[all_calls_in_function.size() - 1], reinterpret_cast<uintptr_t>(FunctionDetour));\
        }\
        else \
        {\
            spdlog::error(L"Failed to hook '{}', invalid function object provided", FunctionObject);\
        }\
    }

#define REGISTER_CALL_DETOUR_WITH_ADDRESS(CallAddress, FunctionDetour) \
    {\
        if (CallAddress != 0)\
        {\
            memory_manager::call_hook(CallAddress, reinterpret_cast<uintptr_t>(FunctionDetour));\
        }\
    }

#define REGISTER_VTABLE_HOOK(VTable, FunctionIndex, FunctionToHook)\
    {\
        if (VTable)\
        {\
            INIT_POINTER(FunctionToHook, VTable[FunctionIndex]);\
            \
            DWORD old_protect, new_protect;\
            \
            VirtualProtect(VTable, 4096, PAGE_EXECUTE_READWRITE, &old_protect);\
            \
            VTable[FunctionIndex] = FunctionToHook##_Hook;\
            \
            VirtualProtect(VTable, 4096, old_protect, &new_protect);\
        }\
    }

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
    
    // Map of functions to hook
    inline static std::map<UObject*, std::function<void(UObject*, UObject*, void*)>> post_hooked_functions;
};

// Queue of code to execute on game thread
inline std::deque<std::function<void()>> game_thread_queue;
inline std::deque<nlohmann::json> kick_events_queue;
inline std::deque<struct progression_entry*> challenges_progression_queue;
inline std::deque<reply_info*> event_replies;

namespace hook_manager_hooks
{
    // Called every time that ProcessEvent is called
    DEFINE_FUNCTION_HOOK(void, AActor_ProcessEvent, (UObject* Object, UObject* Function, void* Params));
    DEFINE_FUNCTION_HOOK(void, UEnvQueryInstanceBlueprintWrapper_ProcessEvent, (UObject* Object, UObject* Function, void* Params));
}
