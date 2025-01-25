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
#include "components/bots_component.hpp"
#include "components/log_component.hpp"
#include "components/moderation_component.hpp"
#include "components/pak_component.hpp"
#include "components/modules_component.hpp"
#include "components/qos_component.hpp"
#include "components/playlist_component.hpp"
#include "components/anticheat_component.hpp"
#include "components/challenges_component.hpp"
#include "components/config_manager.hpp"
#include "components/customproperties_component.hpp"
#include "components/director_client.hpp"
#include "components/era_mutators_component.hpp"
#include "components/fort_mutators_component.hpp"
#include "components/gui_manager.hpp"
#include "components/ssl_bypass_component.hpp"

#include "gui/renderer.hpp"
#include "utils/webhook_manager.hpp"

void Setup()
{
    // Pre init UE functions
    unreal_manager::pre_init();
    
    // Pre-start patches
    patches_component::pre_init();
    
    // Create director_client
    new director_client();

    // Wait for config to be ready
    director_client::wait_for_config();
    
    // Setup logging (File sinks)
    new log_component();

    // Log initialization status
    unreal_manager::status();
    
    // Setup Unreal Engine
    new unreal_manager();

    // Initialize function's component
    new function_component();

    // Load Server Paks
    new pak_component();
    
    // Setup renderer on another thread
    new gui_manager();
    
    new era_mutators_component();
    
#ifdef _RELEASE
    // Initialize the QOS component
    CreateThread(nullptr, 0, [](LPVOID) -> DWORD
    {
        gameserver_client* gs_client = new gameserver_client();
        new qos_component();
        gs_client->Initialize();
            
        return 0;    
    }, nullptr, 0, nullptr);
#endif

    if (GetFortniteVersionEasy() < 10)
    {
        // Set the default map
        CreateDefaultObject<UGameMapsSettings>()->GameDefaultMap().AssetPathName = TEXT("/Game/Maps/FortniteEmptyDedicated.FortniteEmptyDedicated");
    }

    {
        while (true)
        {
            if (*GWorld)
            {
                // Wait 3.5 Seconds to set GIsServer
                if (GetEngineVersionEasy() >= 421)
                {
                    *GIsServer = true;
                }

                // Revert patches
                patches_component::revert_pre_init();
                
                // Initialize FMP blueprint support
                new blueprint_component();

                // Initialize gamemode custom functions
                new gamemodes_component();
                    
                // load into level
                if (era_mutators_component::IsActive("domino_event"))
                {
                    UGameplayStatics::OpenLevel(*GWorld, TEXT("/game/fortmp/content_complex/domino/domino_level"), true, TEXT("game=/Game/fortmp/content_complex/domino/StellarBellum_Content/StellarBellum_GameMode.StellarBellum_GameMode_C"));
                }
                else
                {
                    UGameplayStatics::OpenLevel(*GWorld, GetEngineVersionEasy() > 423 ? TEXT("Apollo_Terrain") : TEXT("Athena_Terrain"), true, TEXT("game=/Game/Athena/Server/Athena_GameMode.Athena_GameMode_C"));
                }
                
                while ((*GWorld)->GetName().find(GetFortniteVersionEasy() >= 10 ? L"Frontend" : L"FortniteEmptyDedicated") != std::string::npos)
                {
                    // block DLL thread until level starts to load
                }

                // Get the LocalPlayers array
                TArrayManager<ULocalPlayer*> LocalPlayers = TArrayManager<ULocalPlayer*>(&(*GWorld)->OwningGameInstance()->LocalPlayers(), 8);

                // Remove the server's player
                LocalPlayers.Remove(0, FindPropertyByName((*GWorld)->OwningGameInstance()->Class, L"LocalPlayers"));

                loader_component::pre_init();
                new hook_manager();
                new net_component();
                new loader_component();
                new fort_mutators_component();
                new patches_component();
                new player_component();
                new aircraft_component();
                new ability_component();
                new inventory_component();
                new pickup_component();
                new looting_component();
                new building_component();
                new bots_component();
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
                new playlist_component();
                new anticheat_component();
                new challenges_component();
                era_mutators_component::InitializeHooks();
                    
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