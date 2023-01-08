#include "include.hpp"
#include "patches_component.hpp"

patches_component::patches_component()
{
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


