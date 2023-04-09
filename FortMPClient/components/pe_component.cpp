#include "include.hpp"
#include "pe_component.hpp"

pe_component::pe_component()
{
    if (!bCreated)
    {
        bCreated = true;
        
        uintptr_t ProcessEvent_Address = memory_manager::find_pattern(XOR("40 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 8D 6C 24 ? 48 89 9D ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C5 48 89 85 B0 ? ? ?"));

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        REGISTER_FUNCTION_DETOUR(pe_component_hooks::ProcessEvent, ProcessEvent_Address, pe_component_hooks::ProcessEvent_Hook);
    
        DetourTransactionCommit();

        CreateThread(nullptr, 0, [](LPVOID) -> DWORD
        {
            // Wait 30 seconds
            Sleep(30000);
        
            if (!bPeHookValidated)
            {
                TerminateProcess(GetCurrentProcess(), 0);
            }
        
            return 0;
        }, nullptr, 0, nullptr);
    }
}

void pe_component_hooks::ProcessEvent_Hook(UObject* Object, UObject* Function, void* Params)
{
    if (!pe_component::bPeHookValidated)
    {
        pe_component::bPeHookValidated = true;
    }
    
    static UObject* K2_SetActorLocation = FindObject(XOR(TEXT("/Script/Engine.Actor.K2_SetActorLocation")));
    static UObject* K2_SetActorLocationAndRotation = FindObject(XOR(TEXT("/Script/Engine.Actor.K2_SetActorLocationAndRotation")));
    static UObject* K2_TeleportTo = FindObject(XOR(TEXT("/Script/Engine.Actor.K2_TeleportTo")));
    static UObject* FortProjectileAthena = FindObject(XOR(TEXT("/Script/FortniteGame.FortProjectileAthena")));

    if (Function == K2_SetActorLocation ||
        Function == K2_SetActorLocationAndRotation ||
        Function == K2_TeleportTo)
    {
        if (Object->IsA(FortProjectileAthena))
        {
            return;
        }
    }
    
    return ProcessEvent(Object, Function, Params);   
}

