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

    static void pre_init();

    // Anti tampering kicks in after this function is called. Here we revert every patch applied in pre_init
    static void revert_pre_init();

    struct FEraPatchInfo
    {
        uintptr_t Address;
        PVOID Bytes;
        SIZE_T Size;

        void ParseBytes()
        {
            Bytes = malloc(Size);
            memcpy(Bytes, reinterpret_cast<PVOID>(Address), Size);
        }

        void Apply() const
        {
            DWORD OldProtect, NewProtect;
            VirtualProtect(reinterpret_cast<PVOID>(Address), 4096, PAGE_EXECUTE_READWRITE, &OldProtect);

            memcpy(reinterpret_cast<PVOID>(Address), Bytes, Size);
            
            VirtualProtect(reinterpret_cast<PVOID>(Address), 4096, OldProtect, &NewProtect);
        }

        void Free() const
        {
            free(Bytes);
        }
    };
    
    inline static std::vector<FEraPatchInfo> temporary_patches;
};

namespace patches_component_hook
{
    DEFINE_FUNCTION_HOOK(uintptr_t, ReplicateActor, (UActorChannel* Self));
    
    DEFINE_FUNCTION_HOOK(UFortClientSettingsRecord*, GetClientSettingsRecord, (UFortLocalPlayer* LocalPlayer));

    DEFINE_FUNCTION_HOOK(bool, SpawnPlayActor, (ULocalPlayer* LocalPlayer, uintptr_t URL, uintptr_t Error, uintptr_t World));
    
    DEFINE_FUNCTION_HOOK(int, GetNetMode, (UNetDriver* NetDriver));
    DEFINE_FUNCTION_HOOK(int, WorldGetNetMode, (UWorld* World));
    DEFINE_FUNCTION_HOOK(FFortAthenaLoadout*, PRIVATE_GetAthenaLoadoutWithOverrides, (AFortPlayerControllerAthena* PlayerController, FFortAthenaLoadout* OutLoadout));
    DEFINE_FUNCTION_HOOK(FFortAthenaLoadout*, GetAthenaLoadoutWithOverrides, (AFortPlayerControllerAthena* PlayerController, PVOID, PVOID));
    DEFINE_FUNCTION_HOOK(char, ReservationCheck, (UObject* a1, __int64 a2, char a3, __int64* a4));
    DEFINE_FUNCTION_HOOK(bool, McpTempCheck, (UObject* a1));
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

    DEFINE_FUNCTION_HOOK(APawn*, SpawnActor_Pawn, (UWorld* Self, UObject* Class, FVector& Location, FRotator& Rotation, const FActorSpawnParameters& Params))
    DEFINE_FUNCTION_HOOK(APawn*, SpawnDefaultPawnAtTransform_Implementation, (AGameModeBase* Self, APlayerController* NewPlayer, FTransform& SpawnTransform))

    // Prevents a crash related to updating the net debug stats widget
    void UpdateNetStats_Hook(UObject* NetDebugWidget);

    // Prevents a crash from using StartMatch
    bool GetTagsFromPawnLocation_Hook(AFortPoiVolume* FortPoiVolume, __int64 Tags);

    // Prevents a crash when travelling to a new level
    __int64 TravelPatch_Hook(UObject* GameInstance, char a1);

    // Prevents a crash related to the loading screen
    __int64 LoadingScreenPatch_Hook(__int64 Object);

    // Replace call to NetConnection::Close
    void NetConnection__Close__Replacement();

    DEFINE_FUNCTION_HOOK(bool, FSocketBSD__RecvFrom, (void* Self, uint8* Data, int32 BufferSize, int32& BytesRead, void* Source, int Flags));

    DEFINE_FUNCTION_HOOK(int, CurlEasy, ());

    DEFINE_FUNCTION_HOOK(FGameplayTagContainer*, __Filter, (FGameplayTagContainer* Self, FGameplayTagContainer* Result, FGameplayTagContainer* OtherContainer));

    DEFINE_FUNCTION_HOOK(void, FortDamageFormula_Execute, (UObject* Self, struct FGameplayEffectCustomExecutionParameters* ExecutionParams, FGameplayEffectCustomExecutionOutput* OutExecutionOutput))

    DEFINE_FUNCTION_HOOK(bool, SerializeObject, (UObject* Self, void* Ar, UObject* Class, UObject** Object, void* OutNetworkGuid))
};

inline int TimesLoggedMSGSIZEExploit;