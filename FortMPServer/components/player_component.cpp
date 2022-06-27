#include "include.hpp"
#include "player_component.hpp"

#include "ability_component.hpp"
#include "inventory_component.hpp"
#include "net_component.hpp"

player_component::player_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(player_component_hooks::SetClientLoginState, memory_manager::find_pattern("48 89 5C 24 ? 57 48 83 EC 40 8B DA 48 8B F9 39 91 ? ? ? ? 0F 85 ? ? ? ? 80 3D ? ? ? ? ? 0F 82 ? ? ? ? 85 D2 74 41 83 EB 01 74 33 83 EB 01"), player_component_hooks::SetClientLoginState_Hook);
    REGISTER_FUNCTION_DETOUR(player_component_hooks::SetClientLoginState, memory_manager::find_pattern("48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 40 8B A9 ? ? ? ? 8B F2 48 8B F9 3B EA 75 55 80 3D ? ? ? ? ? 0F 82 ? ? ? ? 48 8B 05 ? ? ? ? 8B CA 48 89 44 24 ?"), player_component_hooks::SetClientLoginState_Hook);
    REGISTER_FUNCTION_DETOUR(player_component_hooks::SetClientLoginState, memory_manager::find_function_by_string(L"UNetConnection::SetClientLoginState: State same: %s"), player_component_hooks::SetClientLoginState_Hook);

    REGISTER_FUNCTION_DETOUR(player_component_hooks::GetHeroDefinition, memory_manager::find_function_by_string(L"Trying to apply homebase effects on player setup but hero or herotype is not set.", { 0xE8 }, 1), player_component_hooks::GetHeroDefinition_Hook);
    
    DetourTransactionCommit();
}

AFortPlayerPawn* player_component::create_player(AFortPlayerController* player_controller, FTransform spawn_transform)
{
    AFortPlayerPawn* player_pawn = (*GWorld)->AuthorityGameMode()->SpawnDefaultPawnAtTransform(player_controller, spawn_transform);

    if (player_pawn)
    {
        player_controller->Possess(player_pawn);
        
        UObject* ability_set = FindObject(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer") ?
            FindObject(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer") : FindObject(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_DefaultPlayer.GAS_DefaultPlayer");
        
        // Apply Athena ability set
        ability_component::apply_ability_set(player_pawn, ability_set);
        
        return player_pawn;
    }

    return nullptr;
}

void player_component_hooks::SetClientLoginState_Hook(UNetConnection* NetConnection, int State)
{
    if (State == 3) // ReceivedJoin
    {
        AFortPlayerPawn* player_pawn = player_component::create_player(NetConnection->PlayerController(), NetConnection->PlayerController()->GetTransform());

        if (player_pawn)
        {
            // Spawn QuickBars
            NetConnection->PlayerController()->QuickBars() = SpawnDefaultActor<AActor>(*GWorld, FindObject(L"/Script/FortniteGame.FortQuickBars"));

            if (NetConnection->PlayerController()->QuickBars())
            {
                // Update QuickBars ownership
                NetConnection->PlayerController()->QuickBars()->SetOwner(NetConnection->PlayerController());
            }

            // Grant harvesting tool
            inventory_component::add_item(NetConnection->PlayerController(), FindObject<UFortItemDefinition>(L"WID_Harvest_Pickaxe_Athena_C_T01", false, true), 1);

            // Drop loading screen
            NetConnection->PlayerController()->bHasServerFinishedLoading() = true;
        }
    }

    return SetClientLoginState(NetConnection, State);
}

UObject* player_component_hooks::GetHeroDefinition_Hook(UObject* Hero)
{
    static UObject* hero_definition = FindObject(L"/Game/Athena/Heroes/HID_001_Athena_Commando_F.HID_001_Athena_Commando_F", true);

    if (hero_definition == nullptr)
    {
        // Failed to find hero definition
        spdlog::error(L"Failed to override hero definition for hero '{}', as specified hero definition does not exist",
            Hero->GetName().c_str());
    }

    return hero_definition;
}

