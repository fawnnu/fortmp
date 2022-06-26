#include "include.hpp"

#include "components/ability_component.hpp"
#include "components/aircraft_component.hpp"
#include "components/building_component.hpp"
#include "components/inventory_component.hpp"
#include "components/loader_component.hpp"
#include "components/looting_component.hpp"
#include "components/net_component.hpp"
#include "components/patches_component.hpp"
#include "components/pickup_component.hpp"
#include "components/player_component.hpp"

#include "gui/renderer.hpp"

void Setup()
{
    AllocConsole();
    
    // setup Unreal Engine
    unreal_manager::setup_unreal_engine();

    {
        auto renderer_thread = [](LPVOID) -> DWORD
        {
            // Render on a new thread as to not block everything
            new renderer();

            return 0;
        };

        // TODO: Check if renderer has been enabled through the command line before doing anything
        CreateThread(nullptr, 0, renderer_thread, nullptr, 0, nullptr);
    }

    {
        while (true)
        {
            if (*GWorld)
            {
                // TODO: maybe move? 
                APlayerController* player_controller = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics")->GetPlayerController(*GWorld, 0);
                
                if (player_controller)
                {
                    // load into level
                    player_controller->SwitchLevel(FString(L"Athena_Terrain"));

                    while ((*GWorld)->GetName().find(L"Frontend") != -1)
                    {
                        // block DLL thread until level starts to load
                    }

                    new net_component();
                    new loader_component();
                    new patches_component();
                    new player_component();
                    new aircraft_component();
                    new ability_component();
                    new inventory_component();
                    new pickup_component();
                    new looting_component();
                    
                    break;
                }
            }
            
            Sleep(500);
        }
    } 
}

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD   ul_reason_for_call,
                      LPVOID  lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        CreateThread(0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(&Setup), 0, 0, 0);
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {

    }

    return TRUE;
}