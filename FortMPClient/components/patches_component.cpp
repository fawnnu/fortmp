#include "include.hpp"
#include "patches_component.hpp"

patches_component::patches_component()
{
    // Patch out encryption token
    while (uintptr_t encryption_token_ref = memory_manager::find_pattern(XOR("45 00 6E 00 63 00 72 00 79 00 70 00 74 00 69 00 6F 00 6E 00 54 00 6F 00 6B 00 65 00 6E 00")))
    {
        DWORD old_protect, new_protect;
        
        VirtualProtect(reinterpret_cast<LPVOID>(encryption_token_ref), 30, PAGE_EXECUTE_READWRITE, &old_protect);

        ZeroMemory(reinterpret_cast<LPVOID>(encryption_token_ref), 30);
        
        VirtualProtect(reinterpret_cast<LPVOID>(encryption_token_ref), 30, old_protect, &new_protect);
    }
        
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (GetEngineVersionEasy() > 423)
    {
        REGISTER_FUNCTION_DETOUR(patches_component_hooks::SendInventoryChangedNotification, memory_manager::find_function_by_string(XOR(L"STAT_FortInventory_SendInventoryChangedNotification")), patches_component_hooks::SendInventoryChangedNotification_Hook);
    }
    
    DetourTransactionCommit();
}

void patches_component_hooks::SendInventoryChangedNotification_Hook(UObject* Inventory, uintptr_t b, uintptr_t c, uintptr_t d)
{
    if (!Inventory)
    {
        return; //avoid client crash if there's an invalid inventory
    }
    
    return SendInventoryChangedNotification(Inventory, b, c, d);
}


