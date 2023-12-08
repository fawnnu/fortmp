#include "include.hpp"
#include "player_component.hpp"

#include "ability_component.hpp"
#include "anticheat_component.hpp"
#include "challenges_component.hpp"
#include "config_manager.hpp"
#include "creative_component.hpp"
#include "director_client.hpp"
#include "era_mutators_component.hpp"
#include "function_component.hpp"
#include "inventory_component.hpp"
#include "loader_component.hpp"
#include "log_component.hpp"
#include "moderation_component.hpp"
#include "net_component.hpp"
#include "replication_component.hpp"
#include "team_component.hpp"

player_component::player_component()
{
    INIT_POINTER(HealthProperty, memory_manager::find_pattern("48 8B 15 ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? 48 8B CE E8 ? ? ? ? 48 85 C0 74 11 41 0F 28 D0 48 8D 54 24 ? 48 8B C8 E8 ? ? ? ? 48 8B 4C 24 ? 48 85 C9 74 05 E8 ? ? ? ? 48 8D 4E 30 E8 ? ? ? ? 44 0F 28 C0 48 8D 4E 50 F3 45 0F 5C C1 F3 44 0F 5F C7 E8 ? ? ? ? 48 8D 4E 30 44 0F 28 C8 E8 ? ? ? ?", true, 3));
    if (!HealthProperty)
    {
        INIT_POINTER(HealthProperty, memory_manager::find_pattern("48 8B 15 ? ? ? ? 48 8D 4D EF E8 ? ? ? ? 48 8D 55 EF 48 8B CE E8 ? ? ? ? 84 C0 74 05 40 B7 01 EB 03 40 32 FF 41 F6 C6 01 74 12 48 8B 4D EF 41 83 E6 FE 48 85 C9 74 05 E8 ? ? ? ?", true, 3));
    }
    
    // Initialize serverchoosepart
    if (const uintptr_t index = memory_manager::find_function_by_string(L"ServerChoosePart_Validate", {0xff, 0x90}, 2, false, false))
    {
        INIT_POINTER(ServerChoosePart_Implementation, FindObject(L"/Script/FortniteGame.Default__FortPlayerPawn")->VTable[index/8]);
    }

    INIT_POINTER(UpdateZiplineState, memory_manager::find_function_by_string(L"AFortPlayerPawn::OnRep_ZiplineState"));
    INIT_POINTER(GetPoiGridTagsForPawn, memory_manager::find_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 99 ? ? ? ? 49 8B F0 48 8B FA 48 85 DB 0F 84 ? ? ? ? E8 ? ? ? ? 48 8B 53 10 4C 8D 88 ? ? ? ? 49 63 41 08 3B 82 ? ? ? ? 7F 14 48 8B C8 48 8B 82 ? ? ? ? 4C 39 0C C8 75 04"))

    if (UFunction* ClientAddMarker = FindObject<UFunction>(L"/Script/FortniteGame.AthenaMarkerComponent.ClientAddMarker"))
    {
        uintptr_t WorldMarkerDataCtr_Call = memory_manager::scan_for(reinterpret_cast<uintptr_t>(ClientAddMarker->get_native_function()), {0xE8, WILDCARD, WILDCARD, WILDCARD, 0xFF});
        uintptr_t WorldMarkerData_Addr = WorldMarkerDataCtr_Call + 5 + *reinterpret_cast<int*>(WorldMarkerDataCtr_Call + 1);
        
        INIT_POINTER(WorldMarkerDataCtr, WorldMarkerData_Addr);
    }

    // BREAK DOWN THE REGISTER_CALL_HOOK MACRO AND USE STRIPPED FUNCTIONS GETTERS INSTEAD
    { UFunction* function_to_hook = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.SpawnToyInstance") ?
        FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.SpawnToyInstance") :
        FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.SpawnToyInstance", false, true);
        if (function_to_hook != nullptr)
        {
            const auto all_calls_in_function = memory_manager::get_all_stripped_calls_in_function_wiktor(reinterpret_cast<uintptr_t>(function_to_hook->get_native_function()));
            memory_manager::call_hook(all_calls_in_function[all_calls_in_function.size() - 1], reinterpret_cast<uintptr_t>(player_component_hooks::SpawnToyInstance_Hook));
        } else
        {
            spdlog::error(L"Failed to hook '{}', invalid function object provided", L"/Script/FortniteGame.FortPlayerController.SpawnToyInstance");
        } };

    // Call hook in StartGhostMode
    {
        if (uintptr_t target_addr = memory_manager::find_pattern("48 89 5C 24 ? 57 48 83 EC 30 80 B9 ? ? ? ? ? 48 8B FA 48 8B D9 75 61 80 B9 ? ? ? ? ? 75 58 48 8B 01 FF 90 ? ? ? ? 48 C7 44 24"))
        {
            const auto all_calls_in_function = memory_manager::get_all_stripped_calls_in_function_wiktor(target_addr);
            memory_manager::call_hook(all_calls_in_function[all_calls_in_function.size() - 1], reinterpret_cast<uintptr_t>(player_component_hooks::GiveItemToPlayer_Hook));
        }
    }

    // Call hook in ApplyGadgetItem....
    {
        if (uintptr_t target_addr = memory_manager::find_pattern("E8 ? ? ? ? 49 8D 8E ? ? ? ? B2 01 E8 ? ? ? ? 41 8B 96 ? ? ? ? 4C 8B C0 48 8B CF E8 ? ? ? ? EB 1A 3C 02 75 16 48 8B 07 49 8D 96 ? ? ? ? 41 B0 01 48 8B CF"))
        {
            memory_manager::call_hook(target_addr, reinterpret_cast<uintptr_t>(player_component_hooks::PushAnimBPOverride_Hook));
        }
        if (uintptr_t target_addr = memory_manager::find_pattern("E8 ? ? ? ? 48 8D 8F ? ? ? ? B2 01 E8 ? ? ? ? 8B 97 ? ? ? ? 4C 8B C0 48 8B CE E8 ? ? ? ? E9 ? ? ? ? 3C 02 0F 85 ? ? ? ? 49 8B 06 48 8D 97 ? ? ? ? 45 33 C0 49 8B CE FF 90"))
        {
            memory_manager::call_hook(target_addr, reinterpret_cast<uintptr_t>(player_component_hooks::PopAnimBPOverride_Hook));
        }
    }
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerAcknowledgePossession, L"/Script/Engine.PlayerController.ServerAcknowledgePossession", player_component_hooks::ServerAcknowledgePossession_Hook);
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerChoosePart, L"/Script/FortniteGame.FortPlayerPawn.ServerChoosePart", player_component_hooks::ServerChoosePart_Hook);
    
    REGISTER_FUNCTION_DETOUR(player_component_hooks::PostLogin, memory_manager::find_function_by_string(L"FortGameModeAthena: Player [%s] PostLogin"), player_component_hooks::PostLogin_Hook);
    
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerPlayEmoteItem, L"/Script/FortniteGame.FortPlayerController.ServerPlayEmoteItem", player_component_hooks::ServerPlayEmoteItem_Hook);
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerPlaySprayItem, L"/Script/FortniteGame.FortPlayerController.ServerPlaySprayItem", player_component_hooks::ServerPlaySprayItem_Hook);
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::MovingEmoteStopped, L"/Script/FortniteGame.FortPawn.MovingEmoteStopped", player_component_hooks::MovingEmoteStopped_Hook);
    
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerSendZiplineState, L"/Script/FortniteGame.FortPlayerPawn.ServerSendZiplineState", player_component_hooks::ServerSendZiplineState_Hook);
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerAddMapMarker, L"/Script/FortniteGame.AthenaMarkerComponent.ServerAddMapMarker", player_component_hooks::ServerAddMapMarker_Hook);
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerReturnToMainMenu, L"/Script/FortniteGame.FortPlayerController.ServerReturnToMainMenu", player_component_hooks::ServerReturnToMainMenu_Hook);
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerReadyToStartMatch, L"/Script/FortniteGame.FortPlayerController.ServerReadyToStartMatch", player_component_hooks::ServerReadyToStartMatch_Hook);
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerPlaySquadQuickChatMessage, L"/Script/FortniteGame.FortPlayerControllerAthena.ServerPlaySquadQuickChatMessage", player_component_hooks::ServerPlaySquadQuickChatMessage_Hook);
    REGISTER_UFUNCTION_DETOUR(player_component_hooks::ServerAwardVehicleTrickPoints, L"/Script/FortniteGame.FortPlayerControllerGameplay.ServerAwardVehicleTrickPoints", player_component_hooks::ServerAwardVehicleTrickPoints_Hook);
    REGISTER_FUNCTION_DETOUR(player_component_hooks::OnTargetDataReadyCallback, memory_manager::find_function_by_string(L"STAT_FortGA_Ranged_OnTargetDataReadyCB"), player_component_hooks::OnTargetDataReadyCallback_Hook);
    
    if (GetEngineVersionEasy() < 420)
    {
        // Hooking this on later versions causes issues
        REGISTER_FUNCTION_DETOUR(player_component_hooks::GetHeroDefinition, memory_manager::find_function_by_string(L"Trying to apply homebase effects on player setup but hero or herotype is not set.", { 0xE8, 0x100, 0x100, 0x100, 0xff }, 1), player_component_hooks::GetHeroDefinition_Hook);
    }
    
    REGISTER_FUNCTION_DETOUR(player_component_hooks::K2_OnRestartPlayer, memory_manager::find_pe_function(L"K2_OnRestartPlayer"), player_component_hooks::K2_OnRestartPlayer_Hook);
    
    DetourTransactionCommit();

    // Finish spawning a player
    REGISTER_PROCESS_EVENT_DETOUR(L"/Script/FortniteGame.FortPawn.OnDamageServer", player_component_hooks::OnDamageServer_Hook);
    REGISTER_PROCESS_EVENT_DETOUR(L"/Script/FortniteGame.BuildingActor.OnDamageServer", player_component_hooks::OnDamageServer_Hook);
    REGISTER_PROCESS_EVENT_DETOUR(L"/Script/FortniteGame.FortPawn.GameplayCue", player_component_hooks::GameplayCue_Hook);

    REGISTER_CUSTOM_UFUNCTION(OnPawnLandedImplementation, AFortPawn::StaticClass())
    REGISTER_CUSTOM_UFUNCTION(GatherDistanceTraveled, AFortPlayerControllerAthena::StaticClass())

    // Timers for each weapon type
    REGISTER_CUSTOM_UFUNCTION(PistolDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(HandCannonPistolDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(PumpShotgunDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(TacticalShotgunDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(HeavyShotgunDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(ShotgunDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(AssaultRifleDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(BurstAssaultRifleDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(SuppressedAssaultRifleDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(SniperDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(HeavySniperDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(BoltSniperDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(HuntingSniperDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(GrenadeDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(SMGDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(GrenadeLauncherDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(RPGDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(SuppressedSMGDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(SuppressedPistolDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(MinigunDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(LMGDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(EnemyStructureDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(PickaxeDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(HeadshotDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(JewelDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(C4BuildingDamageCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(BuildingPlacedCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(HarvestResourcesCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(TrickPointsCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(CampfireHealthCallback, AFortPlayerController::StaticClass());
    REGISTER_CUSTOM_UFUNCTION(ShieldCallback, AFortPlayerController::StaticClass());
}

AActor* player_component_hooks::SpawnToyInstance_Hook(AFortPlayerController* PlayerController, UObject* ToyClass,
    FTransform SpawnPosition)
{
    if (PlayerController && !memory_manager::is_bad_read_pointer(PlayerController))
    {
        AB_PlayerSpawnedBall_C* toy_instance = static_cast<AB_PlayerSpawnedBall_C*>(UGameplayStatics::SpawnActor(ToyClass, SpawnPosition, 1, PlayerController->PlayerState()));
    
        return toy_instance;   
    }
    
    return nullptr;
}

void player_component_hooks::MovingEmoteStopped_Hook(AFortPawn* FortPawn, FFrame& StackFrame)
{
    MovingEmoteStopped(FortPawn, StackFrame);
    
    if (!FortPawn)
    {
        return;
    }
    
    AFortPlayerController* Controller = FortPawn->Controller();
    if (!Controller)
    {
        return;
    }
    
    UKismetSystemLibrary::SetBoolPropertyByName(FortPawn, L"bMovingEmote", false);
    UKismetSystemLibrary::SetBoolPropertyByName(FortPawn, L"bMovingEmoteForwardOnly", false);

    if (STATIC_HASPROPERTY(FortPawn, EmoteWalkSpeed))
    {
        FortPawn->EmoteWalkSpeed() = 0.0f;
    }
}

void player_component::OnPawnLandedImplementation(AFortPlayerPawn* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self == nullptr || Self->Controller() == nullptr)
    {
        return;
    }

    if (STATIC_HASPROPERTY(Self, bIsSkydivingFromBus))
    {
        if (Self->bIsSkydivingFromBus())
        {
            if (Self->CurrentWeapon() == nullptr)
            {
                Self->Controller()->ServerExecuteInventoryItem(inventory_component::get_pickaxe(Self->Controller())->ItemEntry().ItemGuid());
            }

            // Progress land challenges
            challenges_component::SendLandingEvent(Self->Controller(), Self);
        }
    }
}

void player_component::GatherDistanceTraveled(AFortPlayerControllerAthena* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        if (!memory_manager::is_bad_read_pointer(Self->Pawn()) && Self->Pawn())
        {
            FPlayerStats& PlayerStats = Self->EraPlayerInfo().PlayerStatsTracker;
            
            if (PlayerStats.LastPos.IsValid())
            {
                const FVector CurrentLocation = Self->Pawn()->K2_GetActorLocation();
                const FVector LastPos = PlayerStats.LastPos;

                const float Distance = sqrtf((CurrentLocation.X - LastPos.X) * (CurrentLocation.X - LastPos.X) +
                                            (CurrentLocation.Y - LastPos.Y) * (CurrentLocation.Y - LastPos.Y) +
                                            (CurrentLocation.Z - LastPos.Z) * (CurrentLocation.Z - LastPos.Z));

                PlayerStats.DistanceTraveled += Distance;
            }
            
            PlayerStats.LastPos = Self->Pawn()->K2_GetActorLocation();

            // Get the POI we're currently in
            if (GetPoiGridTagsForPawn)
            {
                bool bOnGround = true;
                
                if (FindFunctionByName(Self->Pawn()->Class, TEXT("IsSkydiving"), 1))
                {
                    if (Self->Pawn()->IsSkydiving())
                    {
                        bOnGround = false;
                    }
                }

                if (bOnGround)
                {
                    FGameplayTagContainer Container;
                    ZeroMemory(&Container, sizeof FGameplayTagContainer);
                    GetPoiGridTagsForPawn((*GWorld)->AuthorityGameMode(), Self->Pawn(), Container);

                    // Align the CurrentPOIList list to the Container
                    for (int i = Self->EraPlayerInfo().CurrentPOIList.size() - 1; i >= 0; i--)
                    {
                        FGameplayTag POI = Self->EraPlayerInfo().CurrentPOIList[i];
                        if (!Container.GameplayTags.Find(Self->EraPlayerInfo().CurrentPOIList[i]))
                        {
                            std::erase(Self->EraPlayerInfo().CurrentPOIList, Self->EraPlayerInfo().CurrentPOIList[i]);

                            // Log
                            if (log_component::enable_debug_logs)
                            {
                                spdlog::info(L"Player '{}' left POI '{}'", Self->MyPlayerInfo()->PlayerName().c_str(), POI.TagName.ToString().c_str());
                            }
                        }
                    }

                    // Now add any tag from the container that isn't already in the CurrentPOIList
                    for (FGameplayTag& Tag : Container.GameplayTags)
                    {
                        if (std::find(Self->EraPlayerInfo().CurrentPOIList.begin(), Self->EraPlayerInfo().CurrentPOIList.end(), Tag) == Self->EraPlayerInfo().CurrentPOIList.end())
                        {
                            Self->EraPlayerInfo().CurrentPOIList.push_back(Tag);

                            // Log
                            if (log_component::enable_debug_logs)
                            {
                                spdlog::info(L"Player '{}' entered POI '{}'", Self->MyPlayerInfo()->PlayerName().c_str(), Tag.TagName.ToString().c_str());
                            }
                    
                            // Notify backend
                            challenges_component::SendCorruptedAreaInteraction(Self, Tag);
                            challenges_component::SendPOIInteraction(Self, Tag.TagName);
                        }
                    }
                }
            }
        }
        
        // Re-execute the same function in 1 second
        UKismetSystemLibrary::K2_SetTimer(Self, L"GatherDistanceTraveled", 1, false);
    }
}

void player_component::PistolDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "pistol"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.PistolDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.PistolDamage = 0;
        }
    }
}

void player_component::HandCannonPistolDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "hand_cannon"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.HandCannonPistolDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.HandCannonPistolDamage = 0;
        }
    }
}

void player_component::PumpShotgunDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "pump_shotgun"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.PumpShotgunDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.PumpShotgunDamage = 0;
        }
    }
}

void player_component::TacticalShotgunDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "tactical_shotgun"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.TacticalShotgunDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.TacticalShotgunDamage = 0;
        }
    }
}

void player_component::HeavyShotgunDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "heavy_shotgun"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.HeavyShotgunDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.HeavyShotgunDamage = 0;
        }
    }
}

void player_component::ShotgunDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "shotgun"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.ShotgunDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.ShotgunDamage = 0;
        }
    }
}

void player_component::AssaultRifleDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "assault_rifle"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.AssaultRifleDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.AssaultRifleDamage = 0;
        }
    }
}

void player_component::BurstAssaultRifleDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "burst_assault_rifle"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.BurstAssaultRifleDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.BurstAssaultRifleDamage = 0;
        }
    }
}

void player_component::SuppressedAssaultRifleDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "silenced_assault_rifle"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.SuppressedAssaultRifleDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.SuppressedAssaultRifleDamage = 0;
        }
    }
}

void player_component::SniperDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "sniper"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.SniperDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.SniperDamage = 0;
        }
    }
}

void player_component::HeavySniperDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "heavy_sniper_rifle"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.HeavySniperDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.HeavySniperDamage = 0;
        }
    }
}

void player_component::BoltSniperDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "bolt_action_sniper_rifle"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.BoltSniperDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.BoltSniperDamage = 0;
        }
    }
}

void player_component::HuntingSniperDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "hunting_rifle"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.HuntingSniperDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.HuntingSniperDamage = 0;
        }
    }
}

void player_component::GrenadeDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "grenade"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.GrenadeDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:grenade", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.GrenadeDamage = 0;
        }
    }
}

void player_component::SMGDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "smg"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.SMGDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.SMGDamage = 0;
        }
    }
}

void player_component::GrenadeLauncherDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "grenade_launcher"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.GrenadeLauncherDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.GrenadeLauncherDamage = 0;
        }
    }
}

void player_component::RPGDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "rpg"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.RPGDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.RPGDamage = 0;
        }
    }
}

void player_component::SuppressedSMGDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "suppressed_smg"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.SuppressedSMGDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.SuppressedSMGDamage = 0;
        }
    }
}

void player_component::SuppressedPistolDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "suppresssed_pistol"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.SuppressedPistolDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.SuppressedPistolDamage = 0;
        }
    }
}

void player_component::MinigunDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "minigun"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.MinigunDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.MinigunDamage = 0;
        }
    }
}

void player_component::LMGDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "lmg"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.LMGDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.LMGDamage = 0;
        }
    }
}

void player_component::EnemyStructureDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "enemy_building"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.EnemyStructureDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.EnemyStructureDamage = 0;
        }
    }
}

void player_component::PickaxeDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"weapon", "pickaxe"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.PickaxeDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.PickaxeDamage = 0;
        }
    }
}

void player_component::HeadshotDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.HeadshotDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:headshot-damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.HeadshotDamage = 0;
        }
    }
}

void player_component::JewelDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"type", "jewel_opponent"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.SpecialDamage.JewelOpponent}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:special-damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.SpecialDamage.JewelOpponent = 0;
        }
    }
}

void player_component::C4BuildingDamageCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (Self)
    {
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json payload =
            {
                {"type", "c4_enemy_building"},
                {"count", static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.SpecialDamage.C4BuildingDamage}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:special-damage", payload);
            static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().DamageDealtTracker.SpecialDamage.C4BuildingDamage = 0;
        }
    }
}

// This function is called whenever we have to process any kind of debounced damage
void player_component::HandleDebouncedDamage(AFortPlayerController* Self, EDamageCauser DamageCauser)
{
    switch (DamageCauser)
    {
    case EDamageCauser::HandCannonPistol:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"HandCannonPistolDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"HandCannonPistolDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"HandCannonPistolDamageCallback", 1.0, false);
        break;
    case EDamageCauser::Pistol:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"PistolDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"PistolDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"PistolDamageCallback", 1.0, false);
        break;
    case EDamageCauser::SuppressedPistol:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"SuppressedPistolDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"SuppressedPistolDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"SuppressedPistolDamageCallback", 1.0, false);
        break;
    case EDamageCauser::PumpShotgun:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"PumpShotgunDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"PumpShotgunDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"PumpShotgunDamageCallback", 1.0, false);
        break;
    case EDamageCauser::TacticalShotgun:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"TacticalShotgunDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"TacticalShotgunDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"TacticalShotgunDamageCallback", 1.0, false);
        break;
    case EDamageCauser::HeavyShotgun:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"HeavyShotgunDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"HeavyShotgunDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"HeavyShotgunDamageCallback", 1.0, false);
        break;
    case EDamageCauser::Shotgun:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"ShotgunDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"ShotgunDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"ShotgunDamageCallback", 1.0, false);
        break;
    case EDamageCauser::AssaultRifle:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"AssaultRifleDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"AssaultRifleDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"AssaultRifleDamageCallback", 1.0, false);
        break;
    case EDamageCauser::BurstAssaultRifle:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"BurstAssaultRifleDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"BurstAssaultRifleDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"BurstAssaultRifleDamageCallback", 1.0, false);
        break;
    case EDamageCauser::SuppressedAssaultRifle:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"SuppressedAssaultRifleDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"SuppressedAssaultRifleDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"SuppressedAssaultRifleDamageCallback", 1.0, false);
        break;
    case EDamageCauser::Sniper:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"SniperDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"SniperDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"SniperDamageCallback", 1.0, false);
        break;
    case EDamageCauser::HeavySniper:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"HeavySniperDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"HeavySniperDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"HeavySniperDamageCallback", 1.0, false);
        break;
    case EDamageCauser::HuntingSniper:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"HuntingSniperDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"HuntingSniperDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"HuntingSniperDamageCallback", 1.0, false);
        break;
    case EDamageCauser::BoltSniper:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"BoltSniperDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"BoltSniperDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"BoltSniperDamageCallback", 1.0, false);
        break;
    case EDamageCauser::Grenade:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"GrenadeDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"GrenadeDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"GrenadeDamageCallback", 1.0, false);
        break;
    case EDamageCauser::SMG:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"SMGDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"SMGDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"SMGDamageCallback", 1.0, false);
        break;
    case EDamageCauser::SuppressedSMG:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"SuppressedSMGDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"SuppressedSMGDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"SuppressedSMGDamageCallback", 1.0, false);
        break;
    case EDamageCauser::GrenadeLauncher:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"GrenadeLauncherDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"GrenadeLauncherDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"GrenadeLauncherDamageCallback", 1.0, false);
        break;
    case EDamageCauser::LMG:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"LMGDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"LMGDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"LMGDamageCallback", 1.0, false);
        break;
    case EDamageCauser::Minigun:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"MinigunDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"MinigunDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"MinigunDamageCallback", 1.0, false);
        break;
    case EDamageCauser::RPG:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"RPGDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"RPGDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"RPGDamageCallback", 1.0, false);
        break;
    case EDamageCauser::Pickaxe:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"PickaxeDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"PickaxeDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"PickaxeDamageCallback", 1.0, false);
        break;
    case EDamageCauser::Headshot:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"HeadshotDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"HeadshotDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"HeadshotDamageCallback", 1.0, false);
        break;
    case EDamageCauser::Jewel:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"JewelDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"JewelDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"JewelDamageCallback", 1.0, false);
        break;
    case EDamageCauser::C4:
        if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"C4BuildingDamageCallback"))
        {
            UKismetSystemLibrary::K2_ClearTimer(Self, L"C4BuildingDamageCallback");
        }
        UKismetSystemLibrary::K2_SetTimer(Self, L"C4BuildingDamageCallback", 1.0, false);
        break;
    default:
        break;
    }
}

void player_component::BuildingPlacedCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (!memory_manager::is_bad_read_pointer(Self) && Self && !memory_manager::is_bad_read_pointer(Self->Pawn()) && Self->Pawn())
    {
        FPlayerStats& Value = static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().PlayerStatsTracker;
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", Self->Pawn()->K2_GetActorLocation().X},
                {"y", Self->Pawn()->K2_GetActorLocation().Y},
                {"count", Value.TemporaryPlacedBuilds}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:structure", parameters);
            Value.TemporaryPlacedBuilds = 0;
        }
    }
}

void player_component::HarvestResourcesCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION();

    if (!memory_manager::is_bad_read_pointer(Self) && Self && !memory_manager::is_bad_read_pointer(Self->Pawn()) && Self->Pawn())
    {
        FPlayerStats& Value = static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().PlayerStatsTracker;
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                    {"x", Self->Pawn()->K2_GetActorLocation().X},
                    {"y", Self->Pawn()->K2_GetActorLocation().Y},
                    {"amount", Value.TemporaryMaterialsGathered}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:building-resource", parameters);
            Value.TemporaryMaterialsGathered = 0;
        }
    }
}

void player_component::TrickPointsCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION()

    if (!memory_manager::is_bad_read_pointer(Self) && Self && !memory_manager::is_bad_read_pointer(Self->Pawn()) && Self->Pawn())
    {
        FPlayerStats& Value = static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().PlayerStatsTracker;
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", Self->Pawn()->K2_GetActorLocation().X},
                {"y", Self->Pawn()->K2_GetActorLocation().Y},
                {"amount", Value.TemporaryTrickPoints}
            };

            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:trick-points", parameters);
            Value.TemporaryTrickPoints = 0;
        }
    }
}

void player_component::ShieldCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION()

    if (!memory_manager::is_bad_read_pointer(Self) && Self && !memory_manager::is_bad_read_pointer(Self->Pawn()) && Self->Pawn())
    {
        FPlayerStats& Value = static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().PlayerStatsTracker;
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", Self->Pawn()->K2_GetActorLocation().X},
                {"y", Self->Pawn()->K2_GetActorLocation().Y},
                {"count", Value.TemporaryObtainedShield}
            };
            
            if (log_component::enable_debug_logs)
            {
                spdlog::info(L"player_component::ShieldCallback is sending TemporaryObtainedShield: {}", Value.TemporaryObtainedShield);
            }

            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:shield", parameters);
            Value.TemporaryObtainedShield = 0;
        }
    }
}

void player_component::CampfireHealthCallback(AFortPlayerController* Self, FFrame& StackFrame)
{
    INIT_CUSTOM_FUNCTION()

    if (!memory_manager::is_bad_read_pointer(Self) && Self && !memory_manager::is_bad_read_pointer(Self->Pawn()) && Self->Pawn())
    {
        FPlayerStats& Value = static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().PlayerStatsTracker;
        std::string account_id = get_account_id(Self);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", Self->Pawn()->K2_GetActorLocation().X},
                {"y", Self->Pawn()->K2_GetActorLocation().Y},
                {"source", "campfire"},
                {"count", Value.TemporaryCampfireHealth}
            };

            if (log_component::enable_debug_logs)
            {
                spdlog::info(L"player_component::CampfireHealthCallback is sending TemporaryCampfireHealth: {}", Value.TemporaryCampfireHealth);
            }

            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:heal", parameters);
            Value.TemporaryCampfireHealth = 0;
        }
    }
}

bool player_component::Login(const FUniqueNetIdRepl& PlayerID, const FString& Options)
{
    // Only let people with a valid encryption token join
#ifdef _RELEASE
    if (UGameplayStatics::HasOption(Options, TEXT("EncryptionToken")))
    {
        FString EncryptionToken = UGameplayStatics::ParseOption(Options, TEXT("EncryptionToken"));

        if (EncryptionToken.IsValid())
        {
            for (int i = 0; i < EncryptionTokenList.size(); i++)
            {
                auto Token = EncryptionTokenList[i];
                 
                if (wcsstr(Token.c_str(), EncryptionToken.c_str()))
                {
                    EncryptionTokenList.erase(EncryptionTokenList.begin() + i);

                    spdlog::info(L"EncryptionToken successfully validated.");
                    
                    return true;
                }
            }

            spdlog::error(L"Client sent '{}' as EncryptionToken, but the server could not validate it. Valid encryption tokens: {}", EncryptionToken.c_str(), EncryptionTokenList.size());
        }
        else
        {
            spdlog::error(L"Invalid EncryptionToken received, kicking...");
        }

        InvalidTokensReceived++;

        if (InvalidTokensReceived >= 10)
        {
            // Mark as pending shutdown
            loader_component::bPendingShutdown = true;

            // Log
            spdlog::warn(L"loader_component::bPendingShutdown changed to true: InvalidTokensReceived exceeds maximum allowed value.");
        }
        
        // Limit for invalid tokens
        if (InvalidTokensReceived >= 150)
        {
            // Leave Era Services
            gameserver_client::Client->SendLeave();
            
            // Close director connection
            if (director_client::client)
            {
                director_client::client->Close();
            }
            
            // Close
            RequestExitWithStatus(true, 0);
        }
    }
    else
    {
        spdlog::error(L"Player tried to join without EncryptionToken, kicking...");
    }
    return false;
#endif

    return true;
}

void player_component::PostLogin(AFortPlayerController* FortPlayerController)
{
    if (FortPlayerController)
    {
        // PHX JOIN the player's channel
        std::string account_id = get_account_id(FortPlayerController);

        if (account_id.size() > 0)
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            std::string team_id;

            if (era_team* team = team_component::find_era_team(FortPlayerController, false))
            {
                if (team->bound_era_team_info)
                {
                    std::wstring wteam_id = team->bound_era_team_info->TeamID;
                    TO_STRING(wteam_id, steam_id);

                    team_id = steam_id;
                }
            }
            
            if (team_id.length() == 0)
            {
                team_id = account_id;
            }
            
            std::map<std::string, std::string> parameters{
                {"match", Matches[account_id]},
                {"team", team_id}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "phx_join", parameters);

            // Initialize stats
            static_cast<AFortPlayerControllerAthena*>(FortPlayerController)->GatherDistanceTraveled();
            
            // Log
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> name_converter;
            std::wstring wPlayer_name = FortPlayerController->MyPlayerInfo()->PlayerName().c_str();
            std::string player_name = name_converter.to_bytes(wPlayer_name);
            
            spdlog::info("Player '{}' joined its channel. [SERVER: {}, TEAM: {}, ACCOUNT_ID: {}]", player_name.c_str(), gameserver_client::Client->UUID.c_str(), team_id.c_str(), account_id.c_str());
        }
    }
}

void player_component::Setup(AFortPlayerController* NewPlayer, const FString& Options, UNetConnection* Connection)
{
    // Setup platform
    if (UGameplayStatics::HasOption(Options, TEXT("Platform")))
    {
        FString Platform = UGameplayStatics::ParseOption(Options, TEXT("Platform"));

        if (Platform.IsValid())
        {
            static_cast<AFortPlayerControllerAthena*>(NewPlayer)->EraPlayerInfo().PlayerPlatform = std::wstring(Platform.c_str());
        }
    }
    
    if (UGameplayStatics::HasOption(Options, TEXT("EncryptionToken")))
    {
        FString Token = UGameplayStatics::ParseOption(Options, TEXT("EncryptionToken"));

        if (Token.IsValid())
        {
            // Encryption token
            std::wstring wToken = Token.c_str();

            // Register this player in the local Players map
            std::wstring wAccountID = wToken.substr(0, wToken.find(L":"));
            static_cast<AFortPlayerControllerAthena*>(NewPlayer)->EraPlayerInfo().AccountId = wAccountID;

            // Convert string
            TO_STRING(wAccountID, sAccountID);

            struct ProfileParsingParameters
            {
                ProfileParsingParameters(AFortPlayerController* InNewPlayer, FString InOptions, UNetConnection* InConnection, std::wstring InAccountID)
                {
                    NewPlayer = InNewPlayer;
                    Options = InOptions;
                    Connection = InConnection;
                    AccountID = InAccountID;
                }
                
                AFortPlayerController* NewPlayer;
                FString Options;
                UNetConnection* Connection;
                std::wstring AccountID;
            };
            ProfileParsingParameters* NewProfileParsingParameters = new ProfileParsingParameters(NewPlayer, Options, Connection, wAccountID);
            
            // Query profile
            CreateThread(nullptr, 0, [](LPVOID INParams) -> DWORD
            {
                try
                {
                    ProfileParsingParameters* NewProfileParsingParameters = static_cast<ProfileParsingParameters*>(INParams);
                    AFortPlayerController* NewPlayer = NewProfileParsingParameters->NewPlayer;
                    FString Options = NewProfileParsingParameters->Options;
                    UNetConnection* Connection = NewProfileParsingParameters->Connection;
                    std::wstring AccountID = NewProfileParsingParameters->AccountID;
                    TO_STRING(AccountID, sAccountID)
                
                    while (true)
                    {
                        httplib::Client ClientReq(API_URL);
                        ClientReq.set_read_timeout(15, 0); // 15 seconds
                        ClientReq.set_write_timeout(15, 0); // 15 seconds
                        httplib::Headers headers = {
                            { "x-era-token", "bn8HUsAfhogEq6E57ledvyGReK9n8E7XP53AbRomzvItAexNB09aXiTgFhoPvUD1" }
                        };
                        auto Response = ClientReq.Get("/api/internal/users/" + sAccountID + "/profile" + "?build=" + std::to_string(GetCL()), headers);
                        if (Response && Response->status == 200)
                        {
                            try
                            {
                                nlohmann::json parsed_response = nlohmann::json::parse(Response->body);

                                try
                                {
                                    static_cast<AFortPlayerControllerAthena*>(NewPlayer)->EraPlayerInfo().Profile = parsed_response;
                                    
                                    break;
                                }
                                catch (nlohmann::json::exception& err)
                                {
                                }
                            }
                            catch (nlohmann::json::parse_error& err)
                            {
                            }
                        }
                        else
                        {
                            spdlog::error("Failed to query profile [{}], invalid Response. Retrying in 1000ms...", "/api/internal/users/" + sAccountID + "/profile");
                            static_cast<AFortPlayerControllerAthena*>(NewPlayer)->EraPlayerInfo().ProfileFailures++;
                            if (static_cast<AFortPlayerControllerAthena*>(NewPlayer)->EraPlayerInfo().ProfileFailures >= 5)
                            {
                                // Mark as bPendingKick
                                static_cast<AFortPlayerControllerAthena*>(NewPlayer)->EraPlayerInfo().bPendingKick = true;
                            }
                            if (Response)
                            {
                                spdlog::error("Query profile failure error code: {}", Response->status);
                            }
                        }

                        Sleep(1000);
                    }
                    
                    nlohmann::json data = static_cast<AFortPlayerControllerAthena*>(NewPlayer)->EraPlayerInfo().Profile;
                    
                    // Handle custom replication
                    if (static_cast<AFortPlayerControllerAthena*>(NewPlayer)->EraPlayerInfo().PlayerPlatform.size() > 0)
                    {
                        // If on iOS (5.41), do NOT replicate the following properties
                        if (GetCL() == 4363240)
                        {
                            if (static_cast<AFortPlayerControllerAthena*>(NewPlayer)->EraPlayerInfo().PlayerPlatform == TEXT("IOS"))
                            {
                                net_component::BlockPropertyReplicationPerConnection(Connection, AFortPlayerStateAthena::StaticClass(), TEXT("CharacterParts"));
                            }
                        }
                        
                        // Log
                        std::wstring PlayerPlatform = static_cast<AFortPlayerControllerAthena*>(NewPlayer)->EraPlayerInfo().PlayerPlatform;
                        TO_STRING(PlayerPlatform, sPlayerPlatform)
                        if (data.contains("display_name"))
                        {
                            spdlog::info("Player's ['{}'] platform: {}", std::string(data["display_name"]), sPlayerPlatform.c_str());
                        }
                    }
            
                    if (data.contains("admin"))
                    {
                        if (static_cast<bool>(data["admin"]))
                        {
                            // Give admin permission
                            NewPlayer->CheatManager() = static_cast<UCheatManager*>(UGameplayStatics::SpawnObject(UCheatManager::StaticClass(), NewPlayer));

                            if (data.contains("display_name"))
                            {
                                spdlog::info("Player '{}' joined as an admin [AUTHORIZED BY ERA SERVICES].", std::string(data["display_name"]));
                            }
                        }
                    }

                    if (data.contains("level"))
                    {
                        if (static_cast<int>(data["level"]) > 0)
                        {
                            initialize_player_banner_level(NewPlayer, data["level"]);
                        }
                    }
    
                    if (data.contains("locker"))
                    {
                        nlohmann::json locker = data["locker"];

                        if (locker.contains("emotes"))
                        {
                            nlohmann::json emotes = locker["emotes"];

                            // Emote ownership
                            if (emotes.is_array())
                            {
                                for (auto& elem : emotes.items())
                                {
                                    std::string emote = elem.value();

                                    if (emote.find(":") != std::string::npos)
                                    {
                                        emote = emote.substr(emote.find(":") + 1);
                                        TO_WSTRING(emote, wEmote);

                                        if (UAthenaDanceItemDefinition* EmoteAsset = FindObject<UAthenaDanceItemDefinition>(wEmote.c_str(), false, true))
                                        {
                                            static_cast<AFortPlayerControllerAthena*>(NewPlayer)->EraPlayerInfo().OwnedEmotes.push_back(EmoteAsset);
                                        }
                                    }
                                }
                            }
                        }

                        for (auto& elem : locker.items())
                        {
                            if (elem.value().is_object())
                            {
                                UObject* PickedAsset = nullptr;
                        
                                if (elem.value().contains("asset"))
                                {
                                    nlohmann::json asset = elem.value()["asset"];
                            
                                    // If it's a valid string
                                    if (asset.is_string() && std::string(asset).find(":") != std::string::npos)
                                    {
                                        // Banners
                                        if (locker.contains("banner") &&
                                            locker["banner"].contains("asset") &&
                                            locker.contains("banner_color") &&
                                            locker["banner_color"].contains("asset"))
                                        {
                                            if (asset == locker["banner"]["asset"])
                                            {
                                                initialize_player_banner_icon(NewPlayer, asset);
                                
                                                continue;
                                            }
                                            if (asset == locker["banner_color"]["asset"])
                                            {
                                                initialize_player_banner_color(NewPlayer, asset);
                                
                                                continue;
                                            }
                                        }
                        
                                        std::string value = std::string(asset).substr(std::string(asset).find(":")+1);

                                        UObject* Object = FindObject(std::wstring(value.begin(), value.end()).c_str(), false, true);

                                        // Check if the backend didn't send a literal random item definition
                                        UAthenaGameData* athena_game_data = UFortGameData::get_athena_game_data();

                                        if (athena_game_data)
                                        {
                                            if (athena_game_data->HasProperty("ShuffleTileItemDefinitions"))
                                            {
                                                for (auto shuffle_tile_soft_item : athena_game_data->ShuffleTileItemDefinitions())
                                                {
                                                    if (UFortItemDefinition* ShuffleTileItem = shuffle_tile_soft_item.Get())
                                                    {
                                                        if (Object == ShuffleTileItem)
                                                        {
                                                            Object = nullptr;
                                                        }
                                                    }
                                                } 
                                            }
                                        }
                                
                                        if (Object)
                                        {
                                            if (Object->IsA(UAthenaCharacterItemDefinition::StaticClass()))
                                            {
                                                PickedAsset = Object;
                                                static_cast<AFortPlayerControllerAthena*>(NewPlayer)->CustomizationLoadout().Character() = static_cast<UAthenaCharacterItemDefinition*>(Object);
                                                if (Object->HasProperty("HeroDefinition"))
                                                {
                                                    if (!NewPlayer->PlayerState()->HeroType())
                                                    {
                                                        NewPlayer->PlayerState()->HeroType() = static_cast<UAthenaCharacterItemDefinition*>(Object)->HeroDefinition();
                                                    }
                                                }
                                            }
                                            else if (Object->IsA(UAthenaBackpackItemDefinition::StaticClass()))
                                            {
                                                PickedAsset = Object;
                                                static_cast<AFortPlayerControllerAthena*>(NewPlayer)->CustomizationLoadout().Backpack() = static_cast<UAthenaBackpackItemDefinition*>(Object);
                                            }
                                            else if (Object->IsA(UAthenaPickaxeItemDefinition::StaticClass()))
                                            {
                                                PickedAsset = Object;
                                                static_cast<AFortPlayerControllerAthena*>(NewPlayer)->CustomizationLoadout().Pickaxe() = static_cast<UAthenaPickaxeItemDefinition*>(Object);
                                            }
                                            else if (Object->IsA(UAthenaGliderItemDefinition::StaticClass()))
                                            {
                                                PickedAsset = Object;
                                                static_cast<AFortPlayerControllerAthena*>(NewPlayer)->CustomizationLoadout().Glider() = static_cast<UAthenaGliderItemDefinition*>(Object);
                                            }
                                            else if (Object->IsA(UAthenaSkyDiveContrailItemDefinition::StaticClass()))
                                            {
                                                PickedAsset = Object;
                                                static_cast<AFortPlayerControllerAthena*>(NewPlayer)->CustomizationLoadout().SkyDiveContrail() = static_cast<UAthenaSkyDiveContrailItemDefinition*>(Object);
                                            }
                                        }
                                    }
                                }
                                if (elem.value().contains("variants"))
                                {
                                    nlohmann::json variants = elem.value()["variants"];

                                    if (variants.is_array())
                                    {
                                        // Save variants
                                        static_cast<AFortPlayerControllerAthena*>(NewPlayer)->EraPlayerInfo().PendingVariantsData.push_back(std::make_pair(variants, PickedAsset));
                                        
                                        // Apply variant
                                        apply_variant(&static_cast<AFortPlayerControllerAthena*>(NewPlayer)->CustomizationLoadout(), variants, PickedAsset);
                                    }
                                }
                            }
                        }

                        static_cast<AFortPlayerControllerAthena*>(NewPlayer)->EraPlayerInfo().bProfileReady = true;
                    }
                }
                catch (std::exception& err)
                {
                    spdlog::error("Error {} occurred while reading profile. Kicking player...", err.what());
                    if (static_cast<ProfileParsingParameters*>(INParams)->NewPlayer)
                    {
                        static_cast<ProfileParsingParameters*>(INParams)->NewPlayer->ClientReturnToMainMenu(L"Failed to parse profile.");
                    }
                }
                
                return 0;
            }, NewProfileParsingParameters, 0, nullptr);
        }
    }
}

bool player_component::is_live_spectator(APlayerController* player_controller)
{
    STATIC_HASPROPERTY((*GWorld)->AuthorityGameMode(), LiveSpectatorPlayerControllerClass);   
    STATIC_HASPROPERTY((*GWorld)->AuthorityGameMode(), LiveBroadcastPlayerControllerClass);

    if (bExists_LiveBroadcastPlayerControllerClass && bExists_LiveSpectatorPlayerControllerClass)
    {
        if (player_controller->IsA((*GWorld)->AuthorityGameMode()->LiveBroadcastPlayerControllerClass()) ||
            player_controller->IsA((*GWorld)->AuthorityGameMode()->LiveSpectatorPlayerControllerClass()))
        {
            return true;
        }
    }
    
    return false;
}

void player_component::ApplyBackpack(AFortPlayerControllerAthena* NewPlayer)
{
    if (NewPlayer->Pawn())
    {
        if (FindPropertyByName(FFortAthenaLoadout::GetClass(), TEXT("Backpack")))
        {
            if (NewPlayer->CustomizationLoadout().Backpack())
            {
                if (STATIC_HASPROPERTY(NewPlayer->CustomizationLoadout().Backpack(), CharacterParts))
                {
                    if (NewPlayer->CustomizationLoadout().Backpack()->CharacterParts().Num())
                    {
                        NewPlayer->Pawn()->ServerChoosePart(NewPlayer->CustomizationLoadout().Backpack()->CharacterParts()[0]->CharacterPartType(), NewPlayer->CustomizationLoadout().Backpack()->CharacterParts()[0]);
                    }
                }
            }
        }
    }
}

void player_component::play_emote_item(AFortPlayerController* player_controller, UAthenaDanceItemDefinition* dance_asset, bool b_requires_validation)
{
    if (dance_asset)
    {
        if (player_controller->Pawn() && player_controller->Pawn()->AbilitySystemComponent())
        {
#ifdef _RELEASE
            if (b_requires_validation)
            {
                // Make sure the player owns the asset
                if (std::find(static_cast<AFortPlayerControllerAthena*>(player_controller)->EraPlayerInfo().OwnedEmotes.begin(),
                              static_cast<AFortPlayerControllerAthena*>(player_controller)->EraPlayerInfo().OwnedEmotes.end(),
                              dance_asset) == static_cast<AFortPlayerControllerAthena*>(player_controller)->EraPlayerInfo().OwnedEmotes.end())
                {
                    // Log
                    spdlog::error(L"Received attempt to play non-owned emote item.");
                
                    return;
                }
            }
#endif

            // Check if our pawn is overlapping anything
            TArray<UPrimitiveComponent*> OverlappedComponents;
            player_controller->Pawn()->GetOverlappingComponents(&OverlappedComponents);
            for (UPrimitiveComponent* Component : OverlappedComponents)
            {
                if (AActor* Owner = Component->GetOwner())
                {
                    if (Owner->IsA(AStreet_Light_Spotlight_AthenaQuest_C::StaticClass()))
                    {
                        if (Owner->HasProperty(TEXT("ObjectiveBackendName")))
                        {
                            challenges_component::SendSpotlightDanceInteraction(player_controller, static_cast<AStreet_Light_Spotlight_AthenaQuest_C*>(Owner), static_cast<AStreet_Light_Spotlight_AthenaQuest_C*>(Owner)->ObjectiveBackendName());
                        }
                    }
                }
            }
            
            auto new_ability_spec = FGameplayAbilitySpec();
        
            // set the ability in the newly created struct
            if (dance_asset->IsA(UAthenaSprayItemDefinition::StaticClass()))
            {
                new_ability_spec.Ability = CreateDefaultObject<UObject>(UFortGameData::get_spray_gameplay_ability());
            
                // Notify backend that we're dancing
                challenges_component::SendSprayInteraction(player_controller);
            }
            else if (dance_asset->IsA(UAthenaToyItemDefinition::StaticClass()))
            {
                auto toy_asset = static_cast<UAthenaToyItemDefinition*>(dance_asset);
                new_ability_spec.Ability = CreateDefaultObject<UObject>(toy_asset->ToySpawnAbility().Get());
            }
            else
            {
                new_ability_spec.Ability = CreateDefaultObject<UObject>(UFortGameData::get_emote_gameplay_ability());
            
                // Notify backend that we're dancing
                challenges_component::SendDanceInteraction(player_controller);
            }
        
            new_ability_spec.SourceObject = dance_asset; // set the sourceobject, in this case the emote we want to execute
            new_ability_spec.Level = 1; 
            new_ability_spec.InputID = -1;
            new_ability_spec.ReplicationID = -1;
            new_ability_spec.ReplicationKey = -1;
            new_ability_spec.MostRecentArrayReplicationKey = -1;
            new_ability_spec.Handle = rand();

            const auto unk = new DWORD();

            // plays the emote by activating the ability.
            // GiveAbility has an additional DWORD* param, which we will simply create and use ourselves
            // by using abilities for emoting, you won't have to deal with stopping the montage yourself when the pawn moves
            GiveAbilityAndActivateOnce(player_controller->Pawn()->AbilitySystemComponent(), unk, new_ability_spec);
            delete unk;
            
            // Traversal emotes implementation ( CR. wiktorwiktor12 for help)
            STATIC_HASPROPERTY(dance_asset, bMovingEmote);
            STATIC_HASPROPERTY(dance_asset, bMoveForwardOnly);
            STATIC_HASPROPERTY(dance_asset, WalkForwardSpeed);
            STATIC_HASPROPERTY(player_controller->Pawn(), EmoteWalkSpeed);
            STATIC_HASPROPERTY(player_controller->Pawn(), LastEmoteItemDef);
            STATIC_HASPROPERTY(player_controller->Pawn(), LastReplicatedEmoteExecuted);
            if (bExists_bMovingEmote &&
                bExists_bMoveForwardOnly &&
                bExists_WalkForwardSpeed &&
                bExists_EmoteWalkSpeed &&
                bExists_LastEmoteItemDef &&
                bExists_LastReplicatedEmoteExecuted)
            {
                UKismetSystemLibrary::SetBoolPropertyByName(player_controller->Pawn(), L"bMovingEmote", dance_asset->bMovingEmote());
                UKismetSystemLibrary::SetBoolPropertyByName(player_controller->Pawn(), L"bMovingEmoteForwardOnly", dance_asset->bMoveForwardOnly());
                player_controller->Pawn()->EmoteWalkSpeed() = dance_asset->WalkForwardSpeed();

                player_controller->Pawn()->LastEmoteItemDef() = dance_asset;
                player_controller->Pawn()->LastReplicatedEmoteExecuted() = dance_asset;
            }
        }
    }
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

        // Apply customization loadout
        player_pawn->PlayerState()->ApplyCharacterCustomization(player_pawn);
        
        return player_pawn;
    }

    return nullptr;
}

std::string player_component::get_account_id(AFortPlayerController* target_player_controller)
{
    if (!memory_manager::is_bad_read_pointer(target_player_controller) && target_player_controller && !static_cast<AFortPlayerControllerAthena*>(target_player_controller)->EraPlayerInfo().bIsDisconneting)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
                
        return converter.to_bytes(static_cast<AFortPlayerControllerAthena*>(target_player_controller)->EraPlayerInfo().AccountId);
    }
    
    return "";
}

AFortPlayerController* player_component::get_player_by_account_id(std::wstring in_account_id)
{
    if (*GWorld && (*GWorld)->NetDriver())
    {
        for (UNetConnection* Connection : (*GWorld)->NetDriver()->ClientConnections())
        {
            if (Connection)
            {
                if (!memory_manager::is_bad_read_pointer(Connection->PlayerController()) && Connection->PlayerController())
                {
                    if (static_cast<AFortPlayerControllerAthena*>(Connection->PlayerController())->EraPlayerInfo().AccountId == in_account_id)
                    {
                        return Connection->PlayerController();
                    }
                }
            }
        }
    }
    
    return nullptr;   
}

void player_component::handle_player_damaged(AFortPlayerPawn* HitActor, AFortPlayerController* Instigator, AActor* DamageCauser, int Damage, FName BoneName)
{
    if (Instigator)
    {
        if (DamageCauser->IsA(AFortWeapon::StaticClass()))
        {
            FDamageDealtMapValue& Value = static_cast<AFortPlayerControllerAthena*>(Instigator)->EraPlayerInfo().DamageDealtTracker;
            FPlayerStats& PlayerStats = static_cast<AFortPlayerControllerAthena*>(Instigator)->EraPlayerInfo().PlayerStatsTracker;

            if (UFortItemDefinition* ItemDefinition = static_cast<AFortWeapon*>(DamageCauser)->WeaponData())
            {
                // Bump hits 
                if (ItemDefinition->IsA(UFortWeaponRangedItemDefinition::StaticClass()))
                {
                    // HeadShots
                    if (BoneName == FName(TEXT("HEAD")))
                    {
                        PlayerStats.HeadShots++;

                        // Track headshot damage
                        Value.HeadshotDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::Headshot);

                        if (HitActor)
                        {
                            if (HitActor->IsA(APlayerPawn_Athena_C::StaticClass()))
                            {
                                if (HitActor->GetHealth() - Damage <= 0.0)
                                {
                                    static_cast<AFortPlayerControllerAthena*>(Instigator)->EraPlayerInfo().bLastKillHeadshot = true;
                                }
                            }
                        }
                    }
                    else
                    {
                        PlayerStats.Hits++;
                    }
                }

                // Damage taken
                if (HitActor)
                {
                    if (HitActor->IsA(APlayerPawn_Athena_C::StaticClass()))
                    {
                        if (AFortPlayerController* PlayerController = HitActor->Controller())
                        {
                            static_cast<AFortPlayerControllerAthena*>(PlayerController)->EraPlayerInfo().PlayerStatsTracker.DamageTaken += Damage;

                            // If this player is carrying the jewel, count damage
                            if (static UFortItemDefinition* Athena_Blind_Pack_Definition = FindObject<UFortItemDefinition>(L"/Game/Athena/Playlists/Bling/BlingPack/Athena_Bling_Pack.Athena_Bling_Pack"))
                            {
                                if (inventory_component::get_inventory_item_by_definition(PlayerController, Athena_Blind_Pack_Definition))
                                {
                                    Value.SpecialDamage.JewelOpponent += Damage;
                                    HandleDebouncedDamage(Instigator, EDamageCauser::Jewel);
                                }
                            }
                        }
                    }
                }
                
                if (STATIC_HASPROPERTY(ItemDefinition, GameplayTags))
                {
                    switch (get_damage_causer(ItemDefinition->GameplayTags()))
                    {
                    case EDamageCauser::Pickaxe:
                        Value.PickaxeDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::Pickaxe);
                        break;
                    case EDamageCauser::Sniper:
                        Value.SniperDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::Sniper);
                        break;
                    case EDamageCauser::HeavySniper:
                        Value.HeavySniperDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::HeavySniper);
                        break;
                    case EDamageCauser::BoltSniper:
                        Value.BoltSniperDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::BoltSniper);
                        break;
                    case EDamageCauser::HuntingSniper:
                        Value.HuntingSniperDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::HuntingSniper);
                        break;
                    case EDamageCauser::HandCannonPistol:
                        Value.HandCannonPistolDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::HandCannonPistol);
                        break;
                    case EDamageCauser::Pistol:
                        {
                            if (static_cast<AFortWeapon*>(DamageCauser)->IsSuppressed())
                            {
                                Value.SuppressedPistolDamage += Damage;
                                HandleDebouncedDamage(Instigator, EDamageCauser::SuppressedPistol);
                            }
                            else
                            {
                                Value.PistolDamage += Damage;
                                HandleDebouncedDamage(Instigator, EDamageCauser::Pistol);
                            }
                            break;
                        }
                    case EDamageCauser::PumpShotgun:
                        Value.PumpShotgunDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::PumpShotgun);
                        break;
                    case EDamageCauser::TacticalShotgun:
                        Value.TacticalShotgunDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::TacticalShotgun);
                        break;
                    case EDamageCauser::HeavyShotgun:
                        Value.HeavyShotgunDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::HeavyShotgun);
                        break;
                    case EDamageCauser::Shotgun:
                        Value.ShotgunDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::Shotgun);
                        break;
                    case EDamageCauser::SuppressedAssaultRifle:
                        Value.SuppressedAssaultRifleDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::SuppressedAssaultRifle);
                        break;
                    case EDamageCauser::BurstAssaultRifle:
                        Value.BurstAssaultRifleDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::BurstAssaultRifle);
                        break;
                    case EDamageCauser::AssaultRifle:
                        Value.AssaultRifleDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::AssaultRifle);
                        break;
                    case EDamageCauser::Grenade:
                        Value.GrenadeDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::GrenadeLauncher);
                        break;
                    case EDamageCauser::SMG:
                        {
                            if (static_cast<AFortWeapon*>(DamageCauser)->IsSuppressed())
                            {
                                Value.SuppressedSMGDamage += Damage;
                                HandleDebouncedDamage(Instigator, EDamageCauser::SuppressedSMG);
                            }
                            else
                            {
                                Value.SMGDamage += Damage;
                                HandleDebouncedDamage(Instigator, EDamageCauser::SMG);
                            }
                            break;
                        }
                    case EDamageCauser::GrenadeLauncher:
                        Value.GrenadeLauncherDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::GrenadeLauncher);
                        break;
                    case EDamageCauser::LMG:
                        Value.LMGDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::LMG);
                        break;
                    case EDamageCauser::Minigun:
                        Value.MinigunDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::Minigun);
                        break;
                    case EDamageCauser::RPG:
                        Value.RPGDamage += Damage;
                        HandleDebouncedDamage(Instigator, EDamageCauser::RPG);
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
}

uint8_t player_component::convert_quick_chat_index_to_message(uint8_t Index)
{
    UEnum* ETeamMemberState = FindObject<UEnum>(TEXT("/Script/FortniteGame.ETeamMemberState"));

    if (Index == 0)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("EnemySpotted")) - 1;
    }
    if (Index == 1)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("NeedBandages")) - 1;
    }
    if (Index == 2)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("NeedShields")) - 1;
    }
    if (Index == 3)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("NeedMaterials")) - 1;
    }
    if (Index == 4)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("NeedWeapon")) - 1;
    }
    if (Index == 5)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("NeedAmmoLight")) - 1;
    }
    if (Index == 6)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("NeedAmmoMedium")) - 1;
    }
    if (Index == 7)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("NeedAmmoHeavy")) - 1;
    }
    if (Index == 8)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("NeedAmmoShells")) - 1;
    }
    if (Index == 9)
    {
        return ETeamMemberState->GetEnumIndex(TEXT("NeedAmmoRocket")) - 1;
    }

    return 0;
}

void player_component::initialize_player_banner_icon(AFortPlayerController* TargetPlayerController, std::string BannerIcon)
{
    if (BannerIcon.find("HomebaseBannerIcon:") != std::string::npos)
    {
        BannerIcon = BannerIcon.substr(BannerIcon.find(":") + 1);
        
        TO_WSTRING(BannerIcon, wBannerIcon);

        if (FindPropertyByName(FFortAthenaLoadout::GetClass(), L"BannerIconId"))
        {
            static_cast<AFortPlayerControllerAthena*>(TargetPlayerController)->CustomizationLoadout().BannerIconId() = wBannerIcon;
        }

        if (AFortPlayerStateAthena* PlayerState = TargetPlayerController->PlayerState())
        {
            if (STATIC_HASPROPERTY(PlayerState, Banner))
            {
                if (FindPropertyByName(FPlayerBannerInfo::GetClass(), L"IconId"))
                {
                    PlayerState->Banner().IconId() = wBannerIcon;
                }
            }
        }
    }
}

void player_component::initialize_player_banner_level(AFortPlayerController* TargetPlayerController, int Level)
{
    if (AFortPlayerStateAthena* PlayerState = TargetPlayerController->PlayerState())
    {
        if (STATIC_HASPROPERTY(PlayerState, Banner))
        {
            if (FindPropertyByName(FPlayerBannerInfo::GetClass(), L"Level"))
            {
                PlayerState->Banner().Level() = Level;
            }
        }
    }
}

void player_component::initialize_player_banner_color(AFortPlayerController* TargetPlayerController, std::string BannerColor)
{
    if (BannerColor.find("HomebaseBannerColor:") != std::string::npos)
    {
        BannerColor = BannerColor.substr(BannerColor.find(":") + 1);
        
        TO_WSTRING(BannerColor, wBannerColor);

        if (FindPropertyByName(FFortAthenaLoadout::GetClass(), L"BannerColorId"))
        {
            static_cast<AFortPlayerControllerAthena*>(TargetPlayerController)->CustomizationLoadout().BannerColorId() = wBannerColor;
        }
    
        if (AFortPlayerStateAthena* PlayerState = TargetPlayerController->PlayerState())
        {
            if (STATIC_HASPROPERTY(PlayerState, Banner))
            {
                if (FindPropertyByName(FPlayerBannerInfo::GetClass(), L"ColorId"))
                {
                    PlayerState->Banner().ColorId() = wBannerColor;
                }
            }
        }
    }
}

EDamageCauser player_component::get_damage_causer(FGameplayTagContainer& Tags)
{
    static std::map<FName, EDamageCauser> death_cause_map =
    {
        { L"Weapon.Ranged.Shotgun.Pump", EDamageCauser::PumpShotgun },
        { L"Weapon.Ranged.Shotgun.Tactical", EDamageCauser::TacticalShotgun },
        { L"Weapon.Ranged.Shotgun.Heavy", EDamageCauser::HeavyShotgun },
        { L"Weapon.Ranged.Shotgun", EDamageCauser::Shotgun },
        { L"weapon.ranged.assault.silenced", EDamageCauser::SuppressedAssaultRifle },
        { L"weapon.ranged.assault.burst", EDamageCauser::BurstAssaultRifle },
        { L"Weapon.Ranged.Assault", EDamageCauser::AssaultRifle },
        { L"weapon.ranged.assault.standard", EDamageCauser::AssaultRifle },
        { L"Weapon.Ranged.LMG", EDamageCauser::LMG },
        { L"Weapon.Ranged.Pistol.HandCannon", EDamageCauser::HandCannonPistol },
        { L"Weapon.Ranged.Pistol.Standard", EDamageCauser::Pistol },
        { L"Weapon.Ranged.Pistol", EDamageCauser::Pistol },
        { L"weapon.ranged.sniper.bolt", EDamageCauser::BoltSniper },
        { L"Weapon.Ranged.Sniper.Hunting", EDamageCauser::HuntingSniper },
        { L"Weapon.Ranged.Sniper.Heavy", EDamageCauser::HeavySniper },
        { L"Weapon.Ranged.Sniper", EDamageCauser::Sniper },
        { L"Weapon.Ranged.SMG", EDamageCauser::SMG },
        { L"Weapon.Ranged.Crossbow", EDamageCauser::Sniper },
        { L"Weapon.Ranged.Minigun", EDamageCauser::Minigun },
        { L"weapon.ranged.heavy.rocket_launcher", EDamageCauser::RPG},
        { L"weapon.ranged.heavy.grenade_launcher", EDamageCauser::GrenadeLauncher},
        { L"Weapon.ranged.heavy.grenade", EDamageCauser::Grenade},
        { L"Weapon.Melee.Impact.Pickaxe", EDamageCauser::Pickaxe}
    };

    for (FGameplayTag gameplay_tag : Tags.GameplayTags)
    {
        if (death_cause_map.contains(gameplay_tag.TagName))
        {
            // Return a death cause if we have it mapped
            return death_cause_map[gameplay_tag.TagName];
        }
    }

    return EDamageCauser::None;
}

void player_component::apply_variant(FFortAthenaLoadout* Loadout, nlohmann::json variants, UObject* ItemVariantIsUsedFor)
{
    for (auto& elem_variant : variants.items())
    {
        if (elem_variant.value().is_object())
        {
            if (elem_variant.value().contains("active") &&
                elem_variant.value()["active"].is_string() &&
                elem_variant.value().contains("channel") &&
                elem_variant.value()["channel"].is_string() &&
                elem_variant.value().contains("owned") &&
                elem_variant.value()["owned"].is_array())
            {
                std::string active = elem_variant.value()["active"];
                TO_WSTRING(active, wActive);
                std::string channel = elem_variant.value()["channel"];
                TO_WSTRING(channel, wChannel);
                nlohmann::json owned = elem_variant.value()["owned"];

                // Convert variant names
                wActive = std::wstring(L"Cosmetics.Variant.Property.") + wActive;
                wChannel = std::wstring(L"Cosmetics.Variant.Channel.") + wChannel;

                FMcpVariantChannelInfo* NewVariantInfo = new FMcpVariantChannelInfo;
                NewVariantInfo->ActiveVariantTag() = FName(wActive.c_str());
                NewVariantInfo->VariantChannelTag() = FName(wChannel.c_str());

                if (owned.is_array())
                {
                    for (auto& elem_owned : owned.items())
                    {
                        if (elem_owned.value().is_string())
                        {
                            std::string sOwned = elem_owned.value();
                            TO_WSTRING(sOwned, wOwned);
                            wOwned = std::wstring(L"Cosmetics.Variant.Property.") + wOwned;

                            NewVariantInfo->OwnedVariantTags().GameplayTags.Add(FGameplayTag(FName(wOwned.c_str())));

                            if (FindPropertyByName(NewVariantInfo->GetClass(), L"ItemVariantIsUsedFor"))
                            {
                                NewVariantInfo->ItemVariantIsUsedFor() = ItemVariantIsUsedFor;
                            }
                        }
                    }

                    // Add NewVariantInfo
                    if (FindPropertyByName(Loadout->GetClass(), L"CharacterVariantChannels"))
                    {
                        TArrayManager<FMcpVariantChannelInfo>(&Loadout->CharacterVariantChannels(), FMcpVariantChannelInfo::GetStructSize()).Add(NewVariantInfo);
                    }

                    free(NewVariantInfo);
                }
            }
        }
    }
}

void player_component_hooks::ServerAcknowledgePossession_Hook(APlayerController* PlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(UObject*, P);

    // Update acknowledged pawn
    PlayerController->AcknowledgedPawn() = P;
    
    return ServerAcknowledgePossession(PlayerController, StackFrame);
}

void player_component_hooks::ServerReturnToMainMenu_Hook(AFortPlayerController* FortPlayerController)
{
    // Make the player return to main menu
    FortPlayerController->ClientReturnToMainMenu(L"");   
}

void player_component_hooks::ServerChoosePart_Hook(AFortPlayerPawn* FortPlayerPawn, FFrame& StackFrame)
{
    auto config = config_manager::get_server_config();
    if (config["scp_enabled"] == true)
    {
        ServerChoosePart(FortPlayerPawn, StackFrame);
    }
}

void player_component_hooks::PostLogin_Hook(AFortGameModeAthena* FortGameModeAthena, APlayerController* NewPlayer)
{
    // Call native function
    PostLogin(FortGameModeAthena, NewPlayer);

#ifdef _RELEASE
    team_component::set_team(static_cast<AFortPlayerController*>(NewPlayer), team_component::find_era_team(static_cast<AFortPlayerController*>(NewPlayer), true)->get_team_index(), team_component::find_era_team(static_cast<AFortPlayerController*>(NewPlayer), false)->get_team_index());
#elif _TESTING
    team_component::set_team(static_cast<AFortPlayerController*>(NewPlayer), 2, 2);
#endif
    
    // Kick the player if a ban record exists
    if (moderation_component::is_player_banned(static_cast<AFortPlayerController*>(NewPlayer)))
    {
        spdlog::info(L"Player '{}' attempted to join but is banned. Kicking...", static_cast<AFortPlayerController*>(NewPlayer)->MyPlayerInfo()->PlayerName().c_str());
        static_cast<AFortPlayerController*>(NewPlayer)->ClientReturnToMainMenu(L"You have been banned from this server!");
    }

    // Grant admin permission if the player is an admin
    moderation_component::grant_admin_permission_safe(static_cast<AFortPlayerController*>(NewPlayer));

    // If livespectator, make sure the player is an admin
    if (player_component::is_live_spectator(NewPlayer))
    {
        if (NewPlayer->CheatManager() == nullptr)
        {
            static_cast<AFortPlayerController*>(NewPlayer)->ClientReturnToMainMenu(L"Only Server Admins are allowed to live spectate.");
            
            return;
        }
    }
    
    // Only proceed if this is an athena playercontroller
    if (NewPlayer->IsA(FortGameModeAthena->PlayerControllerClass()))
    {
        // Make sure the player is joining for the first time (ONLY FOR NON-ADMINS)
        if (NewPlayer->PlayerState()->NumRejoins() > 0 && NewPlayer->CheatManager() == nullptr)
        {
            spdlog::error(L"Player {} attempted to rejoin. Kicking...", static_cast<AFortPlayerController*>(NewPlayer)->MyPlayerInfo()->PlayerName().c_str());
                
            static_cast<AFortPlayerController*>(NewPlayer)->ClientReturnToMainMenu(L"You aren't allowed to rejoin. Please restart the game with a different name and try again.");

            return;
        }

        // Register quest manager
        if(STATIC_HASPROPERTY(NewPlayer, XPComponent))
        {
            static_cast<AFortPlayerController*>(NewPlayer)->XPComponent()->bRegisteredWithQuestManager() = true;
        }
            
        // Spawn QuickBars
        AFortQuickBars* new_quickbars = SpawnDefaultActor<AFortQuickBars>(*GWorld, AFortQuickBars::StaticClass());
            
        AFortQuickBars*& player_quickbars = NewPlayer->HasProperty(L"QuickBars") ?
            static_cast<AFortPlayerController*>(NewPlayer)->QuickBars() : static_cast<AFortPlayerController*>(NewPlayer)->ClientQuickBars();

        // Set quickbars
        player_quickbars = new_quickbars;
            
        // Set ownership
        player_quickbars->SetOwner(NewPlayer);
        
        // Assign team
        if (team_component::find_team(static_cast<AFortPlayerController*>(NewPlayer)) == nullptr)
        {
            spdlog::error(L"Couldn't find a valid team for player '{}'", static_cast<AFortPlayerController*>(NewPlayer)->MyPlayerInfo()->PlayerName().c_str());
        
            static_cast<AFortPlayerController*>(NewPlayer)->ClientReturnToMainMenu(L"");
        }

        // Prevent camera from shaking while spectating or when using the guided missile
        NewPlayer->PlayerCameraManager()->ViewRollMin() = 0.f;
        NewPlayer->PlayerCameraManager()->ViewRollMax() = 0.f;

        // Bind OnStartedRespawn delegate
        if (STATIC_HASPROPERTY(NewPlayer, OnStartedRespawn))
        {
            static_cast<AFortPlayerControllerAthena*>(NewPlayer)->OnStartedRespawn().Bind(TScriptDelegate(static_cast<AFortPlayerController*>(NewPlayer), L"OnPlayerStartedRespawn__DelegateSignature"));
        }

        // Update ping variable
        UKismetSystemLibrary::SetBoolPropertyByName(NewPlayer->PlayerState(), TEXT("bShouldUpdateReplicatedPing"), true);
        
        // Notify gamemode
        (*GWorld)->AuthorityGameMode()->OnPlayerJoined(NewPlayer);

        UKismetSystemLibrary::SetBoolPropertyByName(static_cast<AFortPlayerController*>(NewPlayer)->Pawn(), L"bIsInvulnerable", true);

#ifdef _RELEASE
        // Notify era services
        gameserver_client::Client->GameServer_Status.number_of_players = FortGameModeAthena->AlivePlayers().Num();
        gameserver_client::Client->SendUpdate();
#endif
        
        // Notify backend
        player_component::PostLogin(static_cast<AFortPlayerController*>(NewPlayer));

        // Above specific % of capacity, start the match in x seconds
        try
        {
            auto config = config_manager::get_server_config();
            const int player_cap_override = config["player_cap_override"].get<int>();

            // Get percentages
            const int player_cap_override_85percent = player_cap_override / 100 * 85;
            const int player_cap_override_95percent = player_cap_override / 100 * 95;
            if (FortGameModeAthena->AlivePlayers().Num() >= player_cap_override_85percent)
            {
                // Update the countdown to 20 seconds 
                FortGameModeAthena->WarmupCountdownDuration() = 20.f;
                FortGameModeAthena->WarmupEarlyCountdownDuration() = 20.f;

                // Log
                if (!loader_component::bLogged85Percent)
                {
                    loader_component::bLogged85Percent = true;
                    spdlog::info(L"Starting Aircraft in 20 seconds, 85 percent of the players joined.");
                }
            }
            if (FortGameModeAthena->AlivePlayers().Num() >= player_cap_override_95percent)
            {
                // Update the countdown to 10 seconds 
                FortGameModeAthena->WarmupCountdownDuration() = 10.f;
                FortGameModeAthena->WarmupEarlyCountdownDuration() = 10.f;

                // Log
                if (!loader_component::bLogged95Percent)
                {
                    loader_component::bLogged95Percent = true;
                    spdlog::info(L"Starting Aircraft in 10 seconds, 95 percent of the players joined.");
                }
            }
        }
        catch (nlohmann::json::exception& err)
        {
        }

        spdlog::info(L"Player '{}' joined and spawned", static_cast<AFortPlayerController*>(NewPlayer)->MyPlayerInfo()->PlayerName().c_str());
    }
}

UObject* player_component_hooks::GetHeroDefinition_Hook(UObject* Hero)
{
    static UObject* hero_definition = FindObject(L"/Game/Athena/Heroes/HID_001_Athena_Commando_F.HID_001_Athena_Commando_F", true) ?
        FindObject(L"/Game/Athena/Heroes/HID_001_Athena_Commando_F.HID_001_Athena_Commando_F", true) :
        FindObject(L"/Game/Athena/Heroes/HID_Commando_Athena_25.HID_Commando_Athena_25", true);

    if (hero_definition == nullptr)
    {
        // Failed to find hero definition
        spdlog::error(L"Failed to override hero definition for hero '{}', as specified hero definition does not exist",
            Hero->GetName().c_str());
    }

    return hero_definition;
}

void player_component_hooks::ServerPlaySprayItem_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(UAthenaSprayItemDefinition*, SprayAsset);
    
    player_component::play_emote_item(FortPlayerController, SprayAsset);
}

void player_component_hooks::ServerPlayEmoteItem_Hook(AFortPlayerController* FortPlayerController, FFrame& StackFrame)
{
    GET_PARAMETER(UAthenaDanceItemDefinition*, EmoteAsset);

    player_component::play_emote_item(FortPlayerController, EmoteAsset);
}

void player_component_hooks::ServerSendZiplineState_Hook(AFortPlayerPawn* FortPlayerPawn, FFrame& StackFrame)
{
    GET_PARAMETER(FZiplinePawnState, InZiplineState);

    // Update pawn zipline state
    memcpy(&FortPlayerPawn->ZiplineState(), &InZiplineState, FZiplinePawnState::GetStructSize());

    // Update component
    if (UpdateZiplineState)
    {
        UpdateZiplineState(FortPlayerPawn);
    }

    if (InZiplineState.bJumped())
    {
        // CREDIT: WIKTORWIKTOR12
        float ZiplineJumpDampening = FortPlayerPawn->ZiplineJumpDampening().Eval(0);
        float ZiplineJumpStrength = FortPlayerPawn->ZiplineJumpStrength().Eval(0);

        FVector Velocity = FortPlayerPawn->GetVelocity();

        FVector LaunchVector;

        if (ZiplineJumpDampening * Velocity.X >= -750.0)
        {
            LaunchVector.X = fminf(ZiplineJumpDampening * Velocity.X, 750.0);
        }
        else
        {
            LaunchVector.X = -750.0;
        }
        
        if (ZiplineJumpDampening * Velocity.Y >= -750.0)
        {
            LaunchVector.Y = fminf(ZiplineJumpDampening * Velocity.Y, 750.0);
        }
        else
        {
            LaunchVector.Y = -750.0;
        }
        
        LaunchVector.Z = ZiplineJumpStrength;

        FortPlayerPawn->LaunchCharacterJump(LaunchVector, false, false, false, true);
    }
}

void player_component_hooks::ServerReadyToStartMatch_Hook(AFortPlayerController* FortPlayerController)
{
    ServerReadyToStartMatch(FortPlayerController);
    
    if (STATIC_HASPROPERTY(FortPlayerController->PlayerState(), bHasStartedPlaying))
    {
        if (FortPlayerController->PlayerState()->bHasStartedPlaying() == false)
        {
            // Grant harvesting tool
            inventory_component::add_item(FortPlayerController, static_cast<AFortPlayerControllerAthena*>(FortPlayerController)->CustomizationLoadout().Pickaxe() ? static_cast<AFortPlayerControllerAthena*>(FortPlayerController)->CustomizationLoadout().Pickaxe()->WeaponDefinition() : FindObject<UFortItemDefinition>(L"WID_Harvest_Pickaxe_Athena_C_T01", false, true), 1);
        
            for (FItemAndCount starting_item : (*GWorld)->AuthorityGameMode()->StartingItems())
            {
                // Grant default item
                inventory_component::add_item(FortPlayerController, starting_item.Item, starting_item.Count);
            }
        
            // Equip pickaxe
            FortPlayerController->ServerExecuteInventoryItem(inventory_component::get_pickaxe(FortPlayerController)->ItemEntry().ItemGuid());
    
            creative_component::handle_player_join(FortPlayerController->NetConnection());

            UKismetSystemLibrary::SetBoolPropertyByName(FortPlayerController->PlayerState(), L"bHasStartedPlaying", true);
        }   
    }
}

void player_component_hooks::K2_OnRestartPlayer_Hook(AFortGameMode* GameMode, AFortPlayerController* NewPlayer)
{
    K2_OnRestartPlayer(GameMode, NewPlayer);
    
    // Give abilities to newly spawned pawns
    if (NewPlayer->Pawn() != nullptr)
    {
        UObject* ability_set = FindObject(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer") ?
            FindObject(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer") : FindObject(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_DefaultPlayer.GAS_DefaultPlayer");
        
        // Set Player ID (Sets teams on traps automatically)
        NewPlayer->MyPlayerInfo()->PlayerID() = NewPlayer->NetConnection()->PlayerID();
        
        auto& WorldPlayerId = NewPlayer->PlayerState()->WorldPlayerId();
        
        if (WorldPlayerId <= 0)
        {
            WorldPlayerId = player_component::global_player_id++;
            
            NewPlayer->MyPlayerInfo()->PlayerIndex() = WorldPlayerId;
        }
        NewPlayer->PlayerState()->CachedPreviousWorldPlayerId() = WorldPlayerId;
        
        ability_component::apply_ability_set(NewPlayer->Pawn(), ability_set);

        // Make sure profile is there
        while (true)
        {
            if (static_cast<AFortPlayerControllerAthena*>(NewPlayer)->EraPlayerInfo().bPendingKick)
            {
                NewPlayer->ClientReturnToMainMenu(TEXT("Profile query failed. Please report the issue to a Developer."));
                return;
            }
            
            if (static_cast<AFortPlayerControllerAthena*>(NewPlayer)->EraPlayerInfo().bProfileReady)
            {
                break;
            }
            
            Sleep(1000 / 30);
        }
        
        // Apply CPs
        NewPlayer->Pawn()->PlayerState()->ApplyCharacterCustomization(NewPlayer->Pawn());
        
        // Apply backpack, if specified
        player_component::ApplyBackpack(static_cast<AFortPlayerControllerAthena*>(NewPlayer));

        // Equip the pickaxe on landing
        if (STATIC_HASPROPERTY(NewPlayer->Pawn(), OnPawnLanded))
        {
            NewPlayer->Pawn()->OnPawnLanded().Bind(TScriptDelegate<AFortPawn>(NewPlayer->Pawn(), TEXT("OnPawnLandedImplementation")));
        }

        // Butterfly event
        if (era_mutators_component::IsActive("butterfly_begin"))
        {
            UKismetSystemLibrary::SetBoolPropertyByName(NewPlayer->Pawn(), "bIsInvulnerable", true);
        }

        // Implementable event
        (*GWorld)->AuthorityGameMode()->OnPawnFinishedSpawning(NewPlayer->Pawn());
    }
}

void player_component_hooks::ServerAddMapMarker_Hook(UAthenaMarkerComponent* MarkerComponent, FFrame& StackFrame)
{
    if (!WorldMarkerDataCtr)
    {
        spdlog::error(L"ERROR: Unable to add map marker, WorldMarkerDataCtr is invalid.");
        return;
    }
    
    GET_PARAMETER(FFortClientMarkerRequest, MarkerRequest);
    
    AFortPlayerController* PlayerController = static_cast<AFortPlayerController*>(MarkerComponent->GetOwner());

    // Construct FFortWorldMarkerData
    FFortWorldMarkerData* FortWorldMarkerData = static_cast<FFortWorldMarkerData*>(malloc(FFortWorldMarkerData::GetStructSize()));
    
    FortWorldMarkerData = WorldMarkerDataCtr(FortWorldMarkerData);

    FMarkerID MarkerID(PlayerController->PlayerState()->PlayerID(), MarkerRequest.InstanceId());
    FortWorldMarkerData->MarkerID() = MarkerID;
    FortWorldMarkerData->Owner() = PlayerController->PlayerState();
    FortWorldMarkerData->MarkerType() = MarkerRequest.MarkerType();
    FortWorldMarkerData->WorldPosition() = MarkerRequest.WorldPosition();
    FortWorldMarkerData->WorldPositionOffset() = MarkerRequest.WorldPositionOffset();
    FortWorldMarkerData->WorldNormal() = MarkerRequest.WorldNormal();

    if (MarkerRequest.MarkedActor())
    {
        FortWorldMarkerData->MarkedActor().WeakPtr = MarkerRequest.MarkedActor();
        FortWorldMarkerData->MarkedActorClass().WeakPtr = MarkerRequest.MarkedActor()->Class;

        UEnum* WorldMarkerType = FindObject<UEnum>(L"/Script/FortniteGame.EFortWorldMarkerType");
        if (FortWorldMarkerData->MarkerType() == WorldMarkerType->GetEnumIndex(L"Item"))
        {
            AFortPickup* MarkedActor = static_cast<AFortPickup*>(MarkerRequest.MarkedActor());

            if (FindPropertyByName(FFortWorldMarkerData::GetClass(), L"ItemDefinition"))
            {
                FortWorldMarkerData->ItemDefinition() = MarkedActor->PrimaryPickupItemEntry().ItemDefinition();
            }
            if (FindPropertyByName(FFortWorldMarkerData::GetClass(), L"ItemCount"))
            {
                FortWorldMarkerData->ItemCount() = MarkedActor->PrimaryPickupItemEntry().Count();
            }
        }
    }

    // Call ClientAddMarker for all the teammates
    if (auto TeamInfo = team_component::find_team(PlayerController))
    {
        for (auto TeamMember : TeamInfo->TeamMembers())
        {
            AFortPlayerControllerAthena* TeamPlayerController = static_cast<AFortPlayerControllerAthena*>(TeamMember);

            // Only do for teammates
            if (TeamPlayerController != PlayerController)
            {
                UAthenaMarkerComponent* TeamMarkerComponent = TeamPlayerController->MarkerComponent();

                if (TeamMarkerComponent)
                {
                    TeamMarkerComponent->ClientAddMarker(FortWorldMarkerData);
                }
            }
        }
    }
}

long long player_component_hooks::OnTargetDataReadyCallback_Hook(UFortGameplayAbility_RangedWeapon* RangedWeaponAbility, FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag)
{
    if (anticheat_component::ValidateTargetData(RangedWeaponAbility))
    {
        if (static bool bRun = GetEngineVersionEasy() > 422)
        {
            if (RangedWeaponAbility)
            {
                AFortWeapon* SourceWeapon = RangedWeaponAbility->CurrentWeapon();

                // Controller & pawn
                AFortPlayerPawn* Pawn = static_cast<AFortPlayerPawn*>(SourceWeapon->GetOwner());
                AFortPlayerController* Controller = Pawn->Controller();

                if (!SourceWeapon->IsProjectileWeapon())
                {
                    if (Controller)
                    {
                        TArray<uint8> HitReplaces{};
                        for (uint8 i = 0; i < InData.GetData().Num() && i < 255; ++i)
                        {
                            if (FFortGameplayAbilityTargetData_SingleTargetHit* SingleTargetHit = InData.Get(i))
                            {
                                if (SingleTargetHit->bHitReplaced())
                                {
                                    HitReplaces.Add(i);
                                }
                            }
                        }

                        Controller->ClientConfirmTargetData(InData.UniqueId, true, HitReplaces);
                    }
                }
            }
        }
    
        return OnTargetDataReadyCallback(RangedWeaponAbility, InData, ApplicationTag);
    }

    return 0;
}

void player_component_hooks::ServerPlaySquadQuickChatMessage_Hook(AFortPlayerControllerAthena* SenderPC, FFrame& StackFrame)
{
    GET_PARAMETER(FAthenaQuickChatActiveEntry, ChatEntry);
    
    if (SenderPC)
    {
        if (AFortPlayerStateAthena* PlayerState = SenderPC->PlayerState())
        {
            STATIC_HASPROPERTY(PlayerState, ReplicatedTeamMemberState);
            STATIC_HASPROPERTY(PlayerState, TeamMemberState);
            
            if (bExists_TeamMemberState &&
                bExists_ReplicatedTeamMemberState)
            {
                PlayerState->ReplicatedTeamMemberState() = player_component::convert_quick_chat_index_to_message(ChatEntry.Index());
                PlayerState->TeamMemberState() = player_component::convert_quick_chat_index_to_message(ChatEntry.Index());

                SenderPC->PlayerState()->OnRep_ReplicatedTeamMemberState();
            }
        }
        
        player_component::play_emote_item(SenderPC, FindObject<UAthenaDanceItemDefinition>(L"/Game/Athena/Items/Cosmetics/Dances/Emoji/Emoji_Comm.Emoji_Comm"), false);
    }
}

// Only use this to keep track of sniper damage, as projectiles are not taken into account by NetMulticast_Athena_BatchedDamageCues
void player_component_hooks::OnDamageServer_Hook(UObject* Self, UObject* Function, void* Params)
{
    static UFortItemDefinition* B_BGA_Athena_C4_C = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Consumables/C4/B_BGA_Athena_C4.B_BGA_Athena_C4_C");
    
    GET_PROCESS_EVENT_PARAMETER(AFortPlayerController*, InstigatedBy);   
    GET_PROCESS_EVENT_PARAMETER(float, Damage);   
    GET_PROCESS_EVENT_PARAMETER(AActor*, DamageCauser);
    GET_PROCESS_EVENT_PARAMETER(FHitResult, HitInfo);

    if (Self && InstigatedBy && DamageCauser)
    {
        AFortWeapon* FortWeapon = nullptr;
        
        if (DamageCauser->IsA(AFortWeapon::StaticClass()))
        {
            FortWeapon = static_cast<AFortWeapon*>(DamageCauser);
        }
        else if (DamageCauser->IsA(AFortProjectileAthena::StaticClass()))
        {
            if (DamageCauser->Owner()->IsA(AFortWeapon::StaticClass()))
            {
                FortWeapon = static_cast<AFortWeapon*>(DamageCauser->Owner());
            }
        }

        if (FortWeapon)
        {
            if (Self->IsA(APlayerPawn_Athena_C::StaticClass()))
            {
                player_component::handle_player_damaged(static_cast<AFortPlayerPawn*>(Self), InstigatedBy, FortWeapon, Damage, HitInfo.BoneName());
            }
        }

        if (Self->IsA(ABuildingSMActor::StaticClass()))
        {
            ABuildingSMActor* building_actor = static_cast<ABuildingSMActor*>(Self);
            if (building_actor->AttributeInitLevelSource() == 1 &&
                DamageCauser->IsA(B_BGA_Athena_C4_C))
            {
                if (InstigatedBy->PlayerState())
                {
                    // Check team
                    if (building_actor->Team() != InstigatedBy->PlayerState()->TeamIndex())
                    {
                        static_cast<AFortPlayerControllerAthena*>(InstigatedBy)->EraPlayerInfo().DamageDealtTracker.SpecialDamage.C4BuildingDamage += Damage;
                        player_component::HandleDebouncedDamage(InstigatedBy, EDamageCauser::C4);
                    }
                }
            }
        }
    }
}

void AFortPawn::SetShield(float NewShieldVal)
{
    STATIC_FINDFUNCTION(SetShield)
    STATIC_FINDFUNCTION(SetHealth)

    if (Function_SetShield == nullptr)
    {
        if (player_component::HealthProperty)
        {
            *player_component::HealthProperty = FindPropertyByName(UFortHealthSet::StaticClass(), L"CurrentShield");
            Call(Function_SetHealth, NewShieldVal);
            *player_component::HealthProperty = FindPropertyByName(UFortHealthSet::StaticClass(), L"Health");
        }
    }
    else
    {
        Call(Function_SetShield, NewShieldVal);
    }
}

void player_component_hooks::AFortPlayerStateAthena_PreReplication_Hook(AFortPlayerStateAthena* Self, void*** ChangedPropertyTracker)
{
    // Just a test
    //OVERRIDE_PROPERTY_REPLICATION(CharacterParts, false)
    
    AFortPlayerStateAthena_PreReplication(Self, ChangedPropertyTracker);
}

void player_component_hooks::ServerAwardVehicleTrickPoints_Hook(AFortPlayerController* Self, FFrame& StackFrame)
{
    GET_PARAMETER(int, InPoints)

    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"Player {} has been awarded {} trick points", Self->GetName().c_str(), InPoints);
    }
        
    static_cast<AFortPlayerControllerAthena*>(Self)->EraPlayerInfo().PlayerStatsTracker.TemporaryTrickPoints += InPoints;
    if (UKismetSystemLibrary::K2_IsTimerActive(Self, L"TrickPointsCallback"))
    {
        UKismetSystemLibrary::K2_ClearTimer(Self, L"TrickPointsCallback");
    }
    UKismetSystemLibrary::K2_SetTimer(Self, L"TrickPointsCallback", 5.0, false);
    
    ServerAwardVehicleTrickPoints(Self, StackFrame);
}

void player_component_hooks::ReceiveActorEndOverlap_Hook(UObject* Self, UObject* Function, void* Params)
{
    GET_PROCESS_EVENT_PARAMETER(AActor*, OtherActor)

    if (Self && OtherActor)
    {
        if (Self->IsA(APlayerPawn_Athena_C::StaticClass()))
        {
            if (OtherActor->IsA(ABP_ShootingGallery_01_C::StaticClass()))
            {
                if (log_component::enable_debug_logs)
                {
                    spdlog::info(L"player_component_hooks::ReceiveActorEndOverlap_Hook: Pawn finished overlapping with shooting gallery. Score: {} {}.", static_cast<ABP_ShootingGallery_01_C*>(OtherActor)->PopInt()[0], static_cast<ABP_ShootingGallery_01_C*>(OtherActor)->PopInt()[1]);
                }
            }
        }
    }  
}

void player_component_hooks::GameplayCue_Hook(UObject* Self, UObject* Function, void* Params)
{
    GET_PROCESS_EVENT_PARAMETER(FGameplayCueParameters, Parameters)

    if (Self)
    {
        AFortPlayerController* PlayerController = static_cast<AFortPawn*>(Self)->Controller();
        FPlayerStats& PlayerStats = static_cast<AFortPlayerControllerAthena*>(PlayerController)->EraPlayerInfo().PlayerStatsTracker;

        if (PlayerController)
        {
            if (Parameters.AggregatedSourceTags().GameplayTags.Find(FGameplayTag(L"Item.Trap.CozyCampfire")))
            {
                PlayerStats.TemporaryCampfireHealth += 2;
                if (UKismetSystemLibrary::K2_IsTimerActive(PlayerController, L"CampfireHealthCallback"))
                {
                    UKismetSystemLibrary::K2_ClearTimer(PlayerController, L"CampfireHealthCallback");
                }
                UKismetSystemLibrary::K2_SetTimer(PlayerController, L"CampfireHealthCallback", 5.0, false);
            }
            else if (Parameters.AggregatedSourceTags().GameplayTags.Find(FGameplayTag(L"Trait.Restoration.Shields")))
            {
                if (Parameters.AggregatedSourceTags().GameplayTags.Find(FGameplayTag(L"Item.Consumable.SmallShieldPotion")))
                {
                    PlayerStats.TemporaryObtainedShield += 25;
                    if (UKismetSystemLibrary::K2_IsTimerActive(PlayerController, L"ShieldCallback"))
                    {
                        UKismetSystemLibrary::K2_ClearTimer(PlayerController, L"ShieldCallback");
                    }
                    UKismetSystemLibrary::K2_SetTimer(PlayerController, L"ShieldCallback", 5.0, false);
                }
                else if (Parameters.AggregatedSourceTags().GameplayTags.Find(FGameplayTag(L"Item.Consumable.ShieldPotion")))
                {
                    PlayerStats.TemporaryObtainedShield += 50;
                    if (UKismetSystemLibrary::K2_IsTimerActive(PlayerController, L"ShieldCallback"))
                    {
                        UKismetSystemLibrary::K2_ClearTimer(PlayerController, L"ShieldCallback");
                    }
                    UKismetSystemLibrary::K2_SetTimer(PlayerController, L"ShieldCallback", 5.0, false);
                }
                else if (Parameters.AggregatedSourceTags().GameplayTags.Find(FGameplayTag(L"Item.Consumable.SlurpJuice")))
                {
                    PlayerStats.TemporaryObtainedShield += 1;
                    if (UKismetSystemLibrary::K2_IsTimerActive(PlayerController, L"ShieldCallback"))
                    {
                        UKismetSystemLibrary::K2_ClearTimer(PlayerController, L"ShieldCallback");
                    }
                    UKismetSystemLibrary::K2_SetTimer(PlayerController, L"ShieldCallback", 5.0, false);
                }
            }
        }
    }
}

void player_component_hooks::GiveItemToPlayer_Hook(TScriptInterface<AFortPlayerController>* Interface, UFortItemDefinition* ItemDefinition, int Quantity)
{
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"player_component_hooks::GiveItemToPlayer_Hook: Granting {} {} to {}", ItemDefinition->GetName().c_str(), Quantity, (*Interface)->GetName().c_str());
    }
    
    inventory_component::add_item(static_cast<AFortPlayerController*>(Interface->GetObjectRef()), ItemDefinition, Quantity);
}

void player_component_hooks::PushAnimBPOverride_Hook(AFortPlayerPawn* Self, int Priority, UObject* AnimBPOverride)
{
    if (AnimBPOverride)
    {
        if (log_component::enable_debug_logs)
        {
            spdlog::info(TEXT("player_component_hooks::PushAnimBPOverride_Hook: Setting AnimBPOverride to '{}'."), AnimBPOverride->GetName().c_str());
        }
    
        Self->AnimBPOverride() = AnimBPOverride;
        Self->OnRep_AnimBPOverride();
    }
}

void player_component_hooks::PopAnimBPOverride_Hook(AFortPlayerPawn* Self, int Priority, UObject* AnimBPOverride)
{
    if (AnimBPOverride && Self->AnimBPOverride() == AnimBPOverride)
    {
        if (log_component::enable_debug_logs)
        {
            spdlog::info(TEXT("player_component_hooks::PopAnimBPOverride_Hook: Setting AnimBPOverride to nullptr."));
        }
    
        Self->AnimBPOverride() = nullptr;
        Self->OnRep_AnimBPOverride();
    }
}