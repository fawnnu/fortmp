#include "include.hpp"

#include "components/ability_component.hpp"
#include "components/aircraft_component.hpp"
#include "components/building_component.hpp"
#include "components/creative_component.hpp"
#include "components/elimination_component.hpp"
#include "components/harvesting_component.hpp"
#include "components/interact_component.hpp"
#include "components/inventory_component.hpp"
#include "components/loader_component.hpp"
#include "components/looting_component.hpp"
#include "components/net_component.hpp"
#include "components/patches_component.hpp"
#include "components/pickup_component.hpp"
#include "components/player_component.hpp"
#include "components/reboot_component.hpp"
#include "components/replication_component.hpp"
#include "components/team_component.hpp"
#include "components/trap_component.hpp"
#include "components/vehicles_component.hpp"
#include "components/storm_component.hpp"
#include "components/function_component.hpp"
#include "components/gamemodes_component.hpp"
#include "components/modules_component.hpp"

#include "gui/renderer.hpp"

void Setup()
{
    // Initialize utils
    new dll_utils();
    
    // Create a file sink
    dll_utils::create_spdlog_sink();

    // Bypass anti tampering TODO: Check commit name
    //new anti_tampering_bypass();
    
    // Setup Unreal Engine
    unreal_manager::setup_unreal_engine();

    // Disable unwanted modules
    //new modules_component();

    // Setup rendered on another thread
    CreateThread(nullptr, 0, [](LPVOID) -> DWORD
    {
        new renderer();
            
        return 0;    
    }, nullptr, 0, nullptr);

    {
        while (true)
        {
            if (*GWorld)
            {
                APlayerController* player_controller = UGameplayStatics::GetPlayerController(*GWorld, 0);
                
                if (player_controller)
                {
                    FString server_level;

                    if (GetEngineVersionEasy() > 423)
                    {
                        server_level = L"Apollo_Terrain";
                    }
                    else
                    {
                        server_level = L"Athena_Terrain";
                    }
                    
                    // load into level
                    player_controller->SwitchLevel(server_level);

                    while ((*GWorld)->GetName().find(L"Frontend") != std::string::npos)
                    {
                        // block DLL thread until level starts to load
                    }

                    // Get the LocalPlayers array
                    TArrayManager<ULocalPlayer*> LocalPlayers = TArrayManager<ULocalPlayer*>(&(*GWorld)->OwningGameInstance()->LocalPlayers(), 8);

                    // Remove the server's player
                    LocalPlayers.Remove(0, FindPropertyByName((*GWorld)->OwningGameInstance()->Class, L"LocalPlayers"));

                    new function_component();
                    new hook_manager();
                    new net_component();
                    new loader_component();
                    new patches_component();
                    new player_component();
                    new aircraft_component();
                    new ability_component();
                    new inventory_component();
                    new pickup_component();
                    new looting_component();
                    new building_component();
                    new elimination_component();
                    new trap_component();
                    new reboot_component();
                    new interact_component();
                    new team_component();
                    new harvesting_component();
                    new vehicles_component();
                    new storm_component();
                    new replication_component();
                    new creative_component();
                    new gamemodes_component();
                    
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