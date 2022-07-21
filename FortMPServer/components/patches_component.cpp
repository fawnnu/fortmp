#include "include.hpp"
#include "patches_component.hpp"

patches_component::patches_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    // Limit the tickrate
    REGISTER_FUNCTION_DETOUR(patches_component_hook::GetMaxTickRate, memory_manager::find_function_by_string(L"Hitching by request!"), patches_component_hook::GetMaxTickRate_Hook);
    
    REGISTER_FUNCTION_DETOUR(patches_component_hook::GetTagsFromPawnLocation, memory_manager::find_pattern("48 8B C4 57 48 81 EC ? ? ? ? 4C 8B 82 ? ? ? ? 48 8B F9 0F 29 70 E8 0F 29 78 D8 44 0F 29 40 ? F3 45 0F 10 80 ? ? ? ? F3 41 0F 10 B0 ? ? ? ? F3 41 0F 10 88 ? ? ? ? F3 41 0F 10 A0 ? ? ? ? 44 0F 29 58 ? 44 0F 29 60 ? 45 0F 57 E4"), patches_component_hook::GetTagsFromPawnLocation_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::GetTagsFromPawnLocation, memory_manager::find_function_by_string(L"STAT_PoiVolume_GetTagsForPawnLocation", { 0xE8, 0x100, 0x100, 0x100, 0x0 }, 1, false), patches_component_hook::GetTagsFromPawnLocation_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::UpdateNetStats, memory_manager::find_pattern("40 55 56 41 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 8B 01"), patches_component_hook::UpdateNetStats_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::UpdateNetStats, memory_manager::find_function_by_string(L"STAT_NetDebugWidget_UpdateNetStats"), patches_component_hook::UpdateNetStats_Hook);

    // GetNetMode and WorldGetNetMode are hooked to make Fortnite think we are a dedicated server;
    // With those hooks, double damage numbers, glider being attached to the pawn and other minor issues such as items not working will be fixed.
    REGISTER_FUNCTION_DETOUR(patches_component_hook::GetNetMode, memory_manager::find_pattern("48 83 EC 28 48 8B 01 FF 90 ? ? ? ? 84 C0 74 12 33 C0 38 05 ? ? ? ? 0F 95 C0 FF C0 48 83 C4 28"), patches_component_hook::GetNetMode_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::WorldGetNetMode, memory_manager::find_pattern("40 53 48 81 EC ? ? ? ? 48 83 79 ? ? 48 8B D9 74 0E B8 ? ? ? ? 48 81 C4 ? ? ? ? 5B"), patches_component_hook::WorldGetNetMode_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::WorldGetNetMode, memory_manager::find_function_by_string(L"not creating navigation on clients", {0xe8}, 1), patches_component_hook::WorldGetNetMode_Hook);

    if (GetEngineVersionEasy() > 421)
    {
        // Only apply patches on S7+ to prevent errors on earlier versions
        REGISTER_FUNCTION_DETOUR(patches_component_hook::TravelPatch, memory_manager::find_function_by_string(L"OnInitWorldRecordComplete: Invalid tile", {0xe8, 0x100, 0x100, 0x100, 0xff}, 1, false), patches_component_hook::TravelPatch_Hook);
        REGISTER_FUNCTION_DETOUR(patches_component_hook::LoadingScreenPatch, memory_manager::find_pattern("48 89 5C 24 ? 57 48 83 EC 30 48 8B 01 48 8B F9 FF 90 ? ? ? ? 48 8B D8 48 85 C0 0F 84 ? ? ? ? 48 83 78 ? ? 0F 84 ? ? ? ? 48 8B 17 48 8B CF"), patches_component_hook::LoadingScreenPatch_Hook);
    }

    // Patch issue caused by nomcp above S12 (UE 4.26+)
    if (GetEngineVersionEasy() > 425)
    {
        REGISTER_FUNCTION_DETOUR(patches_component_hook::GetClientSettingsRecord, memory_manager::find_function_by_string(L"Advanced Options Enabled", {0xe8}, 1), patches_component_hook::GetClientSettingsRecord_Hook);
    }

    {
        auto view_point_patch = memory_manager::find_pattern("74 1C 0F 57 C0 0F 2F 81 ? ? ? ? 73 10 4D 8B C6 48 8B D6");
        if (!view_point_patch) view_point_patch = memory_manager::find_pattern("74 ? 0F 57 C0 0F 2F 80 ? ? ? ? 73 ? 8B 9F ? ? ? ? 48 8B CF C1 EB 03");
        
        if (view_point_patch)
        {
            DWORD d2;
            DWORD d12;

            VirtualProtect(reinterpret_cast<LPVOID>(view_point_patch), 1, PAGE_EXECUTE_READWRITE, &d2);

            // Patch GetPlayerViewPoint function to prevent replication issues
            reinterpret_cast<char*>(view_point_patch)[0] = 0x75;

            VirtualProtect(reinterpret_cast<LPVOID>(view_point_patch), 1, d2, &d12);
        }
        else
        {
            spdlog::critical("Failed to setup patch 'GetPlayerViewPoint', invalid patch address provided");
        }
    }
    
    DetourTransactionCommit();
}

float patches_component_hook::GetMaxTickRate_Hook(UObject* Engine, float DeltaTime, bool bAllowSmoothing)
{
    // Force this function to act like this server an actual dedicated server
    if (UNetDriver* NetDriver = (*GWorld)->NetDriver())
    {
        return NetDriver->NetServerMaxTickRate();
    }

    return 30.f;
}

void patches_component_hook::UpdateNetStats_Hook(UObject* NetDebugWidget)
{
    // Do nothing to prevent crash
}

bool patches_component_hook::GetTagsFromPawnLocation_Hook(AFortPoiVolume* FortPoiVolume, long long Tags)
{
    if (FortPoiVolume->BrushComponent())
    {
        return GetTagsFromPawnLocation(FortPoiVolume, Tags);
    }

    return false;
}

__int64 patches_component_hook::TravelPatch_Hook(UObject* GameInstance, char a1)
{
    // Do nothing to prevent crash
    return 0;
}

__int64 patches_component_hook::LoadingScreenPatch_Hook(long long Object)
{
    // Do nothing to prevent crash
    return 0;
}

int patches_component_hook::GetNetMode_Hook(UNetDriver* NetDriver)
{
    return 1; // NM_DedicatedServer
}

int patches_component_hook::WorldGetNetMode_Hook(UWorld* World)
{
    return 1; // NM_DedicatedServer
}

UFortClientSettingsRecord* patches_component_hook::GetClientSettingsRecord_Hook(UFortLocalPlayer* LocalPlayer)
{
    if (!LocalPlayer->ClientSettingsRecord())
    {
        LocalPlayer->ClientSettingsRecord() = static_cast<UFortClientSettingsRecord*>(FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics")->SpawnObject(FindObject(L"/Script/FortniteGame.FortClientSettingsRecord"), LocalPlayer));
    }

    return LocalPlayer->ClientSettingsRecord();
}
