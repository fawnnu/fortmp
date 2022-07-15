#include "include.hpp"
#include "unreal.hpp"

void unreal_manager::setup_unreal_engine()
{
    INIT_POINTER(FName_ToString, memory_manager::find_pattern("E8 ? ? ? ? F3 41 0F 10 06 48 8D 15 ? ? ? ? F3 0F 59 05 ? ? ? ? 48 8B CF", true, 1));
    INIT_POINTER(FName_FName, memory_manager::find_function_by_string(L"EnteringMap", { 0xE9 }, 1, false));
    INIT_POINTER(FMemory_Free, memory_manager::find_pattern("E8 ? ? ? ? 48 8B 0F 48 89 0B 4C 89 37 8B 47 08 89 43 08 8B 47 0C 89 43 0C 4C 89 77 08", true, 1));

    INIT_POINTER(FindFunctionByName, memory_manager::find_function_by_string(L"CallFunctionByNameWithArguments: Name not found", { 0xE8 }, 1, false));
    INIT_POINTER(FindPropertyByName, memory_manager::find_pattern("E8 ? ? ? ? 49 89 ? ? ? ? ? 49 8B ? ? ? ? ? 48 85 C9 74 33 48 85 DB 74 2E 48 8B 01 48 8B D3 ? 8B ? 30 FF 90 ? ? ? ? 49 8B ? ? ? ? ? 49 8B ? ? ? ? ? 48 8B ? 10", true, 1));
    
    INIT_POINTER(StaticFindObject, memory_manager::find_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 30 80 3D ? ? ? ? ? 41 0F B6 D9 49 8B F8 48 8B F2"));
    INIT_POINTER(StaticLoadObject, memory_manager::find_pattern("E8 ? ? ? ? 48 8B 4C 24 ? 48 8B F0 48 85 C9 74 05 E8 ? ? ? ? 48 8B 7C 24 ? 48 8B C6 48 8B 74 24 ?", true, 1));

    INIT_POINTER(SpawnActor, memory_manager::find_pattern("40 53 56 57 48 83 EC 70 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 0F 28 1D ? ? ? ? 0F 57 D2 48 8B B4 24 ? ? ? ? 0F 28 CB"));
    
    INIT_POINTER(GWorld, memory_manager::find_function_by_string(L"UEngine::TickDeferredCommands took %.02", {0x48, 0x8b, 0x1d}, 3));
    INIT_POINTER(GWorld, memory_manager::find_pattern("48 8B 1D ? ? ? ? 48 85 DB 74 3B 41",true, 3));
    INIT_POINTER(GEngine, memory_manager::find_function_by_string(L"Failed to determine if OSS is server in PIE, OSS re", { 0x48, 0x8b, 0x1d }, 3));
    INIT_POINTER(GEngine, memory_manager::find_function_by_string(L"STAT_FEngineLoop_UpdateTimeAndHandleMax", { 0x48 }, 3, false));

    INIT_POINTER(ProcessEvent, memory_manager::find_pattern("40 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 8D 6C 24 ? 48 89 9D ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C5 48 89 85 B0 ? ? ?"));

    INIT_POINTER(FMemory_Realloc, memory_manager::find_pattern("E8 ? ? ? ? 48 89 03 48 8B 5C 24 ? 48 83 C4 20", true, 1));
    INIT_POINTER(FastArraySerializer_MarkArrayDirty, memory_manager::find_function_by_string(L"STAT_FortInfluenceMap", { 0xE8 }, 1, false));
    INIT_POINTER(FastArraySerializer_MarkArrayDirty, memory_manager::find_pattern("E8 ? ? ? ? 8B 45 48 85 C0 7E 3F 4C 8D 55 38 45 33 C9 ", true, 1));
    
    INIT_POINTER(GenericArray_Remove, memory_manager::find_pattern("E8 ? ? ? ? 4C 8B C6 48 8B D7 48 8B CB E8 ? ? ? ? 83 F8 FF", true, 1) ?
        memory_manager::find_pattern("E8 ? ? ? ? 4C 8B C6 48 8B D7 48 8B CB E8 ? ? ? ? 83 F8 FF", true, 1) :
        memory_manager::find_pattern("E8 ? ? ? ? 4D 8B 75 78 33 ED 8B 47 08 BB ? ? ? ? 45 8B 7E 3C", true, 1) ?
        memory_manager::find_pattern("E8 ? ? ? ? 4D 8B 75 78 33 ED 8B 47 08 BB ? ? ? ? 45 8B 7E 3C", true, 1) :
        memory_manager::find_pattern("48 85 C9 0F 84 ? ? ? ? 56 48 83 EC 70 4C 8B 4A 78 41 8B F0 8B 82 ? ? ? ? 45 8B 41 3C 89 44 24 64"));
    
    INIT_POINTER(GetEngineVersion, memory_manager::find_pattern("40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B C8 41 B8 04 ? ? ? 48 8B D3"));

    INIT_POINTER(Step, memory_manager::find_pattern("E8 ? ? ? ? EB ? 4C 8B 82 ? ? ? ? 49 8B 40", true, 1));

    INIT_POINTER(Get, memory_manager::find_pattern("E8 ? ? ? ? 0F 10 73 10 48 8D 53 20 48 8B F8 48 8D 4C 24 ? E8 ? ? ? ?", true, 1));

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
    }
}

bool FString::operator==(FString in_str)
{
    return UKismetStringLibrary::Equals(*this, in_str);
}
