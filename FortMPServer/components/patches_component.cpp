#include "include.hpp"
#include "patches_component.hpp"

#include "net_component.hpp"

anti_tampering_bypass::anti_tampering_bypass()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    // Disable anti tampering
    REGISTER_FUNCTION_DETOUR(anti_tampering_bypass_hooks::AntiTamper, memory_manager::find_pattern("E8 ? ? ? ? 48 8D 4C 24 ? FF 15 ? ? ? ? 0F 57 C0 F2 48 0F 2A 44 24 ? F2 0F 59 05 ? ? ? ? F2 0F 58 05 ? ? ? ? F2 0F 11 05 ? ? ? ? 48 83 C4 28 C3", true, 1), anti_tampering_bypass_hooks::AntiTamper_Hook);
    
    DetourTransactionCommit();
}

uintptr_t anti_tampering_bypass_hooks::AntiTamper_Hook()
{
    spdlog::info(L"AntiTamper called, bypassing.");

    return 0;
}

patches_component::patches_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    // Re-implement GC checks
    REGISTER_FUNCTION_DETOUR(patches_component_hook::TryCollectGarbage, GetEngineVersionEasy() < 421 ? memory_manager::find_function_by_string(L"TryCollectGarbage: forcing GC after %d skipped attempts.") : 0, patches_component_hook::TryCollectGarbage_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::ConditionalCollectGarbage, GetEngineVersionEasy() > 420 ? memory_manager::find_function_by_string(L"STAT_GCSweepTime") : 0, patches_component_hook::ConditionalCollectGarbage_Hook);

    // Stop random crashing
    REGISTER_FUNCTION_DETOUR(patches_component_hook::FortPVPGameOverlayerCrash, memory_manager::find_function_by_string(L"Fortnite.Font.S.Normal"), patches_component_hook::FortPVPGameOverlayerCrash_Hook);
    
    // Limit the tickrate
    REGISTER_FUNCTION_DETOUR(patches_component_hook::GetMaxTickRate, memory_manager::find_function_by_string(L"Hitching by request!"), patches_component_hook::GetMaxTickRate_Hook);

    // start match patch
    REGISTER_FUNCTION_DETOUR(patches_component_hook::GetTagsFromPawnLocation, memory_manager::find_pattern("E8 ? ? ? ? 44 0F B6 F0 84 C0 0F 84 ? ? ? ? 48 8D 97 ? ? ? ? 49 8B CC E8 ? ? ? ?", true, 1), patches_component_hook::GetTagsFromPawnLocation_Hook);

    // Loading crash fix
    REGISTER_FUNCTION_DETOUR(patches_component_hook::UpdateNetStats, memory_manager::find_pattern("40 55 56 41 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 8B 01"), patches_component_hook::UpdateNetStats_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::UpdateNetStats, memory_manager::find_function_by_string(L"STAT_NetDebugWidget_UpdateNetStats"), patches_component_hook::UpdateNetStats_Hook);

    // GetNetMode and WorldGetNetMode are hooked to make Fortnite think we are a dedicated server;
    // With those hooks, double damage numbers, glider being attached to the pawn and other minor issues such as items not working will be fixed.
    REGISTER_FUNCTION_DETOUR(patches_component_hook::GetNetMode, memory_manager::find_pattern("48 83 EC 28 48 8B 01 FF 90 ? ? ? ? 84 C0 74 12 33 C0 38 05 ? ? ? ? 0F 95 C0 FF C0 48 83 C4 28"), patches_component_hook::GetNetMode_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::WorldGetNetMode, memory_manager::find_pattern("40 53 48 81 EC ? ? ? ? 48 83 79 ? ? 48 8B D9 74 0E B8 ? ? ? ? 48 81 C4 ? ? ? ? 5B"), patches_component_hook::WorldGetNetMode_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::WorldGetNetMode, memory_manager::find_function_by_string(L"not creating navigation on clients", {0xe8}, 1), patches_component_hook::WorldGetNetMode_Hook);

    REGISTER_FUNCTION_DETOUR(patches_component_hook::AddSquadMember, memory_manager::find_function_by_string(L"AddSquadMember: %s %d [%s]"), patches_component_hook::AddSquadMember_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::StatManagerFunc, memory_manager::find_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 30 49 63 F0 80 FA 02 0F 83 ? ? ? ? 0F B6 D2"), patches_component_hook::StatManagerFunc_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::NotifyAcceptingConnection, memory_manager::find_function_by_string(L"NotifyAcceptingConnection: Client refused"), patches_component_hook::NotifyAcceptingConnection_Hook);
    
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
        // FLASHING FIX

        if (const uintptr_t target_address = memory_manager::find_pattern("FF 90 ? ? ? ? F2 0F 10 44 24 ? 8B 44 24 28 48 8B 8C 24 ? ? ? ?"))
        {
            const UObject* DefaultCameraObject = FindObject(L"/Script/FortniteGame.Default__FortPlayerCameraZone") ?
                FindObject(L"/Script/FortniteGame.Default__FortPlayerCameraZone") :
                FindObject(L"/Script/FortniteGame.Default__FortPlayerCameraBase");
        
            const UObject* DefaultCameraObject_Replace = FindObject(L"/Script/Engine.Default__PlayerCameraManager");

            const int target_patch_index = *reinterpret_cast<int*>(target_address + 2) / 8;

            DWORD old_protect, new_protect;
    
            VirtualProtect(DefaultCameraObject->VTable, 4096, PAGE_EXECUTE_READWRITE, &old_protect);

            DefaultCameraObject->VTable[target_patch_index - 2] = DefaultCameraObject_Replace->VTable[target_patch_index - 2];
            DefaultCameraObject->VTable[target_patch_index - 1] = DefaultCameraObject_Replace->VTable[target_patch_index - 1];
            DefaultCameraObject->VTable[target_patch_index] = DefaultCameraObject_Replace->VTable[target_patch_index];
            DefaultCameraObject->VTable[target_patch_index + 1] = DefaultCameraObject_Replace->VTable[target_patch_index + 1];
    
            VirtualProtect(DefaultCameraObject->VTable, 4096, old_protect, &new_protect);
        }
        else
        {
            spdlog::error(L"Failed to patch DefaultCameraObject's VFTable, pattern not found.");
        }
    }

    {
        // Re-add AFortPlayerController::ServerSuicide

        const UObject* DefaultFortPlayerControllerObject = FindObject(L"/Script/FortniteGame.Default__FortPlayerController");
        const UObject* DefaultFortPlayerControllerAthenaObject = FindObject(L"/Script/FortniteGame.Default__FortPlayerControllerAthena");

        if (const int target_index = memory_manager::find_function_by_string(L"ServerSuicide_Validate", {0x48, 0xFF, 0xA0}, 3, false, false))
        {
            DWORD old_protect, new_protect;

            VirtualProtect(DefaultFortPlayerControllerAthenaObject->VTable, 8192, PAGE_EXECUTE_READWRITE, &old_protect);

            DefaultFortPlayerControllerAthenaObject->VTable[target_index / 8] = DefaultFortPlayerControllerObject->VTable[target_index / 8];
            
            VirtualProtect(DefaultFortPlayerControllerAthenaObject->VTable, 8192, old_protect, &new_protect);
        }
        else
        {
            spdlog::error(L"Failed to patch ServerSuicide");
        }
    }

    {
        // Pawn spawning patches

        const UObject* DefaultFortGameModeAthenaObject = FindObject(L"/Script/FortniteGame.Default__FortGameModeAthena");
        const UObject* DefaultGameModeBaseObject = FindObject(L"/Script/Engine.Default__GameModeBase");

        // Use Engine's SpawnDefaultPawnFor
        if (const uintptr_t target_address = memory_manager::find_function_by_string(L"SpawnDefaultPawnFor: Fail"))
        {
            for (auto i = 0; i < INT_MAX; i++)
            {
                if (DefaultFortGameModeAthenaObject->VTable[i] != nullptr &&
                    DefaultFortGameModeAthenaObject->VTable[i] == reinterpret_cast<LPVOID>(target_address))
                {
                    DWORD new_protect, old_protect;

                    VirtualProtect(DefaultFortGameModeAthenaObject->VTable, 4096, PAGE_EXECUTE_READWRITE, &old_protect);

                    DefaultFortGameModeAthenaObject->VTable[i] = DefaultGameModeBaseObject->VTable[i];

                    VirtualProtect(DefaultFortGameModeAthenaObject->VTable, 4096, old_protect, &new_protect);

                    break;
                }
            }
        }
        else
        {
            spdlog::error(L"Failed to patch SpawnDefaultPawnFor: invalid target_address");
        }
    }

    {
        // NOMCP BEHAVIOUR ON PRE 1.10

        if (const uintptr_t target_address = memory_manager::find_pattern("FF 97 ? ? ? ? 48 8B 8D ? ? ? ? 84 C0 0F 94 C3 48 85 C9 74 05 E8 ? ? ? ?"))
        {
            const UFortLocalPlayer* DefaultLocalPlayer = FindObject<UFortLocalPlayer>(L"/Script/FortniteGame.Default__FortLocalPlayer");

            const int target_hook_idx = *reinterpret_cast<int*>(target_address + 2) / 8;

            INIT_POINTER(patches_component_hook::SpawnPlayActor, DefaultLocalPlayer->VTable[target_hook_idx]);
            
            DWORD old_protect, new_protect;

            VirtualProtect(DefaultLocalPlayer->VTable, 4096, PAGE_EXECUTE_READWRITE, &old_protect);

            DefaultLocalPlayer->VTable[target_hook_idx] = patches_component_hook::SpawnPlayActor_Hook;
            
            VirtualProtect(DefaultLocalPlayer->VTable, 4096, old_protect, &new_protect);
        }
        else
        {
            spdlog::error(L"Failed to patch UFortLocalPlayer::SpawnPlayActor, pattern not found");
        }

        // patch reservations
        
        if (const uintptr_t target_address = memory_manager::find_pattern("0F 84 ? ? ? ? 48 85 DB 0F 84 ? ? ? ? 48 8B 8B ? ? ? ? 48 85"))
        {
            DWORD old_protect, new_protect;
            
            VirtualProtect(reinterpret_cast<LPVOID>(target_address), 2, PAGE_EXECUTE_READWRITE, &old_protect);
            
            reinterpret_cast<char*>(target_address)[1] = 0x85;
                        
            VirtualProtect(reinterpret_cast<LPVOID>(target_address), 2, old_protect, &new_protect);
        }
        else
        {
            spdlog::error(L"Failed to fix reservations, pattern not found");
        }
    }

    {
        // Don't kick on "ServerUpdateLevelVisibility() ignored non-existant package"
        if (const uintptr_t target_address = memory_manager::find_function_by_string(L"ServerUpdateLevelVisibility() ignored non-existant package"))
        {
            // Get all calls inside the function we found
            auto calls = memory_manager::get_all_calls_in_function(target_address);

            for (auto call : calls)
            {
                if (auto called_function = call + 5 + *reinterpret_cast<int*>(call + 1))
                {
                    if (called_function == reinterpret_cast<uintptr_t>(NetConnection__Close))
                    {
                        // Call hook
                        memory_manager::call_hook(call, reinterpret_cast<uintptr_t>(patches_component_hook::NetConnection__Close__Replacement));
                        break;
                    }
                }
            }
        }
    }
    
    DetourTransactionCommit();
}

long long patches_component_hook::FortPVPGameOverlayerCrash_Hook()
{
    // Do nothing to prevent crash
    return 0;
}

bool patches_component_hook::TryCollectGarbage_Hook(int KeepFlags, bool bPerformFullPurge)
{
    if (*GWorld && (*GWorld)->AuthorityGameMode() && (*GWorld)->GameState())
    {
        if (STATIC_HASPROPERTY((*GWorld)->AuthorityGameMode(), bDisableGCOnServerDuringMatch))
        {
            const bool bDisableGCOnServerDuringMatch = (*GWorld)->AuthorityGameMode()->bDisableGCOnServerDuringMatch();

            static FName NAME_InProgress(L"InProgress");
            
            if (bDisableGCOnServerDuringMatch == true && (*GWorld)->GameState()->MatchState() == NAME_InProgress)
            {
                return false;
            }
        }
    }

    spdlog::info(L"Performing garbage collection.");

    return TryCollectGarbage(KeepFlags, bPerformFullPurge);
}

void patches_component_hook::ConditionalCollectGarbage_Hook(UObject* Self)
{
    if (*GWorld && (*GWorld)->AuthorityGameMode() && (*GWorld)->GameState())
    {
        if (STATIC_HASPROPERTY((*GWorld)->AuthorityGameMode(), bDisableGCOnServerDuringMatch))
        {
            const bool bDisableGCOnServerDuringMatch = (*GWorld)->AuthorityGameMode()->bDisableGCOnServerDuringMatch();

            static FName NAME_InProgress(L"InProgress");
            
            if (bDisableGCOnServerDuringMatch == true && (*GWorld)->GameState()->MatchState() == NAME_InProgress)
            {
                return;
            }
        }
    }

    return ConditionalCollectGarbage(Self);
}


bool patches_component_hook::SpawnPlayActor_Hook(ULocalPlayer* LocalPlayer, uintptr_t URL, uintptr_t Error, uintptr_t World)
{
    // Call native function
    SpawnPlayActor(LocalPlayer, URL, Error, World);

    // Force return true
    return true;
}

float patches_component_hook::GetMaxTickRate_Hook(UObject* Engine, float DeltaTime, bool bAllowSmoothing)
{
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
        LocalPlayer->ClientSettingsRecord() = static_cast<UFortClientSettingsRecord*>(UGameplayStatics::SpawnObject(FindObject(L"/Script/FortniteGame.FortClientSettingsRecord"), LocalPlayer));
    }

    return LocalPlayer->ClientSettingsRecord();
}

void patches_component_hook::AddSquadMember_Hook(void* Self, APlayerState* PlayerState)
{
    if (memory_manager::is_bad_read_pointer(PlayerState))
    {
        spdlog::error(L"patches_component_hook::AddSquadMember_Hook: PlayerState is bad read pointer.");

        // The c++ function can handle nullptr playerstates
        PlayerState = nullptr;
    }

    return AddSquadMember(Self, PlayerState);
}

void* patches_component_hook::StatManagerFunc_Hook(void* Self, uint8_t Unk1, int unk2)
{
    void* Result = StatManagerFunc(Self, Unk1, unk2);

    if (Result != nullptr)
    {
        if (memory_manager::is_bad_read_pointer(Result))
        {
            return nullptr;
        }
    }
    
    return Result;
}

int patches_component_hook::NotifyAcceptingConnection_Hook(UWorld* Self)
{
    if (*GWorld && (*GWorld)->AuthorityGameMode() && (*GWorld)->NetDriver())
    {
        if (AGameSession* GameSession = (*GWorld)->AuthorityGameMode()->GameSession())
        {
            if ((*GWorld)->NetDriver()->ClientConnections().Num() >= GameSession->MaxPlayers())
            {
                // Reject
                return 0;
            }
        }
    }

    return NotifyAcceptingConnection(Self);
}

void patches_component_hook::NetConnection__Close__Replacement()
{
    // Do nothing   
}