#include "include.hpp"
#include "engine_manager.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

engine_manager::engine_manager()
{
    global_engine_manager = this;

    // allocates a console
    AllocConsole();

    // Tweak spdlog options
    dynamic_cast<spdlog::sinks::stdout_color_sink_mt*>(spdlog::default_logger()->sinks().back().get())->set_color(spdlog::level::level_enum::info, 0xFFFF);
    spdlog::default_logger()->set_level(spdlog::level::level_enum::debug);
    spdlog::default_logger()->flush_on(spdlog::level::debug);
    spdlog::default_logger()->set_pattern("%^[%H:%M:%S] %v %$");

    // init and check ptrs
    INIT_PTR(fname_to_string, memory_manager::find_pattern("E8 ? ? ? ? F3 41 0F 10 06 48 8D 15 ? ? ? ? F3 0F 59 05 ? ? ? ? 48 8B CF", true, 1))
    INIT_PTR(free_internal, memory_manager::find_pattern("E8 ? ? ? ? 48 8B 0F 48 89 0B 4C 89 37 8B 47 08 89 43 08 8B 47 0C 89 43 0C 4C 89 77 08", true, 1))
    INIT_PTR(static_find_object, memory_manager::find_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 30 80 3D ? ? ? ? ? 41 0F B6 D9 49 8B F8 48 8B F2"))
    INIT_PTR(static_load_object, memory_manager::find_pattern("E8 ? ? ? ? 48 8B 4C 24 ? 48 8B F0 48 85 C9 74 05 E8 ? ? ? ? 48 8B 7C 24 ? 48 8B C6 48 8B 74 24 ?", true, 1))
    INIT_PTR(get_engine_version, memory_manager::find_pattern("40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B C8 41 B8 04 ? ? ? 48 8B D3"))
    INIT_PTR(find_function_by_name, memory_manager::find_function_by_string(L"CallFunctionByNameWithArguments: Name not found", { 0xE8 }, 1,false))
    INIT_PTR(process_event, static_find_object(nullptr, nullptr, L"/Script/CoreUObject", false) ->VTableObject[64])
    INIT_PTR(fname_fname, memory_manager::find_function_by_string(L"EnteringMap", {0xE9}, 1, false))
    INIT_PTR(GWorld, memory_manager::find_function_by_string(L"UEngine::TickDeferredCommands took %.02", {0x48, 0x8b, 0x1d}, 3))
    INIT_PTR(GEngine, memory_manager::find_function_by_string(L"STAT_FEngineLoop_UpdateTimeAndHandleMax", {0x48}, 3, false))
    
    check(fname_to_string);
    check(free_internal);
    check(static_find_object);
    check(static_load_object);
    check(get_engine_version);
    check(find_function_by_name);
    check(GWorld);
    check(GEngine);
    check(process_event);
    check(fname_fname)

    spdlog::info(L"FortMP was initialized successfully.");

    wait_for_server();
}

void engine_manager::wait_for_server()
{
    while (TRUE)
    {
        if (*GWorld)
        {
            UGameplayStatics* gameplay_statics = static_cast<UGameplayStatics*>(static_find_object(nullptr, nullptr, L"/Script/Engine.Default__GameplayStatics", false));

            APlayerController* server_controller = gameplay_statics->call_function<APlayerController*>(L"GetPlayerController", *GWorld, 0);

            if (server_controller)
            {
                server_controller->call_function(L"SwitchLevel", FString(L"Athena_Terrain"));

                while ( (*GWorld)->get_name().find(L"Frontend") != -1 )
                {
                }

                // start server here
                setup_components_and_hooks();
                
                break;
            }
        }

        Sleep(500);
    }
}

void engine_manager::setup_components_and_hooks()
{
    //construct a new hook manager 
    new hook_manager();

    //construct a new component loader
    new component_loader();

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    for (auto hook : global_hook_manager->hooks)
    {
        if (DetourAttach(hook->function_pointer, hook->function_detour) == 0)
        {
            spdlog::info("Hooked {}", hook->function_name.c_str());
        }
        else
        {
            spdlog::error("Failed to hook {}", hook->function_name.c_str());
        }
    }
    
    DetourTransactionCommit();
}


