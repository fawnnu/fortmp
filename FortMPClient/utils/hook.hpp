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
        spdlog::error(XOR("Failed to setup hook '{}', invalid function pointer provided"),  \
            XOR(#FunctionName));                                                            \
    }

#define DEFINE_FUNCTION_HOOK(ReturnType, FunctionName, FunctionDefinition)  \
inline ReturnType (* FunctionName##) FunctionDefinition;                \
\
ReturnType FunctionName##_Hook FunctionDefinition;

class VectoredExceptionHook
{
public:
    inline static unsigned __int64 oldfunc;
    inline static unsigned __int64 hookfunc;
    inline static DWORD Protection;
    inline static PVOID Handle;

    static void StartHook(unsigned __int64 oldfunction, unsigned __int64 hookfunction);

    static void DisableHook();
    
    static long __stdcall Handler(EXCEPTION_POINTERS* Info);
};