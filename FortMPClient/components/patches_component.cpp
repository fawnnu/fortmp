#include "include.hpp"
#include "patches_component.hpp"

patches_component::patches_component()
{
    // Null count of an array to prevent a crash
    if (auto GameData = FindObject(XOR(L"/Game/Balance/DefaultGameData.DefaultGameData")))
    {
        if (GameData->HasProperty(XOR(L"RandomItemPickers")))
        {
            static_cast<UFortGameData*>(GameData)->RandomItemPickers().Count = 0;
        }
    }
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (GetEngineVersionEasy() > 423)
    {
        REGISTER_FUNCTION_DETOUR(patches_component_hooks::SendInventoryChangedNotification, memory_manager::find_function_by_string(XOR(L"STAT_FortInventory_SendInventoryChangedNotification")), patches_component_hooks::SendInventoryChangedNotification_Hook);
    }
    
    DetourTransactionCommit();
}

void patches_component::initialize_ProjectWorldToScreen_patch()
{
    // Detour the function
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(patches_component_hooks::ProjectWorldToScreen, memory_manager::find_pattern(XOR("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 41 0F B6 E9 49 8B D8")), patches_component_hooks::ProjectWorldToScreen_Hook);
    
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

void patches_component_hooks::ProjectWorldToScreen_Hook()
{
    TerminateProcess(GetCurrentProcess(), 0);
}


