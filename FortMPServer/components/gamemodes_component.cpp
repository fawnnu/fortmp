#include "include.hpp"
#include "gamemodes_component.hpp"

#include "function_component.hpp"
#include "inventory_component.hpp"
#include "loader_component.hpp"
#include "looting_component.hpp"
#include "../utils/webhook_manager.hpp"

gamemodes_component::gamemodes_component()
{
    // Register UFunctions
    REGISTER_CUSTOM_UFUNCTION(OnServerReady, AFortGameModeAthena::StaticClass());

    REGISTER_CUSTOM_UFUNCTION(OnPlayerJoined, AFortGameModeAthena::StaticClass());
    INIT_CUSTOM_FUNC_PARAMS(OnPlayerJoined, AFortGameModeAthena::StaticClass(), 0x8);

    REGISTER_CUSTOM_UFUNCTION(OnUpdateStorm, AFortGameModeAthena::StaticClass());
    INIT_CUSTOM_FUNC_RET_OFFSET(OnUpdateStorm, AFortGameModeAthena::StaticClass(), 0x0);
    
    REGISTER_CUSTOM_UFUNCTION(OnSetPlaylist, AFortGameModeAthena::StaticClass());
    INIT_CUSTOM_FUNC_RET_OFFSET(OnSetPlaylist, AFortGameModeAthena::StaticClass(), 0x0);
    
    REGISTER_CUSTOM_UFUNCTION(OnEnterAircraft, AFortGameModeAthena::StaticClass());
    INIT_CUSTOM_FUNC_PARAMS(OnEnterAircraft, AFortGameModeAthena::StaticClass(), 0x8);

    REGISTER_CUSTOM_UFUNCTION(SetSafeZoneStartDelay_Value, AFortGameModeAthena::StaticClass());
    INIT_CUSTOM_FUNC_PARAMS(SetSafeZoneStartDelay_Value, AFortGameModeAthena::StaticClass(), 0x4);
    
    REGISTER_CUSTOM_UFUNCTION(GiveItemToPlayer, AFortGameModeAthena::StaticClass());
    INIT_CUSTOM_FUNC_PARAMS(GiveItemToPlayer, AFortGameModeAthena::StaticClass(), 0x18);
    
    REGISTER_CUSTOM_UFUNCTION(OnPawnFinishedSpawning, AFortGameModeAthena::StaticClass());
    INIT_CUSTOM_FUNC_PARAMS(OnPawnFinishedSpawning, AFortGameModeAthena::StaticClass(), 0x8);

    REGISTER_CUSTOM_UFUNCTION(GetRandomLootPackageData, AFortGameModeAthena::StaticClass());
    INIT_CUSTOM_FUNC_PARAMS(GetRandomLootPackageData, AFortGameModeAthena::StaticClass(), 1024);
    INIT_CUSTOM_FUNC_RET_OFFSET(GetRandomLootPackageData, AFortGameModeAthena::StaticClass(), 0x10);
}

void gamemodes_component::OnServerReady(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

#ifdef _RELEASE
    // Notify era services
    gameserver_client::Client->GameServer_Status.accepting_players = true;
    gameserver_client::Client->GameServer_Status.max_players = loader_component::get_max_players();
    gameserver_client::Client->GameServer_Status.status = "online";
    gameserver_client::Client->SendUpdate();
#endif
}

void gamemodes_component::OnPlayerJoined(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

#ifdef _RELEASE
    // Notify era services
    gameserver_client::Client->GameServer_Status.number_of_players = FortGameModeAthena->AlivePlayers().Num();
    gameserver_client::Client->SendUpdate();
#endif
}

void gamemodes_component::OnUpdateStorm(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame, void* ReturnValue)
{
    INIT_CUSTOM_FUNCTION();

    // Unless a Blueprint implements this function, we will return false
    *reinterpret_cast<bool*>(ReturnValue) = false;
}

void gamemodes_component::OnSetPlaylist(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame, void* ReturnValue)
{
    INIT_CUSTOM_FUNCTION();

    // Unless a Blueprint implements this function, we will return false
    *reinterpret_cast<UObject**>(ReturnValue) = nullptr;
}

void gamemodes_component::OnEnterAircraft(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();   
}

void gamemodes_component::SetSafeZoneStartDelay_Value(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame)
{
    GET_CUSTOM_FUNCTION_BP_PARAM(float, InValue);
    
    INIT_CUSTOM_FUNCTION();

    (*GWorld)->GameState()->MapInfo()->SafeZoneStartDelay().Value() = InValue;
}

void gamemodes_component::GiveItemToPlayer(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame)
{
    GET_CUSTOM_FUNCTION_BP_PARAM(AFortPlayerController*, PlayerController);   
    GET_CUSTOM_FUNCTION_BP_PARAM(UFortItemDefinition*, InItemDefinition);   
    GET_CUSTOM_FUNCTION_BP_PARAM(int, Count);

    INIT_CUSTOM_FUNCTION();

    if (InItemDefinition)
    {
        inventory_component::add_item(PlayerController, InItemDefinition, Count, looting_component::get_default_loaded_ammo(InItemDefinition));
    }
}

void gamemodes_component::OnPawnFinishedSpawning(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();
}

void gamemodes_component::GetRandomLootPackageData(AFortGameModeAthena* FortGameModeAthena, FFrame& StackFrame, void* ReturnValue)
{
    GET_CUSTOM_FUNCTION_BP_PARAM(UDataTable*, DataTable);
    GET_CUSTOM_FUNCTION_BP_PARAM(FName, LootPackageID);
    
    INIT_CUSTOM_FUNCTION();

    memcpy(ReturnValue, looting_component::get_random_loot_package_data_from_id(DataTable, LootPackageID), FFortLootPackageData::GetStructSize());
}