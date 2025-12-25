#include "include.hpp"
#include "unreal.hpp"

#include "../components/net_component.hpp"
#include "../components/replication_component.hpp"
#include "../components/customproperties_component.hpp"
#include "../components/qos_component.hpp"

void unreal_manager::pre_init()
{
    // Init
    isUnrealInitialized = false;
    
#ifdef ERA_LL_LOGS
    debug_log << L"unreal_manager::pre_init: Initializing custom properties (pre-init)." << std::endl;
#endif
    
    // hook
    customproperties_component::pre_init();

#ifdef ERA_LL_LOGS
    debug_log << L"unreal_manager::pre_init: Successfully processed custom properties' pre-init stage." << std::endl;
#endif
    
    std::vector<async_wrapper*> early_async_initializers;
    early_async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(FMemory_Malloc_, memory_manager::find_pattern("4C 8B C9 48 8B 0D ? ? ? ? 48 85 C9 75 08 49 8B C9 E9 ? ? ? ? 48 8B 01 44 8B C2 49 8B D1 48 FF 60 10"));
        if (!FMemory_Malloc_)
        {
            INIT_POINTER(FMemory_Malloc_, memory_manager::find_pattern("48 89 5C 24 ? 57 48 83 EC 20 48 8B F9 8B DA 48 8B 0D ? ? ? ? 48 85 C9 75 0C E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 8B 01 44 8B C3 48 8B D7 48 8B 5C 24 ? 48 83 C4 20 5F 48 FF 60 10"));
        }
    }));
    early_async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(GetEngineVersion, memory_manager::find_pattern("40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B C8 41 B8 04 ? ? ? 48 8B D3"));
    }));
    early_async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(ConstructUScriptStruct, memory_manager::find_pattern("40 55 53 56 57 41 55 41 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 33 F6 4C 8B F2 89 74 24 40 4C 8B E9 48 8B 02 48 8B 5A 08 48 8B 7A 10 48 85 C0 74 09 FF D0 48 89 44 24 ? EB 05 48 89 74 24 ? 48 85 DB"));
        if (!ConstructUScriptStruct)
        {
            INIT_POINTER(ConstructUScriptStruct, memory_manager::find_pattern("40 53 56 57 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 33 F6 4C 8B F2 89 74 24 40 4C 8B F9 48 8B 02 48 8B 5A 08 48 8B 7A 10 48 85 C0 74 07 FF D0"));
        }
    }));
    early_async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(RequestExitWithStatus, memory_manager::find_function_by_string(L"FPlatformMisc::RequestExitWithStatus(%i, %d)"));
    }));
    early_async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(StaticFindObject, memory_manager::find_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 30 80 3D ? ? ? ? ? 41 0F B6 D9 49 8B F8 48 8B F2"));
    }));
    early_async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(FName_ToString, memory_manager::find_pattern("E8 ? ? ? ? F3 41 0F 10 06 48 8D 15 ? ? ? ? F3 0F 59 05 ? ? ? ? 48 8B CF", true, 1));
    }));
    early_async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(FMemory_Free, memory_manager::find_pattern("E8 ? ? ? ? 48 8B 0F 48 89 0B 4C 89 37 8B 47 08 89 43 08 8B 47 0C 89 43 0C 4C 89 77 08", true, 1));
    }));
    async_wrapper::wait_for_wrappers(early_async_initializers);

#ifdef ERA_LL_LOGS
    debug_log << L"unreal_manager::pre_init: Processed basic wrappers." << std::endl;
#endif
    
    // initialize offsets
    if (GetEngineVersionEasy() == 416 ||
        GetEngineVersionEasy() == 419 ||
        GetEngineVersionEasy() == 420 ||
        GetEngineVersionEasy() == 421)
    {
        CoreOffsets::Children = 0x38;
        CoreOffsets::Next = 0x28;
        CoreOffsets::StructSize = 0x40;
        CoreOffsets::Offset_Internal = 0x44;
        CoreOffsets::ElementSize = 0x34;
        CoreOffsets::PropertyFlags = 0x38;
        CoreOffsets::SuperStruct = 0x30;
        CoreOffsets::ByteOffset = 0x71;
        CoreOffsets::FieldMask =  0x73;
        CoreOffsets::Script = 0x48;
    }
    else if (GetEngineVersionEasy() == 422 ||
             GetEngineVersionEasy() == 423 ||
             GetEngineVersionEasy() == 424)
    {
        CoreOffsets::Children = 0x48;
        CoreOffsets::Next = 0x28;
        CoreOffsets::StructSize = 0x50;
        CoreOffsets::Offset_Internal = 0x44;
        CoreOffsets::ElementSize = 0x34;
        CoreOffsets::PropertyFlags = 0x38;
        CoreOffsets::SuperStruct = 0x40;
        CoreOffsets::ByteOffset = 0x71;
        CoreOffsets::FieldMask =  0x73;
        CoreOffsets::Script = 0x58;
    }
    else if (GetEngineVersionEasy() == 425 ||
             GetEngineVersionEasy() == 426)
    {
        CoreOffsets::Children = 0x50;
        CoreOffsets::Next = 0x20;
        CoreOffsets::StructSize = 0x58;
        CoreOffsets::Offset_Internal = 0x4C;
        CoreOffsets::ElementSize = 0x3C;
        CoreOffsets::PropertyFlags = 0x40;
        CoreOffsets::SuperStruct = 0x40;
        CoreOffsets::ByteOffset = 121;
        CoreOffsets::FieldMask =  123;
        CoreOffsets::Script = 0x60;
    }

#ifdef ERA_LL_LOGS
    debug_log << L"unreal_manager::pre_init: Finished UNREAL initialization.." << std::endl;
#endif
    
    isUnrealInitialized = true;
    
    // Create custom properties component
    new customproperties_component();
    
#ifdef ERA_LL_LOGS
    debug_log << L"unreal_manager::pre_init: created customproperties_component." << std::endl;
#endif
    
    std::vector<async_wrapper*> async_initializers;
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(FName_FName, memory_manager::find_function_by_string(L"EnteringMap", { 0xE9 }, 1, false));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(InitializeStruct, memory_manager::find_pattern("48 89 54 24 ? 53 55 56 57 41 54 41 55 41 56 48 83 EC 20 44 8B 49 54 48 8B DA 8B 71 50 4C 8B F1 FF CE 45 8B E0 41 03 F1 33 D2 41 F7 D9 48 8B CB 41 23 F1 8B C6"));
        if (!InitializeStruct)
        {
            INIT_POINTER(InitializeStruct, memory_manager::find_pattern("48 89 54 24 ? 53 55 56 57 41 54 41 55 41 56 48 83 EC 20 ? ? 49 44 48 8B DA 8B 71 40 4C 8B F1 FF CE 45 8B E0 41 03 F1 33 D2"));
        }
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(StaticLoadObject, memory_manager::find_pattern("E8 ? ? ? ? 48 8B 4C 24 ? 48 8B F0 48 85 C9 74 05 E8 ? ? ? ? 48 8B 7C 24 ? 48 8B C6 48 8B 74 24 ?", true, 1));
    }));
    async_wrapper::wait_for_wrappers(async_initializers);

#ifdef ERA_LL_LOGS
    debug_log << L"unreal_manager::pre_init: pre-init completed." << std::endl;
#endif
    
    gameserver_client::PreInitialize();
}

void unreal_manager::status()
{
    // Scan & Log
    for (const auto& [key, val] : UninitializedPointers)
    {
        if (key == nullptr)
        {
            spdlog::warn("Pointer {} has failed initialization.", val.c_str());
        }
    }
}

unreal_manager::unreal_manager()
{
    std::vector<async_wrapper*> async_initializers;

    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(FMemory_Realloc_, memory_manager::find_pattern("E8 ? ? ? ? 48 89 03 48 8B 5C 24 ? 48 83 C4 20", true, 1));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(FindFunctionByName, memory_manager::find_function_by_string(L"CallFunctionByNameWithArguments: Name not found", { 0xE8 }, 1, false));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(FindPropertyByName, memory_manager::find_pattern("E8 ? ? ? ? 49 89 ? ? ? ? ? 49 8B ? ? ? ? ? 48 85 C9 74 33 48 85 DB 74 2E 48 8B 01 48 8B D3 ? 8B ? 30 FF 90 ? ? ? ? 49 8B ? ? ? ? ? 49 8B ? ? ? ? ? 48 8B ? 10", true, 1));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(StaticConstructObject, memory_manager::find_pattern("E8 ? ? ? ? 48 8B 8C 24 ? ? ? ? 48 8B D8 48 89 48 28 40 88 70 30 48 8B 4C 24", true, 1));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(StaticConstructObject, memory_manager::find_function_by_string(L"STAT_ConstructObject"));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(SpawnActor, memory_manager::find_pattern("40 53 56 57 48 83 EC 70 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 0F 28 1D ? ? ? ? 0F 57 D2 48 8B B4 24 ? ? ? ? 0F 28 CB"));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(GWorld, memory_manager::find_function_by_string(L"UEngine::TickDeferredCommands took %.02", {0x48, 0x8b, 0x1d}, 3));
        INIT_POINTER(GWorld, memory_manager::find_pattern("48 8B 1D ? ? ? ? 48 85 DB 74 3B 41",true, 3));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(GEngine, memory_manager::find_function_by_string(L"Failed to determine if OSS is server in PIE, OSS re", { 0x48, 0x8b, 0x1d }, 3));
        INIT_POINTER(GEngine, memory_manager::find_function_by_string(L"STAT_FEngineLoop_UpdateTimeAndHandleMax", { 0x48 }, 3, false));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(ProcessEvent, memory_manager::find_pattern("40 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 8D 6C 24 ? 48 89 9D ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C5 48 89 85 B0 ? ? ?"));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(FastArraySerializer_MarkArrayDirty, memory_manager::find_function_by_string(L"STAT_FortInfluenceMap", { 0xE8 }, 1, false));
        INIT_POINTER(FastArraySerializer_MarkArrayDirty, memory_manager::find_pattern("E8 ? ? ? ? 8B 45 48 85 C0 7E 3F 4C 8D 55 38 45 33 C9 ", true, 1));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(GenericArray_Remove, memory_manager::find_pattern("E8 ? ? ? ? 4C 8B C6 48 8B D7 48 8B CB E8 ? ? ? ? 83 F8 FF", true, 1) ?
           memory_manager::find_pattern("E8 ? ? ? ? 4C 8B C6 48 8B D7 48 8B CB E8 ? ? ? ? 83 F8 FF", true, 1) :
           memory_manager::find_pattern("E8 ? ? ? ? 4D 8B 75 78 33 ED 8B 47 08 BB ? ? ? ? 45 8B 7E 3C", true, 1) ?
           memory_manager::find_pattern("E8 ? ? ? ? 4D 8B 75 78 33 ED 8B 47 08 BB ? ? ? ? 45 8B 7E 3C", true, 1) :
           memory_manager::find_pattern("48 85 C9 0F 84 ? ? ? ? 56 48 83 EC 70 4C 8B 4A 78 41 8B F0 8B 82 ? ? ? ? 45 8B 41 3C 89 44 24 64"));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(Step, memory_manager::find_pattern("E8 ? ? ? ? EB ? 4C 8B 82 ? ? ? ? 49 8B 40", true, 1));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(Get, memory_manager::find_pattern("E8 ? ? ? ? 0F 10 73 10 48 8D 53 20 48 8B F8 48 8D 4C 24 ? E8 ? ? ? ?", true, 1));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(SetFWeakObjectPtr, memory_manager::find_function_by_string(L"Failed to find PlayerStart", {0xe8}, 1));
        INIT_POINTER(SetFWeakObjectPtr, memory_manager::find_pattern("40 53 48 83 EC 20 48 8B D9 48 85 D2 74 1A 8B 52 0C 89 11"));
        INIT_POINTER(SetFWeakObjectPtr, memory_manager::find_pattern("48 89 74 24 ? 57 48 83 EC 30 48 8B F1 48 85 D2 0F 84 ? ? ? ?"));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(GNatives, memory_manager::find_pattern("48 8D 05 ? ? ? ? B9 ? ? ? ? 0F 1F 40 00 66 0F 1F 84 00 ? ? ? ?", true, 3));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(SRand, memory_manager::find_pattern("69 0D ? ? ? ? ? ? ? ? F3 0F 10 05 ? ? ? ? F3 0F 11 44 24 ? 81 C1 ? ? ? ? 89 0D ? ? ? ? 33 4C 24 08 81 E1 ? ? ? ? 31 4C 24 08 F3 0F 10 44 24 ?"));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(TryLoadMips, memory_manager::find_pattern("40 53 55 56 57 41 57 48 83 EC 30 4C 89 64 24"));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(TryLoadMips, memory_manager::find_pattern("40 53 55 56 57 41 57 48 83 EC 20 4C 89 64 24 ? 4D 8B F8"));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(TryLoadMips, memory_manager::find_pattern("8B D6 4D 03 C6 48 8B CD E8 ? ? ? ? 84 C0", true, 9));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(TryLoadMips, memory_manager::find_function_by_string(L"Loading non-streamed mips from an external bulk file.  This is not desireable.  File %s"));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(TryLoadMips, memory_manager::find_function_by_string(L"TryLoadMips failed for %s, NumMipsCached: %d, LoadableMips: %d, FirstMipToLoad: %d"));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(Parse__CommandInternal, memory_manager::find_function_by_string(TEXT("DEMOSPEED"), {0xe8}, 1, false));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(Parse__TokenInternal, memory_manager::find_function_by_string(TEXT("You must specify a speed in the form of a float"), {0xe8}, 1));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(GIsClient, memory_manager::find_pattern("48 83 EC 28 48 8B 01 FF 90 ? ? ? ? 84 C0 74 12 33 C0 38 05 ? ? ? ? 0F 95 C0 FF C0 48 83 C4 28", true, 21));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(FCollisionQueryParams__Ctor, memory_manager::find_pattern("E8 ? ? ? ? 48 8D 44 24 ? 4C 8B CE 48 89 44 24 ? 4C 8B C7 48 8B 83", true, 1));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(LineTraceMultiByChannel, memory_manager::find_pattern("48 83 EC 48 48 8D 05 ? ? ? ? 48 89 44 24 ? 48 8B 84 24 ? ? ? ? 48 89 44 24 ? 48 8B 44 24 ? 48 89 44 24 ? 8B 44 24 70 89 44 24 20 E8 ? ? ? ? 48 83 C4 48 C3"));
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(FCollisionQueryParams__AddIgnoredActors, memory_manager::find_pattern("40 53 41 55 41 56 48 83 EC 20 33 DB 4C 8B F2 4C 8B E9 39 5A 08 7E 7A 48 89 74 24 ? 33 F6"));
        if (!FCollisionQueryParams__AddIgnoredActors)
        {
            INIT_POINTER(FCollisionQueryParams__AddIgnoredActors, memory_manager::find_pattern("40 57 41 55 41 56 48 83 EC 20 33 FF 4C 8B F2 4C 8B E9 39 7A 08 0F 8E ? ? ? ? 48 89 74 24 ? 8B F7 48 89 5C 24"));
        }
    }));
    async_initializers.push_back(new async_wrapper([&]
    {
        INIT_POINTER(UActorComponent__RegisterComponentInWorld, memory_manager::find_function_by_string(L"RegisterComponentWithWorld: (%s) Trying to register component with IsPendingKill() == true. Aborting."));
    }));
    async_wrapper::wait_for_wrappers(async_initializers);
    
    INIT_POINTER(GIsServer, reinterpret_cast<uintptr_t>(GIsClient) + 1);

    while (!FindObject(L"/Script/CoreUObject.Default__PackageMap"))
    {
        Sleep(1000 / 60);
    }
    INIT_POINTER(CreateDefaultObject_Internal, *(void**)(reinterpret_cast<uintptr_t>(FindObject(L"/Script/CoreUObject.Default__PackageMap")->VTable) - 8));
    
    if (const uintptr_t LastHitchSig = memory_manager::find_pattern("8B 88 ? ? ? ? 89 8B ? ? ? ? F2 0F 11 B3 ? ? ? ? 48 8B 5C 24 ? 0F 28 74 24 ? 48 83 C4 30"))
    {
        CoreOffsets::LastHitch = *reinterpret_cast<DWORD*>(LastHitchSig + 8);
    }

    // For now, set this offset on CL only
    if (GetCL() == 4363240)
    {
        NetworkOffsets::ActorReplicator = 0x90;
        NetworkOffsets::ObjectPtr = 0x10;
    }
    else
    {
        NetworkOffsets::ActorReplicator = 0x0;
        NetworkOffsets::ObjectPtr = 0x0;
    }
    
    // Instantly set GIsClient to false
    if (GetEngineVersionEasy() >= 421 && GetFortniteVersionEasy() < 10)
    {
        *GIsClient = false;
    }
    
    // Components pre-init
    net_component::pre_init();
    replication_component::pre_init();
    
    spdlog::info(L"Server successfully initialized. Module FortniteClient-Win64-Shipping.exe base address is {}",
        reinterpret_cast<void*>(GetModuleHandle(nullptr)));
    spdlog::info(L"Running version: {}", GetEngineVersion().c_str());
}

bool FString::operator==(FString in_str)
{
    return UKismetStringLibrary::Equals(*this, in_str);
}

int UFortItemDefinition::GetMaxStackSize()
{
    auto MaxStackSizeProperty = FindPropertyByName(this->Class, L"MaxStackSize");

    // INT
    if (*reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(MaxStackSizeProperty) + CoreOffsets::ElementSize) == 4)
    {
        return MaxStackSize();
    }
    else // ScalableFloat
    {
        return reinterpret_cast<FScalableFloat*>(&MaxStackSize())->GetValueAtLevel(0, nullptr);
    }   
}

UFortPlaylistAthena* GetPlaylistEasy()
{
    STATIC_HASPROPERTY((*GWorld)->GameState(), CurrentPlaylistData);
    STATIC_HASPROPERTY((*GWorld)->GameState(), CurrentPlaylistInfo);
    
    UFortPlaylistAthena* current_playlist = bExists_CurrentPlaylistData ?
            (*GWorld)->GameState()->CurrentPlaylistData() :
            bExists_CurrentPlaylistInfo ?
            (*GWorld)->GameState()->CurrentPlaylistInfo().BasePlaylist() :
            nullptr;

    return current_playlist;
}

void TArray<FGameplayTag>::Free()
{
    Count = 0;
    Max = 0;
    FMemory_Free(Data);
    Data = nullptr;
}

std::wstring TArray<FGameplayTag>::ToString()
{
    std::wstring Result;

    for (FGameplayTag Tag : *this)
    {
        Result.append(Tag.TagName.c_str()).append(L" ");
    }

    return Result;
}

void TArray<FMcpVariantChannelInfo>::Free()
{
    Count = 0;
    Max = 0;
    FMemory_Free(Data);
    Data = nullptr;
}

void TArray<UAthenaDanceItemDefinition*>::Free()
{
    Count = 0;
    Max = 0;
    FMemory_Free(Data);
    Data = nullptr;
}

void TArray<UAthenaItemWrapDefinition*>::Free()
{
    Count = 0;
    Max = 0;
    FMemory_Free(Data);
    Data = nullptr;
}

FCollisionQueryParams::FCollisionQueryParams(FName TagName, bool bTraceComplex, AActor* ActorToIgnore)
{
    constexpr FName EmptyStatId = FName();
    FCollisionQueryParams__Ctor(this, TagName, EmptyStatId, bTraceComplex, ActorToIgnore);
    bReturnPhysicalMaterial = true;
    TArray<AActor*> ActorsToIgnore = ActorToIgnore->GetAttachedActors();
    AddIgnoredActors(ActorsToIgnore);
}

FVector AActor::K2_GetActorLocation()
{
    return RootComponent() != nullptr ? RootComponent()->ComponentToWorld().Translation : FVector(0, 0, 0);
}