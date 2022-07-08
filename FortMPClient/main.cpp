#include "include.hpp"

#include "components/ssl_bypass_component.hpp"

void Setup()
{
    unreal_manager::setup_unreal_engine();

    ssl_bypass_component::redirect_url = "http://127.0.0.1:3551";
    
    //Instantly start the SSL bypass
    new ssl_bypass_component();

    {
        while (true)
        {
            if (*GWorld)
            {
                // TODO: maybe move? 
                UObject* player_controller = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics")->GetPlayerController(*GWorld, 0);
                
                if (player_controller)
                {
                    //Create and set console
                    (*GEngine)->GameViewport()->ViewportConsole() = static_cast<UConsole*>(FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics")->SpawnObject(FindObject(L"/Script/Engine.Console"), (*GEngine)->GameViewport()));
                    
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
        CreateThread(0,0, reinterpret_cast<LPTHREAD_START_ROUTINE>(&Setup), 0,0,0);
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {

    }

    return TRUE;
}

