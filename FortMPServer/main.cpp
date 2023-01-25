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
#include "components/blueprint_component.hpp"
#include "components/log_component.hpp"
#include "components/moderation_component.hpp"
#include "components/pak_component.hpp"
#include "components/modules_component.hpp"
#include "components/qos_component.hpp"

#include "gui/renderer.hpp"
#include "utils/webhook_manager.hpp"

void Setup()
{
    // Setup logging (File sinks)
    new log_component();

    // Setup webhooks (config file)
    new webhook_manager();
    
    // Setup Unreal Engine
    new unreal_manager();

    // Initialize function's component
    new function_component();

    // Load Server Paks
    new pak_component();
    
    // Setup renderer on another thread
    CreateThread(nullptr, 0, [](LPVOID) -> DWORD
    {
        new renderer();
            
        return 0;    
    }, nullptr, 0, nullptr);
    
    // Initialize the QOS component
    CreateThread(nullptr, 0, [](LPVOID) -> DWORD
    {
        new qos_component();
        new gameserver_client();
            
        return 0;    
    }, nullptr, 0, nullptr);

    // Set the default map
    CreateDefaultObject<UGameMapsSettings>()->GameDefaultMap().AssetPathName = TEXT("/Game/Maps/FortniteEmptyDedicated.FortniteEmptyDedicated");

    {
        while (true)
        {
            if (*GWorld)
            {
                // Wait 3.5 Seconds to set GIsServer
                if (GetEngineVersionEasy() > 421)
                {
                    *GIsServer = true;
                }
                
                // Initialize FMP blueprint support
                new blueprint_component();

                // Initialize gamemode custom functions
                new gamemodes_component();
                    
                // load into level
                UGameplayStatics::OpenLevel(*GWorld, GetEngineVersionEasy() > 423 ? TEXT("Apollo_Terrain") : TEXT("Athena_Terrain"), true, TEXT("game=/Game/Athena/Server/Athena_GameMode.Athena_GameMode_C"));

                while ((*GWorld)->GetName().find(L"FortniteEmptyDedicated") != std::string::npos)
                {
                    // block DLL thread until level starts to load
                }

                // Get the LocalPlayers array
                TArrayManager<ULocalPlayer*> LocalPlayers = TArrayManager<ULocalPlayer*>(&(*GWorld)->OwningGameInstance()->LocalPlayers(), 8);

                // Remove the server's player
                LocalPlayers.Remove(0, FindPropertyByName((*GWorld)->OwningGameInstance()->Class, L"LocalPlayers"));
                    
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
                new moderation_component();
                    
                break;
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