#include "include.hpp"
#include "patches_component.hpp"

#include "log_component.hpp"
#include "net_component.hpp"
#include "player_component.hpp"
#include "replication_component.hpp"

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
    TimesLoggedMSGSIZEExploit = 0;
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    // Hook ReplicateActor to patch a crash
    REGISTER_FUNCTION_DETOUR(patches_component_hook::ReplicateActor, memory_manager::find_function_by_string(L"ReplicateActor: bPausedUntilReliableACK"), patches_component_hook::ReplicateActor_Hook);

    // Hook cURL functions, used by attackers
    REGISTER_FUNCTION_DETOUR(patches_component_hook::CurlEasy, memory_manager::find_pattern("89 54 24 10 4C 89 44 24 18 4C 89 4C 24 20 48 83 EC 28 48 85 C9 75 08 8D 41 2B 48 83 C4 28 C3 4C"), patches_component_hook::CurlEasy_Hook);
    
    // Prevent players from having no pawn
    REGISTER_CALL_DETOUR_WITH_SAVE(memory_manager::find_pattern("E8 ? ? ? ? 48 89 44 24 ? 48 85 C0 0F 85 ? ? ? ? 49 63 5F 08 8D 4B 01 41 89 4F 08 41 3B 4F 0C 7E 0A 8B D3 49 8B CF"), patches_component_hook::SpawnActor_Pawn);
    
    // Re-implement GC checks
    REGISTER_FUNCTION_DETOUR(patches_component_hook::TryCollectGarbage, GetEngineVersionEasy() < 420 ? memory_manager::find_function_by_string(L"TryCollectGarbage: forcing GC after %d skipped attempts.") : 0, patches_component_hook::TryCollectGarbage_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::ConditionalCollectGarbage, GetEngineVersionEasy() > 419 ? memory_manager::find_function_by_string(L"STAT_GCSweepTime") : 0, patches_component_hook::ConditionalCollectGarbage_Hook);

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
    REGISTER_FUNCTION_DETOUR(patches_component_hook::FSocketBSD__RecvFrom, memory_manager::find_pattern("48 89 5C 24 ? 57 48 83 EC 30 44 8B 54 24 ? 49 8B D9 48 8B 44 24 ? 45 8B DA 41 83 E3 02"), patches_component_hook::FSocketBSD__RecvFrom_Hook);
    REGISTER_FUNCTION_DETOUR(patches_component_hook::PRIVATE_GetAthenaLoadoutWithOverrides, memory_manager::find_function_by_string(L"AFortPlayerController::PRIVATE_GetAthenaLoadoutWithOverrides(): MCP is enabled but we were unable to"), patches_component_hook::PRIVATE_GetAthenaLoadoutWithOverrides_Hook);
    if (!patches_component_hook::PRIVATE_GetAthenaLoadoutWithOverrides)
    {
        REGISTER_FUNCTION_DETOUR(patches_component_hook::PRIVATE_GetAthenaLoadoutWithOverrides, memory_manager::find_function_by_string(L"AFortPlayerController::GetAthenaLoadoutWithOverridesInternal(): MCP is enabled but we were unable to"), patches_component_hook::PRIVATE_GetAthenaLoadoutWithOverrides_Hook);
    }
    
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
#ifdef _RELEASE
        // Allow matchmaking
        if (GetFortniteVersionEasy() < 9.00f)
        {
            if (uintptr_t target_address = memory_manager::find_function_by_string(L"NotifyControlMessage: Client connecting with invalid version. LocalNetworkVersion: %i, RemoteNetworkVersion: %i", {0x7f}, 0 , false, false, true))
            {
                DWORD new_protect, old_protect;

                VirtualProtect(CreateDefaultObject<UFortGameInstance>()->VTable, 4096, PAGE_EXECUTE_READWRITE, &old_protect);

                *reinterpret_cast<unsigned char*>(target_address) = 0x7E;

                VirtualProtect(CreateDefaultObject<UFortGameInstance>()->VTable, 4096, old_protect, &new_protect);

                spdlog::info(L"Patched matchmaking [PATCH TYPE: 1]");
            }
        }
        else
        {
            if (uintptr_t target_address = memory_manager::find_function_by_string(L"NotifyControlMessage: Client connecting with invalid version. LocalNetworkVersion: %i, RemoteNetworkVersion: %i", {0x1, 0x7f}, 0 , false, false, true))
            {
                DWORD new_protect, old_protect;

                VirtualProtect(CreateDefaultObject<UFortGameInstance>()->VTable, 4096, PAGE_EXECUTE_READWRITE, &old_protect);

                *reinterpret_cast<unsigned char*>(target_address + 1) = 0x7E;

                VirtualProtect(CreateDefaultObject<UFortGameInstance>()->VTable, 4096, old_protect, &new_protect);

                spdlog::info(L"Patched matchmaking [PATCH TYPE: 2]");
            }
        }
#endif
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
        const UObject* DefaultGameModeBaseObject = FindObject(L"/Script/FortniteGame.Default__FortGameMode");

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

        // patch reservations (Only on below 1.11)

        if (GetEngineVersionEasy() < 419)
        {
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

    {
        // Ignore client/server context on SpawnActor
        uintptr_t target_address = memory_manager::find_pattern("75 4E 80 3D ? ? ? ? ? 0F 82 ? ? ? ? 48 8B 46 18 48 8D 55 80 48 8D 4D 90 48 89 45 90 E8 ? ? ? ? 48 8B 05 ? ? ? ? 4C 8D 44 24 ?");
        if (!target_address)
        {
            target_address = memory_manager::find_pattern("75 4F 80 3D ? ? ? ? ? 0F 82 ? ? ? ? 48 8B 46 18 48 8D 54 24 ? 48 8D 4D C0 48 89 45 C0 E8 ? ? ? ? 48 8B 05 ? ? ? ? 4C 8D");
        }
        if (target_address)
        {
            DWORD new_protection, old_protection;

            VirtualProtect(reinterpret_cast<LPVOID>(target_address), 2, PAGE_EXECUTE_READWRITE, &old_protection);

            // JNZ -> EB
            reinterpret_cast<uint8_t*>(target_address)[0] = 0xEB;
            
            VirtualProtect(reinterpret_cast<LPVOID>(target_address), 2, old_protection, &new_protection);
        }
        else
        {
            spdlog::error(L"Failed to patch client/server context checks in UWorld::SpawnActor.");
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

            if (bDisableGCOnServerDuringMatch == true && (*GWorld)->NetDriver())
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

            if (bDisableGCOnServerDuringMatch == true && (*GWorld)->NetDriver())
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

    if (!FortPoiVolume->EraPoiInfo().bLoggedInvalidBrushComponent)
    {
        FortPoiVolume->EraPoiInfo().bLoggedInvalidBrushComponent = true;
        spdlog::error(L"GetTagsFromPawnLocation_Hook found Volume [{}] with no BrushComponent.", FortPoiVolume->GetName().c_str());
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

FFortAthenaLoadout* patches_component_hook::PRIVATE_GetAthenaLoadoutWithOverrides_Hook(AFortPlayerControllerAthena* PlayerController, FFortAthenaLoadout* OutLoadout)
{
    // Get aircraft index in enum
    static UEnum* GamePhaseEnum = FindObject<UEnum>(TEXT("/Script/FortniteGame.EAthenaGamePhase"));
    uint8_t Aircraft = GamePhaseEnum->GetEnumIndex(TEXT("Aircraft"));
    
    // Only execute the code on prelobby
    if ((*GWorld)->GameState()->GamePhase() < Aircraft)
    {
        // Clear
        for (auto& CharacterVariantChannel : TArrayManager<FMcpVariantChannelInfo>(&PlayerController->CustomizationLoadout().CharacterVariantChannels(), FMcpVariantChannelInfo::GetStructSize()))
        {
            CharacterVariantChannel.OwnedVariantTags().GameplayTags.Free();
            CharacterVariantChannel.OwnedVariantTags().ParentTags.Free();
        }
    
        PlayerController->CustomizationLoadout().CharacterVariantChannels().Free();
    
        memcpy(OutLoadout, &PlayerController->CustomizationLoadout(), FFortAthenaLoadout::GetStructSize());

        if (FindPropertyByName(PlayerController->CustomizationLoadout().GetClass(), L"CharacterVariantChannels"))
        {
            // Apply variants
            if (STATIC_HASPROPERTY(PlayerController, EraPlayerInfo))
            {
                for (std::pair<nlohmann::json, UObject*> variant_data : PlayerController->EraPlayerInfo().PendingVariantsData)
                {
                    player_component::apply_variant(OutLoadout, variant_data.first, variant_data.second);
                }
            }
        }
            
        return OutLoadout;
    }
    else
    {
        return PRIVATE_GetAthenaLoadoutWithOverrides(PlayerController, OutLoadout);
    }
}

APawn* patches_component_hook::SpawnActor_Pawn_Hook(UWorld* Self, UObject* Class, FVector& Location, FRotator& Rotation, const FActorSpawnParameters& Params)
{
    // When we're spawning an athena player pawn, act accordingly
    if (Class->This_IsA(APlayerPawn_Athena_C::StaticClass()))
    {
        if (APawn* ReturnValue = SpawnActor_Pawn(Self, Class, Location, Rotation, Params))
        {
            return ReturnValue;
        }

        // Call function again with offseted location
        APawn* ReturnValue = nullptr;
        do
        {
            spdlog::error(L"SpawnActor_Pawn_Hook failed. Calling again with a different location...");
        
            Location.Z -= 200.f;
        
            ReturnValue = SpawnActor_Pawn(Self, Class, Location, Rotation, Params);

            if (ReturnValue)
            {
                spdlog::info(L"SpawnActor_Pawn_Hook succeeded.");
            }
        } while (ReturnValue == nullptr);

        return ReturnValue;
    }

    return SpawnActor_Pawn(Self, Class, Location, Rotation, Params);
}

bool patches_component_hook::FSocketBSD__RecvFrom_Hook(void* Self, uint8* Data, int32 BufferSize,  int32& BytesRead, void* Source, int Flags)
{
    bool ReturnValue = FSocketBSD__RecvFrom(Self, Data, BufferSize, BytesRead, Source, Flags);
    
    if (!ReturnValue)
    {
        if (GetLastError() == WSAEMSGSIZE)
        {
            if (TimesLoggedMSGSIZEExploit < 10)
            {
                spdlog::error(L"patches_component_hook::FSocketBSD__RecvFrom_Hook: EMSGSIZE, ignoring packet");
                
                TimesLoggedMSGSIZEExploit++;
            }

            ReturnValue = true;
            BytesRead = 0;
        }
    }

    return ReturnValue;
}

int patches_component_hook::CurlEasy_Hook()
{
    return 43; // Don't process anything cURL related   
}

uintptr_t patches_component_hook::ReplicateActor_Hook(UActorChannel* Self)
{
    if (NetworkOffsets::ActorReplicator != 0x0 &&
        NetworkOffsets::ObjectPtr != 0x0)
    {
        // Make sure Self is valid
        if (Self)
        {
            // Check if it's blacklisted
            if (Self->Actor())
            {
                if (std::find(replication_component::BlacklistedActor.begin(), replication_component::BlacklistedActor.end(), Self->Actor()) != replication_component::BlacklistedActor.end())
                {
                    if (log_component::enable_debug_logs)
                    {
                        spdlog::error(L"Found blacklisted Actor in ReplicateActor_Hook.");
                    }
                    
                    return 0;
                }
            }
        
            // Make sure both the Actor Replicator and the Object ptr are valid
            if (!Self->ActorReplicator() ||
                !Self->ActorReplicator()->ObjectPtr())
            {
                if (Self->Actor())
                {
                    spdlog::warn(L"Blacklisted actor: invalid Actor Replicator or Object Ptr on channel [{}]", replication_component::BlacklistedActor.size());
                    
                    replication_component::BlacklistedActor.push_back(Self->Actor());
                }
                
                return 0;
            }
        }
    }

    return ReplicateActor(Self);
}
