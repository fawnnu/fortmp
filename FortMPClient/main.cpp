#include "include.hpp"

#include "components/anticheat_component.hpp"
#include "components/ssl_bypass_component.hpp"
#include "components/patches_component.hpp"
#include "components/hwid_component.hpp"
#include "components/log_component.hpp"
#include "components/voice_component.hpp"

void Setup()
{
    // Fake pak to bypass check
    unreal_manager::setup_unreal_engine();
#ifdef FMPLOG
    new log_component();
#endif

    ssl_bypass_component::redirect_url = XOR("https://api.v1.external.erafn.org/api/external/fortnite");

    //Instantly start the SSL bypass
    new ssl_bypass_component();
    new hwid_component();
    //new anticheat_component(); Temporarily disable, throws false positives

    {
        while (true)
        {
            if (*GWorld)
            {
                UObject* player_controller = FindObject<UGameplayStatics>(XOR(L"/Script/Engine.Default__GameplayStatics"))->GetPlayerController(*GWorld, 0);
                
                if (player_controller)
                {
                    //Create and set console
                    (*GEngine)->GameViewport()->ViewportConsole() = static_cast<UConsole*>(FindObject<UGameplayStatics>(XOR(L"/Script/Engine.Default__GameplayStatics"))->SpawnObject(FindObject(XOR(L"/Script/Engine.Console")), (*GEngine)->GameViewport()));

                    new patches_component();
                    
                    break;
                }
            }
            
            Sleep(500);
        }
    }

    while (true)
    {
        if (*GWorld)
        {
            UObject* player_controller = FindObject<UGameplayStatics>(XOR(L"/Script/Engine.Default__GameplayStatics"))->GetPlayerController(*GWorld, 0);
                
            if (player_controller)
            {
                if (static_cast<APlayerController*>(player_controller)->CheatManager() == nullptr)
                {
                    static_cast<APlayerController*>(player_controller)->CheatManager() = FindObject<UGameplayStatics>(XOR(L"/Script/Engine.Default__GameplayStatics"))->SpawnObject(FindObject(XOR(L"/Script/Engine.CheatManager")), player_controller);
                }
            }
        }
            
        Sleep(15000);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD   ul_reason_for_call,
                      LPVOID  lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        CreateThread(0,0, reinterpret_cast<LPTHREAD_START_ROUTINE>(&Setup), 0,0,0);
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {

    }

    return TRUE;
}

