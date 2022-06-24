#include "include.hpp"
#include "patches_component.hpp"

patches_component::patches_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(patches_component_hook::GetPlayerViewPoint, memory_manager::find_function_by_string(L"APlayerController::GetPlayerViewPoint: out_Location, ViewTarget=%s"), patches_component_hook::GetPlayerViewPoint_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::GetTagsFromPawnLocation, memory_manager::find_pattern("48 8B C4 57 48 81 EC ? ? ? ? 4C 8B 82 ? ? ? ? 48 8B F9 0F 29 70 E8 0F 29 78 D8 44 0F 29 40 ? F3 45 0F 10 80 ? ? ? ? F3 41 0F 10 B0 ? ? ? ? F3 41 0F 10 88 ? ? ? ? F3 41 0F 10 A0 ? ? ? ? 44 0F 29 58 ? 44 0F 29 60 ? 45 0F 57 E4"), patches_component_hook::GetTagsFromPawnLocation_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::GetTagsFromPawnLocation, memory_manager::find_function_by_string(L"STAT_PoiVolume_CheckPawnOverlap", { 0xE8, 0x100, 0x100, 0x1 }, 1, false), patches_component_hook::GetTagsFromPawnLocation_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::UpdateNetStats, memory_manager::find_pattern("40 55 56 41 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 8B 01"), patches_component_hook::UpdateNetStats_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::UpdateNetStats, memory_manager::find_function_by_string(L"STAT_NetDebugWidget_UpdateNetStats"), patches_component_hook::UpdateNetStats_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::TravelPatch, memory_manager::find_function_by_string(L"OnInitWorldRecordComplete: Invalid tile", {0xe8, 0x100, 0x100, 0x100, 0xff}, 1, false), patches_component_hook::TravelPatch_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::LoadingScreenPatch, memory_manager::find_pattern("48 89 5C 24 ? 57 48 83 EC 30 48 8B 01 48 8B F9 FF 90 ? ? ? ? 48 8B D8 48 85 C0 0F 84 ? ? ? ? 48 83 78 ? ? 0F 84 ? ? ? ? 48 8B 17 48 8B CF"), patches_component_hook::LoadingScreenPatch_Hook);

    DetourTransactionCommit();
}

void patches_component_hook::GetPlayerViewPoint_Hook(AController* PlayerController, FVector& out_Location, FRotator& out_Rotation)
{
    if (PlayerController->NetConnection())
    {
        AActor* view_target = PlayerController->NetConnection()->ViewTarget();

        if (view_target)
        {
            out_Location = view_target->K2_GetActorLocation();
            out_Rotation = view_target->K2_GetActorRotation();
        }
    }
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