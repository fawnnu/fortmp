#include "include.hpp"
#include "server_component.hpp"

DEFINE_HOOK(void, ReadyToStartMatch, (AGameMode* GameMode, struct FFrame& StackFrame))
{
    spdlog::info(L"readytostartmatch");
    
    return ReadyToStartMatch(GameMode, StackFrame);
}

server_component::server_component()
{
    component_name = "server_component";

    INIT_PTR(ServerReplicateActors, memory_manager::find_function_by_string(L"No Replication Lists were returned for connection"));
    INIT_PTR(CreateNamedNetDriver, memory_manager::find_function_by_string(L"PlayReplay: failed to create demo net d", { 0xE8 }, 1));
    INIT_PTR(FindNamedNetDriver, memory_manager::find_function_by_string(L"PlayReplay: failed to create demo net d",{0xe8, 0x100, 0x100, 0x100 , 0x0}, 1, false));
    INIT_PTR(InitListen, memory_manager::find_function_by_string(L"Failed to init net driver ListenURL: %s: %s"));
    INIT_PTR(SetWorld, memory_manager::find_function_by_string(L"Demo playback failed: %s", { 0xE8 }, 1));

    check(ServerReplicateActors);
    check(CreateNamedNetDriver);
    check(FindNamedNetDriver);
    check(InitListen);
    check(SetWorld);

    // Set up hooks
    INIT_PTR(ReadyToStartMatch, static_cast<UFunction*>(global_engine_manager->static_find_object(nullptr, nullptr, L"/Script/Engine.GameMode.ReadyToStartMatch", false))->get_func());
    new hook("ReadyToStartMatch", &reinterpret_cast<LPVOID&>(ReadyToStartMatch), ReadyToStartMatch_Hook);
}


