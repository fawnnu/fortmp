#include "include.hpp"
#include "hook.hpp"

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