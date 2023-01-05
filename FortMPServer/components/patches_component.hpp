#pragma once

class anti_tampering_bypass
{
public:
    anti_tampering_bypass();
};

namespace anti_tampering_bypass_hooks
{
    DEFINE_FUNCTION_HOOK(uintptr_t, AntiTamper, ());
}

class patches_component
{
public:
    patches_component();
};

namespace patches_component_hook
{
    DEFINE_FUNCTION_HOOK(UFortClientSettingsRecord*, GetClientSettingsRecord, (UFortLocalPlayer* LocalPlayer));

    DEFINE_FUNCTION_HOOK(bool, SpawnPlayActor, (ULocalPlayer* LocalPlayer, uintptr_t URL, uintptr_t Error, uintptr_t World));
    
    DEFINE_FUNCTION_HOOK(int, GetNetMode, (UNetDriver* NetDriver));
    DEFINE_FUNCTION_HOOK(int, WorldGetNetMode, (UWorld* World));

    DEFINE_FUNCTION_HOOK(float, GetMaxTickRate, (UObject* Engine, float DeltaTime, bool bAllowSmoothing));
    DEFINE_FUNCTION_HOOK(bool, TryCollectGarbage, (int KeepFlags, bool bPerformFullPurge));
    DEFINE_FUNCTION_HOOK(void, ConditionalCollectGarbage, (UObject* Self));

    DEFINE_FUNCTION_HOOK(void, AddSquadMember, (void* Self, APlayerState* PlayerState));

    DEFINE_FUNCTION_HOOK(void*, StatManagerFunc, (void* Self, uint8_t Unk1, int unk2));
    
    inline void (*UpdateNetStats)(UObject* NetDebugWidget);
    inline bool (*GetTagsFromPawnLocation)(AFortPoiVolume* FortPoiVolume, __int64 Tags);
    inline __int64 (*TravelPatch)(UObject* GameInstance, char a1);
    inline __int64 (*LoadingScreenPatch)(__int64 Object);

    DEFINE_FUNCTION_HOOK(int, NotifyAcceptingConnection, (UWorld*));

    DEFINE_FUNCTION_HOOK(__int64, FortPVPGameOverlayerCrash, ());

    // Prevents a crash related to updating the net debug stats widget
    void UpdateNetStats_Hook(UObject* NetDebugWidget);

    // Prevents a crash from using StartMatch
    bool GetTagsFromPawnLocation_Hook(AFortPoiVolume* FortPoiVolume, __int64 Tags);

    // Prevents a crash when travelling to a new level
    __int64 TravelPatch_Hook(UObject* GameInstance, char a1);

    // Prevents a crash related to the loading screen
    __int64 LoadingScreenPatch_Hook(__int64 Object);
};