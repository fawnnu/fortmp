#include "include.hpp"
#include "challenges_component.hpp"

#include "creative_component.hpp"
#include "function_component.hpp"
#include "log_component.hpp"
#include "looting_component.hpp"
#include "player_component.hpp"
#include "team_component.hpp"

challenges_component::challenges_component()
{
    QuestToInteractionMatch = {
        {FindObject<UFortQuestItemDefinition>(L"/Game/Athena/Items/Quests/BattlePass/Quest_BR_PianoChain_Play_Piano1.Quest_BR_PianoChain_Play_Piano1"), "interaction:piano"},
        {FindObject<UFortQuestItemDefinition>(L"/Game/Athena/Items/Quests/BattlePass/Quest_BR_Dance_ScavengerHunt_DanceOff_Abandoned_Mansion.Quest_BR_Dance_ScavengerHunt_DanceOff_Abandoned_Mansion"), "interaction:dance-off-competition"}
    };

    DestroyTagsToInteractionMatch = {
        { {FGameplayTag(L"Athena.Quests.Building.Chair")}, "interaction:chair" },
        { {FGameplayTag(L"Athena.Quests.Building.TelephonePoles")}, "interaction:pole" },
        { {FGameplayTag(L"Athena.Quests.Building.Wood.Palettes")}, "interaction:pallet" }
    };
}

void challenges_component::Initialize()
{
    // load object
    if (GetEngineVersionEasy() == 420)
    {
        FindObject(L"/Game/Athena/Prototype/SoccerGame/Athena_SoccerGame.Athena_SoccerGame_C", true);

        REGISTER_PROCESS_EVENT_DETOUR(L"/Game/Athena/Items/QuestInteractables/DancePlatforms/BA_QuestInteractable_DancePlatforms.BA_QuestInteractable_DancePlatforms_C.Rising__FinishedFunc", challenges_component_hooks::Rising__FinishedFunc_Hook);
        REGISTER_POST_PROCESS_EVENT_DETOUR(L"/Game/Athena/Prototype/SoccerGame/Athena_SoccerGame.Athena_SoccerGame_C.BndEvt__Goal_02_Overlap_K2Node_ComponentBoundEvent_0_ComponentBeginOverlapSignature__DelegateSignature", challenges_component_hooks::BndEvt__Goal_02_Overlap_K2Node_ComponentBoundEvent_0_ComponentBeginOverlapSignature__DelegateSignature_Hook);
        REGISTER_POST_PROCESS_EVENT_DETOUR(L"/Game/Athena/Prototype/SoccerGame/Athena_SoccerGame.Athena_SoccerGame_C.BndEvt__Goal_01_Overlap_K2Node_ComponentBoundEvent_1_ComponentBeginOverlapSignature__DelegateSignature", challenges_component_hooks::BndEvt__Goal_01_Overlap_K2Node_ComponentBoundEvent_1_ComponentBeginOverlapSignature__DelegateSignature_Hook);
    }
    if (GetEngineVersionEasy() == 421)
    {
        FindObject(L"/Game/Abilities/Toys/Basketball/Minigame/B_BasketballHoopWithMinigame.B_BasketballHoopWithMinigame_C", true);
        FindObject(L"/Game/Athena/Items/QuestInteractables/ClayPidgeons/Prop_Quest_ClayPidgeonLauncher.Prop_Quest_ClayPidgeonLauncher_C", true);
        FindObject(L"/Game/Abilities/Toys/Golf/Minigame/B_GolfMinigame_Hole.B_GolfMinigame_Hole_C", true);

        REGISTER_POST_PROCESS_EVENT_DETOUR(L"/Game/Abilities/Toys/Golf/Minigame/B_GolfMinigame_Hole.B_GolfMinigame_Hole_C.BndEvt__HoleCollision_K2Node_ComponentBoundEvent_0_ComponentBeginOverlapSignature__DelegateSignature", challenges_component_hooks::BndEvt__HoleCollision_K2Node_ComponentBoundEvent_0_ComponentBeginOverlapSignature__DelegateSignature_Hook);
        REGISTER_POST_PROCESS_EVENT_DETOUR(L"/Game/Abilities/Toys/Basketball/Minigame/B_BasketballHoopWithMinigame.B_BasketballHoopWithMinigame_C.BndEvt__HoopTriggerBox_K2Node_ComponentBoundEvent_0_ComponentBeginOverlapSignature__DelegateSignature", challenges_component_hooks::BndEvt__HoopTriggerBox_K2Node_ComponentBoundEvent_0_ComponentBeginOverlapSignature__DelegateSignature_Hook);   
        REGISTER_POST_PROCESS_EVENT_DETOUR(L"/Game/Athena/Items/QuestInteractables/ClayPidgeons/Prop_Quest_ClayPidgeonLauncher.Prop_Quest_ClayPidgeonLauncher_C.PidgeonShot", challenges_component_hooks::PidgeonShot_Hook);   
    }
    if (GetEngineVersionEasy() == 422)
    {
        FindObject(L"/Game/Abilities/Toys/IcePuck/Shared/B_Toy_IcePuck_Base.B_Toy_IcePuck_Base_C", true);

        REGISTER_POST_PROCESS_EVENT_DETOUR(L"/Game/Abilities/Toys/IcePuck/Shared/B_Toy_IcePuck_Base.B_Toy_IcePuck_Base_C.KillPuck", challenges_component_hooks::KillPuck_Hook);
    }

    {
        if (auto Function = FindFunctionByName(FindObject(L"/Game/Athena/PlayerPawn_Athena_Generic.PlayerPawn_Athena_Generic_C"), L"GameplayCue.Shield.PotionConsumed", 1))
        {
            hook_manager::hooked_functions[Function] = std::function(&player_component_hooks::GameplayCue_Hook);
        }
    }

    // register hook
    if (UObject* Property = FindObject(L"/Game/Athena/Items/QuestInteractables/AthenaQuestFunctionLibrary.AthenaQuestFunctionLibrary_C.Athena_GiveQuestUpdateToPlayers.PlayerControllersForUpdate"))
    {
        *reinterpret_cast<uint64_t*>(reinterpret_cast<__int64>(Property) + CoreOffsets::PropertyFlags) &= ~0x0000000000000100;
    }
    REGISTER_SCRIPT_FUNCTION_HOOK(L"/Game/Athena/Items/QuestInteractables/AthenaQuestFunctionLibrary.AthenaQuestFunctionLibrary_C.Athena_GiveQuestUpdateToPlayers", challenges_component_hooks::Athena_GiveQuestUpdateToPlayers_Hook, true);
}

void challenges_component::SendChestEvent(AFortPlayerController* InteractingController, ABuildingContainer* TargetContainer)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && TargetContainer)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            std::map<std::string, std::string> parameters{
                {"x", std::to_string(TargetContainer->K2_GetActorLocation().X)},
                {"y", std::to_string(TargetContainer->K2_GetActorLocation().Y)}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "event:chest", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }
}

void challenges_component::SendSupplyDropEvent(AFortPlayerController* InteractingController, AFortAthenaSupplyDrop* SupplyDrop)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && SupplyDrop)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            std::map<std::string, std::string> parameters{
                {"x", std::to_string(SupplyDrop->K2_GetActorLocation().X)},
                {"y", std::to_string(SupplyDrop->K2_GetActorLocation().Y)}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "event:supply", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }
}

void challenges_component::SendAmmoEvent(AFortPlayerController* InteractingController, ABuildingContainer* TargetContainer)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && TargetContainer)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            std::map<std::string, std::string> parameters{
                {"x", std::to_string(TargetContainer->K2_GetActorLocation().X)},
                {"y", std::to_string(TargetContainer->K2_GetActorLocation().Y)}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "event:ammo", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }
}

void challenges_component::SendLandingEvent(AFortPlayerController* OwnerPC, AFortPlayerPawn* Self)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (OwnerPC && Self)
    {
        std::string account_id = player_component::get_account_id(OwnerPC);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters {
                {"x", Self->K2_GetActorLocation().X},
                {"y", Self->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "event:landing", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }
}

void challenges_component::SendLlamaEvent(AFortPlayerController* InteractingController, AFortAthenaSupplyDrop* Llama)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && Llama)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            std::map<std::string, std::string> parameters{
                {"x", std::to_string(Llama->K2_GetActorLocation().X)},
                {"y", std::to_string(Llama->K2_GetActorLocation().Y)}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "event:llama", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }
}

void challenges_component::SendSummary(AFortPlayerController* PlayerController, int Placement)
{
    CHECK_PAST_WARMUP()
    
    if (PlayerController)
    {
        std::string account_id = player_component::get_account_id(PlayerController);
        FDamageDealtMapValue DamageDealt = static_cast<AFortPlayerControllerAthena*>(PlayerController)->EraPlayerInfo().DamageDealtTracker;

        if (account_id.size() > 0)
        {
            // Get account ids of alive team members
            std::vector<UNetConnection*> ClientConnections = (*GWorld)->NetDriver()->ClientConnections().ToVector();
            std::vector<std::string> team_users;
            if (AFortTeamInfo* TeamInfo = team_component::find_team(PlayerController))
            {
                for (AController* TeamMember : TeamInfo->TeamMembers())
                {
                    if (TeamMember && TeamMember != PlayerController)
                    {
                        if (std::find_if(ClientConnections.begin(), ClientConnections.end(), [&TeamMember](const UNetConnection* x){ return TeamMember->NetConnection() == x; }) != ClientConnections.end())
                        {
                            if (std::string team_user_account_id = player_component::get_account_id(static_cast<AFortPlayerController*>(TeamMember)); account_id.length() > 0)
                            {
                                team_users.push_back(team_user_account_id);
                            }
                        }
                    }
                }
            }
            
            nlohmann::json parameters = {
                {"placement", Placement},
                {"team_users", team_users}
            };

            gameserver_client::WaitForReply(gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "summary", parameters), HandleSummaryReply);

            // Log
            if (!memory_manager::is_bad_read_pointer(PlayerController->MyPlayerInfo()) && PlayerController->MyPlayerInfo())
            {
                std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
                std::wstring wAccount_id = converter.from_bytes(account_id);

                spdlog::info(L"Sent summary event for {} [ACCOUNT_ID: {}, PLACEMENT: {}, DAMAGEDEALT: {}]", PlayerController->MyPlayerInfo()->PlayerName().c_str(), wAccount_id.c_str(), Placement, DamageDealt.GetPlayerDamage());
            }
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }
}

void challenges_component::SendElimination(AFortPlayerController* KillerPlayerController, AFortPlayerController* VictimPlayerController, uint8_t DeathCause, FGameplayTagContainer GameplayTags, std::string rarity)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (KillerPlayerController && VictimPlayerController)
    {
        if (!memory_manager::is_bad_read_pointer(VictimPlayerController->Pawn()) && VictimPlayerController->Pawn())
        {
            std::string killer_account_id = player_component::get_account_id(KillerPlayerController);
            std::string victim_account_id = player_component::get_account_id(VictimPlayerController);

            if (killer_account_id.size() > 0 && victim_account_id.size() > 0)
            {
                std::string weapon;
                static UEnum* death_cause_enum = FindObject<UEnum>(L"/Script/FortniteGame.EDeathCause");

                if (DeathCause > 0)
                {
                    if (DeathCause == death_cause_enum->GetEnumIndex(L"Pistol"))
                    {
                        weapon = "pistol";

                        if (GameplayTags.GameplayTags.Find(FGameplayTag(L"Weapon.Ranged.Pistol.HandCannon")))
                        {
                            weapon = "hand_cannon";
                        }
                        else if (GameplayTags.GameplayTags.Find(FGameplayTag(L"Weapon.Meta.Suppressed")))
                        {
                            weapon = "suppressed_pistol";
                        }
                    }
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"Shotgun"))
                    {
                        weapon = "shotgun";

                        if (GameplayTags.GameplayTags.Find(FGameplayTag("Weapon.Ranged.Shotgun.Heavy")))
                        {
                            weapon = "heavy_shotgun";
                        }
                        else if (GameplayTags.GameplayTags.Find(FGameplayTag("Weapon.Ranged.Shotgun.Tactical")))
                        {
                            weapon = "tactical_shotgun";
                        }
                        else if (GameplayTags.GameplayTags.Find(FGameplayTag("Weapon.Ranged.Shotgun.Pump")))
                        {
                            weapon = "pump_shotgun";
                        }
                    }
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"Rifle"))
                    {
                        weapon = "assault_rifle";

                        if (GameplayTags.GameplayTags.Find(FGameplayTag("Weapon.Ranged.Assault.Burst")))
                        {
                            weapon = "burst_assault_rifle";
                        }
                        else if (GameplayTags.GameplayTags.Find(FGameplayTag("Weapon.Ranged.Assault.Silenced")))
                        {
                            weapon = "silenced_assault_rifle";
                        }
                    }
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"Sniper") ||
                             DeathCause == death_cause_enum->GetEnumIndex(L"SniperNoScope") )
                    {
                        weapon = "sniper";

                        if (GameplayTags.GameplayTags.Find(FGameplayTag("Weapon.Ranged.Sniper.Hunting")))
                        {
                            weapon = "hunting_rifle";
                        }
                        else if (GameplayTags.GameplayTags.Find(FGameplayTag("Weapon.Ranged.Sniper.Bolt")))
                        {
                            weapon = "bolt_action_sniper_rifle";
                        }
                        else if (GameplayTags.GameplayTags.Find(FGameplayTag("Weapon.Ranged.Sniper.Heavy")))
                        {
                            weapon = "heavy_sniper_rifle";
                        }
                    }
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"Grenade"))
                    {
                        weapon = "grenade";
                    }
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"SMG"))
                    {
                        weapon = "smg";

                        if (GameplayTags.GameplayTags.Find(FGameplayTag(L"Weapon.Meta.Suppressed")))
                        {
                            weapon = "suppressed_smg";
                        }
                    }
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"LMG"))
                    {
                        weapon = "lmg";
                    }
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"Minigun"))
                    {
                        weapon = "minigun";
                    }
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"GrenadeLauncher"))
                    {
                        weapon = "grenade_launcher";
                    }
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"RocketLauncher"))
                    {
                        weapon = "rpg";
                    }
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"Trap"))
                    {
                        weapon = "trap";
                    }
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"GasGrenade"))
                    {
                        weapon = "stink_bomb";
                    }
                }

                nlohmann::json parameters {
                    {"x", VictimPlayerController->Pawn()->K2_GetActorLocation().X},
                    {"y", VictimPlayerController->Pawn()->K2_GetActorLocation().Y},
                    {"weapon", weapon},
                    {"victim", victim_account_id},
                    {"in_vehicle", KillerPlayerController->Pawn()->HasFunction(L"IsInVehicle") ? KillerPlayerController->Pawn()->IsInVehicle() : false},
                    {"headshot", static_cast<AFortPlayerControllerAthena*>(KillerPlayerController)->EraPlayerInfo().bLastKillHeadshot},
                    {"distance", (VictimPlayerController->Pawn()->GetDistanceTo(KillerPlayerController->Pawn()) / 100)}
                };

                static std::vector<std::string> POITagsForCorruptedZone = {
                    "Athena.Location.Meta.SevenSeals.01",
                    "Athena.Location.Meta.SevenSeals.02",
                    "Athena.Location.Meta.SevenSeals.03",
                    "Athena.Location.Meta.SevenSeals.04",
                    "Athena.Location.Meta.SevenSeals.05",
                    "Athena.Location.Meta.SevenSeals.06",
                    "Athena.Location.Meta.SevenSeals.07",
                };

                static std::vector<std::string> POITagsForExpeditionOutposts = {
                    "Athena.Quests.Location.ExplorerOutposts.A",
                    "Athena.Quests.Location.ExplorerOutposts.B",
                    "Athena.Quests.Location.ExplorerOutposts.C",
                    "Athena.Quests.Location.ExplorerOutposts.D",
                    "Athena.Quests.Location.ExplorerOutposts.E",
                    "Athena.Quests.Location.ExplorerOutposts.F",
                    "Athena.Quests.Location.ExplorerOutposts.G"
                };

                // Read player POI
                std::vector POIList = static_cast<AFortPlayerControllerAthena*>(KillerPlayerController)->EraPlayerInfo().CurrentPOIList;
                for (std::string POITag : POITagsForCorruptedZone)
                {
                    TO_WSTRING(POITag, wPOITag);
                    FGameplayTag POIGameplayTag(wPOITag.c_str());
                    if (std::find(POIList.begin(), POIList.end(), POIGameplayTag) != POIList.end())
                    {
                        // Found one of the POI tags: break and add to elimination payload
                        parameters["meta"]["in_corrupted_area"] = true;
                        
                        break;
                    }
                }
                for (std::string POITag : POITagsForExpeditionOutposts)
                {
                    TO_WSTRING(POITag, wPOITag);
                    FGameplayTag POIGameplayTag(wPOITag.c_str());
                    if (std::find(POIList.begin(), POIList.end(), POIGameplayTag) != POIList.end())
                    {
                        // Found one of the POI tags: break and add to elimination payload
                        parameters["meta"]["in_expedition_outpost"] = true;
                        
                        break;
                    }
                }

                if (rarity.size() > 0)
                {
                    parameters["meta"]["weapon_rarity"] = rarity;
                }

                // Invalidate headshot kill record
                static_cast<AFortPlayerControllerAthena*>(KillerPlayerController)->EraPlayerInfo().bLastKillHeadshot = false;
            
                gameserver_client::Client->SendEventToChannel("player:v2:" + killer_account_id, "event:elimination", parameters);
            }
            else
            {
                if (log_component::enable_debug_logs)
                {
                    spdlog::error(L"Invalid account_id while sending event.");
                }
            }
        }
    }
}

void challenges_component::SendRevive(AFortPlayerController* ExecutingPlayerController, AFortPlayerController* VictimPlayerController)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (ExecutingPlayerController && VictimPlayerController)
    {
        const std::string executing_account_id = player_component::get_account_id(ExecutingPlayerController);
        const std::string victim_account_id = player_component::get_account_id(VictimPlayerController);

        if (executing_account_id.size() > 0 &&
            victim_account_id.size() > 0)
        {
            if (!memory_manager::is_bad_read_pointer(ExecutingPlayerController->Pawn()) && ExecutingPlayerController->Pawn())
            {
                nlohmann::json parameters{
                    {"x", ExecutingPlayerController->Pawn()->K2_GetActorLocation().X},
                    {"y", ExecutingPlayerController->Pawn()->K2_GetActorLocation().Y},
                    {"victim", victim_account_id},
                    {"knocked_by_opponent", VictimPlayerController->PlayerState()->DeathInfo().FinisherOrDowner() != VictimPlayerController->PlayerState() && VictimPlayerController->PlayerState()->DeathInfo().FinisherOrDowner() != nullptr}
                };
            
                gameserver_client::Client->SendEventToChannel("player:v2:" + executing_account_id, "event:revive", parameters);
            }
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }
}

void challenges_component::SendLetterInteraction(AFortPlayerController* InteractingController, AActor* Letter, FName Objective)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && Letter)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        // Convert FName to std::string
        std::string sObjective = Objective.sToString();

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", Letter->K2_GetActorLocation().X},
                {"y", Letter->K2_GetActorLocation().Y},
                {"objective", sObjective}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:letter", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendGnomeInteraction(AFortPlayerController* InteractingController, AProp_Quest_GniceGnomes_Parent_C* Gnome, FName Objective)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && Gnome)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        // Convert FName to std::string
        std::string sObjective = Objective.sToString();

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", Gnome->K2_GetActorLocation().X},
                {"y", Gnome->K2_GetActorLocation().Y},
                {"objective", sObjective}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:chilly-gnome", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendBattleStarInteraction(AFortPlayerController* InteractingController, AProp_QuestObject_Interactable_Parent_C* BattleStar, FName Objective)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && BattleStar)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        // Convert FName to std::string
        std::string sObjective = Objective.sToString();

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", BattleStar->K2_GetActorLocation().X},
                {"y", BattleStar->K2_GetActorLocation().Y},
                {"objective", sObjective}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:star", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendBattleStarInteraction(AFortPlayerController* InteractingController, AProp_QuestInteractable_HiddenStar_Parent_C* BattleStar, FName Objective)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && BattleStar)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        // Convert FName to std::string
        std::string sObjective = Objective.sToString();

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", BattleStar->K2_GetActorLocation().X},
                {"y", BattleStar->K2_GetActorLocation().Y},
                {"objective", sObjective}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:star", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendPortAFortInteraction(AFortPlayerController* InteractingController)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingController->Pawn()->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:port-a-fort", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendHopRockInteraction(AFortPlayerController* InteractingController, ACBGA_LowGravity_UsingJump_C* HopRock)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && HopRock)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                    {"x", HopRock->K2_GetActorLocation().X},
                    {"y", HopRock->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:hop-rock", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendAppleInteraction(AFortPlayerController* InteractingController, ACBGA_HealthSmall_C* Apple)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && Apple)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                    {"x", Apple->K2_GetActorLocation().X},
                    {"y", Apple->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:apple", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendMushroomInteraction(AFortPlayerController* InteractingController, ACBGA_ShieldsSmall_C* Mushroom)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && Mushroom)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                    {"x", Mushroom->K2_GetActorLocation().X},
                    {"y", Mushroom->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:mushroom", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendStormCenterInteraction(AFortPlayerController* InteractingController, ABA_QuestInteractable_StormCenter_Single* StormCenter)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && StormCenter)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", StormCenter->K2_GetActorLocation().X},
                {"y", StormCenter->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:storm-center", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendLocationInteraction(AFortPlayerController* InteractingController, AProp_MountainSummit_QuestInteractable_Parent* LocationProp, FName Objective)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && LocationProp)
    {
        std::string account_id = player_component::get_account_id(InteractingController);
        
        // Convert FName to std::string
        std::string sObjective = Objective.sToString();

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", LocationProp->K2_GetActorLocation().X},
                {"y", LocationProp->K2_GetActorLocation().Y},
                {"objective", sObjective}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:location", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendSkyDiveRingInteraction(AFortPlayerController* InteractingController, AProp_SkydiveRings_QuestInteractable* SkydiveRing)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && SkydiveRing)
    {
        std::string account_id = player_component::get_account_id(InteractingController);
        
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", SkydiveRing->K2_GetActorLocation().X},
                {"y", SkydiveRing->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:ring", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }  
}

void challenges_component::SendGoldenRingBiplaneInteraction(AFortPlayerController* InteractingController, Afortmp_golden_ring_quest* SkydiveRing, FName Objective)
{
    CHECK_PAST_WARMUP()

    if (InteractingController && SkydiveRing)
    {
        std::string account_id = player_component::get_account_id(InteractingController);
        
        if (account_id.size() > 0)
        {
            // Convert FName to std::string
            std::string sObjective = Objective.sToString();

            nlohmann::json parameters{
                                {"x", SkydiveRing->K2_GetActorLocation().X},
                                {"y", SkydiveRing->K2_GetActorLocation().Y},
                                {"objective", sObjective}
            };
            
                
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:plane-rings", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    } 
}

void challenges_component::SendGooseEggInteraction(AFortPlayerController* InteractingController, AProp_FMP_GooseEgg_Interactable* GooseEgg, FName Objective)
{
    CHECK_PAST_WARMUP()
    
    if (InteractingController && GooseEgg)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        // Convert FName to std::string
        std::string sObjective = Objective.sToString();

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                        {"x", GooseEgg->K2_GetActorLocation().X},
                        {"y", GooseEgg->K2_GetActorLocation().Y},
                        {"objective", sObjective}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:goose-egg", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendLaunchPadInteraction(AFortPlayerController* InteractingController, ATrap_Floor_Player_Launch_Pad_C* LaunchPad)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && LaunchPad)
    {
        std::string account_id = player_component::get_account_id(InteractingController);
        
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", LaunchPad->K2_GetActorLocation().X},
                {"y", LaunchPad->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:launchpad", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }  
}

void challenges_component::SendDanceInteraction(AFortPlayerController* InteractingController)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingController->Pawn()->K2_GetActorLocation().Y},
                {"z", InteractingController->Pawn()->K2_GetActorLocation().Z}
            };

            static std::map<std::string, std::string> POITagToObjectivesMap = {
                {"Athena.Location.Quest.Dance.Forbidden.01", "battlepass_dance_athena_location_forbidden_01"},
                {"Athena.Location.Quest.Dance.Forbidden.02", "battlepass_dance_athena_location_forbidden_02"},
                {"Athena.Location.Quest.Dance.Forbidden.03", "battlepass_dance_athena_location_forbidden_03"},
                {"Athena.Location.Quest.Dance.Forbidden.04", "battlepass_dance_athena_location_forbidden_04"},
                {"Athena.Location.Quest.Dance.Forbidden.05", "battlepass_dance_athena_location_forbidden_05"},
                {"Athena.Location.Quest.Dance.Forbidden.06", "battlepass_dance_athena_location_forbidden_06"},
                {"Athena.Location.Quest.Dance.Forbidden.07", "battlepass_dance_athena_location_forbidden_07"},
                {"Athena.Location.Quest.Dance.Forbidden.08", "battlepass_dance_athena_location_forbidden_08"},
                {"Athena.Location.Quest.Dance.Forbidden.09", "battlepass_dance_athena_location_forbidden_09"},
                {"Athena.Location.Quest.Dance.Forbidden.10", "battlepass_dance_athena_location_forbidden_10"},
                {"Athena.Location.Quest.Dance.Forbidden.11", "battlepass_dance_athena_location_forbidden_11"},
                {"Athena.Location.Quest.Dance.Forbidden.12", "battlepass_dance_athena_location_forbidden_12"},
                {"Athena.Location.Quest.Dance.Forbidden.13", "battlepass_dance_athena_location_forbidden_13"},
                {"Athena.Location.Quest.Dance.Forbidden.14", "battlepass_dance_athena_location_forbidden_14"},
                {"Athena.Location.Quest.Dance.Forbidden.15", "battlepass_dance_athena_location_forbidden_15"},
                {"Athena.Quests.DanceLocation.Generic.01", "battlepass_dance_chain_athena_scavengerhunt_locations_a"},
                {"Athena.Quests.DanceLocation.Generic.02", "battlepass_dance_chain_athena_scavengerhunt_locations_b"},
                {"Athena.Quests.DanceLocation.Generic.03", "battlepass_dance_chain_athena_scavengerhunt_locations_c"},
                {"Athena.Quests.DanceLocation.Generic.11", "battlepass_dance_chain_athena_scavengerhunt_locations_02_a"},
                {"Athena.Quests.DanceLocation.Generic.12", "battlepass_dance_chain_athena_scavengerhunt_locations_02_b"},
                {"Athena.Quests.DanceLocation.Generic.13", "battlepass_dance_chain_athena_scavengerhunt_locations_02_c"},
                {"EraEvent.FortmasDanceArea.01", "FDF_dance_athena_christmas_tree_01"}
            };

            for (FGameplayTag Tag : static_cast<AFortPlayerControllerAthena*>(InteractingController)->EraPlayerInfo().CurrentPOIList)
            {
                if (POITagToObjectivesMap.contains(Tag.TagName.sToString()))
                {
                    std::string Objective = POITagToObjectivesMap[Tag.TagName.sToString()];

                    // Add objective key!
                    parameters["objective"] = Objective;
                }
            }
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:dance", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendCandyCaneVisitInteraction(AFortPlayerController* InteractingController, FName CurrentEraPOI)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingController->Pawn()->K2_GetActorLocation().Y},
                {"z", InteractingController->Pawn()->K2_GetActorLocation().Z}
            };

            static std::map<std::string, std::string> POITagToObjectivesMap = {
                {"Era.CandyCane.01", "FDF_visit_athena_giant_candy_cane_01"},
                {"Era.CandyCane.02", "FDF_visit_athena_giant_candy_cane_02"},
                {"Era.CandyCane.03", "FDF_visit_athena_giant_candy_cane_03"},
                {"Era.CandyCane.04", "FDF_visit_athena_giant_candy_cane_04"}
            };

            if (POITagToObjectivesMap.contains(CurrentEraPOI.sToString()))
            {
                std::string Objective = POITagToObjectivesMap[CurrentEraPOI.sToString()];

                // Add objective key!
                parameters["objective"] = Objective;
            }
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:candy-cane", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendSprayInteraction(AFortPlayerController* InteractingController)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                    {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                    {"y", InteractingController->Pawn()->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:spray", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendVendingMachineInteraction(AFortPlayerController* InteractingController, AB_Athena_VendingMachine_C* VendingMachine)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && VendingMachine)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", VendingMachine->K2_GetActorLocation().X},
                {"y", VendingMachine->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:vending", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendRubberDuckInteraction(AFortPlayerController* InteractingController, AProp_QuestInteractable_RubberDuckies_Parent_C* RubberDuck, FName Objective)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && RubberDuck)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        // Convert FName to std::string
        std::string sObjective = Objective.sToString();

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", RubberDuck->K2_GetActorLocation().X},
                {"y", RubberDuck->K2_GetActorLocation().Y},
                {"objective", sObjective}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:rubber-duck", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendLightningBoltInteraction(AFortPlayerController* InteractingController, AProp_QuestActor_FloatingLightningBolt_Parent_C* LightningBolt, FName Objective)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && LightningBolt)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        // Convert FName to std::string
        std::string sObjective = Objective.sToString();

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", LightningBolt->K2_GetActorLocation().X},
                {"y", LightningBolt->K2_GetActorLocation().Y},
                {"objective", sObjective}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:lightningbolt", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::HandleSummaryReply(std::string Channel, nlohmann::json Payload)
{
    if (Channel.find("player:v2:") != std::string::npos)
    {
        Channel = Channel.substr(Channel.find(":") + 1);
        TO_WSTRING(Channel, wChannel);
        
        if (AFortPlayerController* PlayerController = player_component::get_player_by_account_id(wChannel))
        {
            if (Payload.is_object())
            {
                if (Payload.contains("status") && Payload["status"] == "ok")
                {
                    if (Payload.contains("response") && Payload["response"].is_object())
                    {
                        nlohmann::json response = Payload["response"];
                        
                        if (FAthenaRewardResult::GetClass())
                        {
                            FAthenaRewardResult* NewReward = new FAthenaRewardResult;

                            if (FindPropertyByName(FAthenaRewardResult::GetClass(), TEXT("Rewards")) &&
                                FindPropertyByName(FAthenaRewardResult::GetClass(), TEXT("XpMultipliers")))
                            {
                                if (FAthenaAwardGroup::GetClass() && FAthenaMatchXpMultiplierGroup::GetClass())
                                {
                                    TArrayManager Rewards = TArrayManager<FAthenaAwardGroup>(&NewReward->Rewards(), FAthenaAwardGroup::GetStructSize());
                                    TArrayManager XpMultipliers = TArrayManager<FAthenaMatchXpMultiplierGroup>(&NewReward->XpMultipliers(), FAthenaMatchXpMultiplierGroup::GetStructSize());

                                    if (response.contains("score") &&
                                        response.contains("xp"))
                                    {
                                        if (FindPropertyByName(FAthenaRewardResult::GetClass(), TEXT("TotalSeasonXpGained")))
                                        {
                                            NewReward->TotalSeasonXpGained() = response["xp"];
                                        }
                                        if (FindPropertyByName(FAthenaRewardResult::GetClass(), TEXT("PrePenaltySeasonXpGained")))
                                        {
                                            NewReward->PrePenaltySeasonXpGained() = response["xp"];
                                        }
                                        if (FindPropertyByName(FAthenaRewardResult::GetClass(), TEXT("AntiAddictionMultiplier")))
                                        {
                                            NewReward->AntiAddictionMultiplier() = 1.f;
                                        }
                                        
                                        static_cast<AFortPlayerControllerAthena*>(PlayerController)->ClientSendTeamStatsForPlayer(new FAthenaMatchTeamStats(static_cast<AFortPlayerControllerAthena*>(PlayerController)->EraPlayerInfo().PlayerStatsTracker.Placement, player_component::starting_players));
                                    }
                        
                                    if (response.contains("log") && response["log"].is_array())
                                    {
                                        nlohmann::json log = response["log"];

                                        for (int i = 0; i < log.size(); i++)
                                        {
                                            nlohmann::json log_element = log.at(i);

                                            if (log_element.is_object() &&
                                                log_element.contains("type") &&
                                                log_element.contains("value"))
                                            {
                                                std::string type = log_element["type"];
                                                int value = log_element["value"];

                                                if (type == "bonus")
                                                {
                                                    UEnum* EAthenaMatchXpMultiplierSource = FindObject<UEnum>(L"/Script/FortniteGame.EAthenaMatchXpMultiplierSource");
                                        
                                                    FAthenaMatchXpMultiplierGroup* NewXPMultiplier = new FAthenaMatchXpMultiplierGroup(EAthenaMatchXpMultiplierSource->GetEnumIndex(L"BonusXpEvent"), value);
                                                    XpMultipliers.Add(NewXPMultiplier);
                                                    free(NewXPMultiplier);
                                                }
                                                else if (type == "placement")
                                                {
                                                    UEnum* ERewardSource = FindObject<UEnum>(L"/Script/FortniteGame.ERewardSource");

                                                    FAthenaAwardGroup* NewAward = new FAthenaAwardGroup(ERewardSource->GetEnumIndex(L"Placement"), value, value);
                                                    Rewards.Add(NewAward);
                                                    free(NewAward);
                                                }
                                                else if (type == "first_kill")
                                                {
                                                    UEnum* ERewardSource = FindObject<UEnum>(L"/Script/FortniteGame.ERewardSource");

                                                    FAthenaAwardGroup* NewAward = new FAthenaAwardGroup(ERewardSource->GetEnumIndex(L"FirstKill"), value, value);
                                                    Rewards.Add(NewAward);
                                                    free(NewAward);
                                                }
                                                else if (type == "time")
                                                {
                                                    UEnum* ERewardSource = FindObject<UEnum>(L"/Script/FortniteGame.ERewardSource");

                                                    FAthenaAwardGroup* NewAward = new FAthenaAwardGroup(ERewardSource->GetEnumIndex(L"MinutesPlayed"), value, value);
                                                    Rewards.Add(NewAward);
                                                    free(NewAward);
                                                }
                                                else if (type == "eliminations")
                                                {
                                                    UEnum* ERewardSource = FindObject<UEnum>(L"/Script/FortniteGame.ERewardSource");

                                                    FAthenaAwardGroup* NewAward = new FAthenaAwardGroup(ERewardSource->GetEnumIndex(L"TeamKills"), value, value);
                                                    Rewards.Add(NewAward);
                                                    free(NewAward);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            
                            static_cast<AFortPlayerControllerAthena*>(PlayerController)->ClientSendEndBattleRoyaleMatchForPlayer(true, NewReward);
                        }
                    }
                }
            }
        }
    }
}

void challenges_component::SendDiscoBallInteraction(AFortPlayerController* InteractingController)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                    {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                    {"y", InteractingController->Pawn()->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:disco-ball", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendShoppingCartInteraction(AFortPlayerController* InteractingController)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingController->Pawn()->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:cart", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendPitchGoalInteraction(AFortPlayerController* InteractingController, FName BackendObjectiveName)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);
        
        // Convert FName to std::string
        std::string sObjective = BackendObjectiveName.sToString();
        
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingController->Pawn()->K2_GetActorLocation().Y},
                {"objective", sObjective}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:goal", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }  
}

void challenges_component::SendBasketBallHoopInteraction(AFortPlayerController* InteractingController, FName BackendObjectiveName)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);
        
        // Convert FName to std::string
        std::string sObjective = BackendObjectiveName.sToString();
        
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingController->Pawn()->K2_GetActorLocation().Y},
                {"objective", sObjective}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:basket", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }  
}

void challenges_component::SendClayPidgeonInteraction(AFortPlayerController* InteractingController, FName BackendObjectiveName)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);
        
        // Convert FName to std::string
        std::string sObjective = BackendObjectiveName.sToString();
        
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingController->Pawn()->K2_GetActorLocation().Y},
                {"objective", sObjective}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:pigeon", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }  
}

void challenges_component::SendPOIInteraction(AFortPlayerController* InteractingController, FName BackendObjectiveName)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);
        
        // Convert FName to std::string
        std::string sObjective = BackendObjectiveName.sToString();

        if (sObjective.find("Athena.Location.POI") == std::string::npos)
        {
            return;
        }
        
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingController->Pawn()->K2_GetActorLocation().Y},
                {"poi", sObjective}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:poi", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }  
}

void challenges_component::SendJewelInteraction(AFortPlayerController* InteractingController)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);
        
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingController->Pawn()->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:jewel", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendGolfInteraction(AFortPlayerController* InteractingController, FName BackendObjectiveName)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);
        
        // Convert FName to std::string
        std::string sObjective = BackendObjectiveName.sToString();
        
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingController->Pawn()->K2_GetActorLocation().Y},
                {"objective", sObjective}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:golf", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendRiftInteraction(AFortPlayerController* InteractingController, ABGA_RiftPortal_Athena* RiftPortal)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && RiftPortal)
    {
        std::string account_id = player_component::get_account_id(InteractingController);
        
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", RiftPortal->K2_GetActorLocation().X},
                {"y", RiftPortal->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:rift", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendRiftInteraction(AFortPlayerController* InteractingController)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);
        
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingController->Pawn()->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:rift", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }  
}

void challenges_component::SendDestroyPlaneInteraction(AFortPlayerController* InteractingController)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);
        
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                    {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                    {"y", InteractingController->Pawn()->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:destroy-plane", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }
}

void challenges_component::SendTrapInteraction(AFortPlayerController* InteractingController)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);
        
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                        {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                        {"y", InteractingController->Pawn()->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:placed-trap", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }
}

void challenges_component::SendJigsawPuzzleInteraction(AFortPlayerController* InteractingController,
    AProp_QuestInteractable_Jigsaw_Parent* JigsawPuzzle, FName Objective)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && JigsawPuzzle)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        // Convert FName to std::string
        std::string sObjective = Objective.sToString();

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                    {"x", JigsawPuzzle->K2_GetActorLocation().X},
                    {"y", JigsawPuzzle->K2_GetActorLocation().Y},
                    {"objective", sObjective}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:jigsaw-puzzle", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendSpotlightDanceInteraction(AFortPlayerController* InteractingController,
    AStreet_Light_Spotlight_AthenaQuest_C* StreetLight, FName Objective)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && StreetLight)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        // Convert FName to std::string
        std::string sObjective = Objective.sToString();

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", StreetLight->K2_GetActorLocation().X},
                {"y", StreetLight->K2_GetActorLocation().Y},
                {"objective", sObjective}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:streetlight-dance", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendPickupInteraction(AFortPlayerController* InteractingPlayerController,
                                                 AFortPickup* Pickup)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingPlayerController && InteractingPlayerController->Pawn() && Pickup)
    {
        std::string account_id = player_component::get_account_id(InteractingPlayerController);

        // Convert tier to rarity enum
        std::string rarity;

        if (Pickup && Pickup->PrimaryPickupItemEntry().ItemDefinition())
        {
            rarity = Pickup->PrimaryPickupItemEntry().ItemDefinition()->GetRarity();
        }

        if (account_id.size() > 0 && rarity.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingPlayerController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingPlayerController->Pawn()->K2_GetActorLocation().Y},
                {"rarity", rarity}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:pickup", parameters);
        }
        else
        {
            if (account_id.size() <= 0)
            {
                if (log_component::enable_debug_logs)
                {
                    spdlog::error(L"Invalid account_id while sending event.");
                }
            }
        }
    }  
}

void challenges_component::SendCorruptedAreaInteraction(AFortPlayerController* InteractingPlayerController,
    FGameplayTag POI)
{
    CHECK_CREATIVE()
    
    if (InteractingPlayerController && InteractingPlayerController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingPlayerController);
        
        // Convert FName to std::string
        std::string sObjective = POI.TagName.sToString();
        
        if (account_id.size() > 0)
        {
            static std::map<std::string, std::string> ObjectivesMap = {
                {"Athena.Location.Meta.SevenSeals.01", "battlepass_visit_athena_location_seal_01"},
                {"Athena.Location.Meta.SevenSeals.02", "battlepass_visit_athena_location_seal_02"},
                {"Athena.Location.Meta.SevenSeals.03", "battlepass_visit_athena_location_seal_03"},
                {"Athena.Location.Meta.SevenSeals.04", "battlepass_visit_athena_location_seal_04"},
                {"Athena.Location.Meta.SevenSeals.05", "battlepass_visit_athena_location_seal_05"},
                {"Athena.Location.Meta.SevenSeals.06", "battlepass_visit_athena_location_seal_06"},
                {"Athena.Location.Meta.SevenSeals.07", "battlepass_visit_athena_location_seal_07"}
            };

            if (ObjectivesMap.contains(sObjective))
            {
                nlohmann::json parameters{
                    {"x", InteractingPlayerController->Pawn()->K2_GetActorLocation().X},
                    {"y", InteractingPlayerController->Pawn()->K2_GetActorLocation().Y},
                    {"objective", ObjectivesMap[sObjective]}
                };
            
                gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:corrupted-area", parameters);
            }
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    } 
}

void challenges_component::SendNomsSignInteraction(AFortPlayerController* InteractingPlayerController, FGameplayTag POI)
{
    CHECK_CREATIVE()
    
    if (InteractingPlayerController && InteractingPlayerController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingPlayerController);
        
        // Convert FName to std::string
        std::string sObjective = POI.TagName.sToString();
        
        if (account_id.size() > 0)
        {
            if (sObjective == "Athena.Location.Visit.Quest.NOMS")
            {
                nlohmann::json parameters{
                    {"x", InteractingPlayerController->Pawn()->K2_GetActorLocation().X},
                    {"y", InteractingPlayerController->Pawn()->K2_GetActorLocation().Y}
                };
            
                gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:noms", parameters);
            }
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    } 
}

void challenges_component::SendSpookyMistInteraction(AFortPlayerController* InteractingPlayerController)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
   if (InteractingPlayerController && InteractingPlayerController->Pawn())
   {
       std::string account_id = player_component::get_account_id(InteractingPlayerController);
        
       if (account_id.size() > 0)
       {
           nlohmann::json parameters{
               {"x", InteractingPlayerController->Pawn()->K2_GetActorLocation().X},
               {"y", InteractingPlayerController->Pawn()->K2_GetActorLocation().Y}
           };
            
           gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:shadow-stone", parameters);
       }
       else
       {
           if (log_component::enable_debug_logs)
           {
               spdlog::error(L"Invalid account_id while sending event.");
           }
       }
   }   
}

void challenges_component::SendTomatoInteraction(AFortPlayerController* InteractingPlayerController, float distance)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingPlayerController && InteractingPlayerController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingPlayerController);
         
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingPlayerController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingPlayerController->Pawn()->K2_GetActorLocation().Y},
                {"distance", distance}
            };
             
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:tomato", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendSnowballInteraction(AFortPlayerController* InteractingPlayerController)
{
    CHECK_CREATIVE()
    
    if (InteractingPlayerController && InteractingPlayerController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingPlayerController);
         
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingPlayerController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingPlayerController->Pawn()->K2_GetActorLocation().Y}
            };
             
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:snowball", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendDoorbellInteraction(AFortPlayerController* InteractingPlayerController)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingPlayerController && InteractingPlayerController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingPlayerController);
         
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingPlayerController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingPlayerController->Pawn()->K2_GetActorLocation().Y}
            };
             
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:doorbell", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendDoorbellPOIInteraction(AFortPlayerController* InteractingPlayerController, FName Objective)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingPlayerController && InteractingPlayerController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingPlayerController);

        std::string sObjective = Objective.sToString();
        
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingPlayerController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingPlayerController->Pawn()->K2_GetActorLocation().Y},
                {"objective", sObjective}
            };
             
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:doorbell-poi", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }   
}

void challenges_component::SendThankBusDriverInteraction(AFortPlayerControllerAthena* InteractingPlayerController)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()

    if (InteractingPlayerController)
    {
        std::string account_id = player_component::get_account_id(InteractingPlayerController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters
            {
                {"x", 0.0},
                {"y", 0.0}
            };

            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:bus-thank-you", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }
}

void challenges_component::SendZiplineInteraction(AFortPlayerControllerAthena* InteractingPlayerController)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()

    if (InteractingPlayerController)
    {
        std::string account_id = player_component::get_account_id(InteractingPlayerController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingPlayerController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingPlayerController->Pawn()->K2_GetActorLocation().Y}
            };

            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:zipline", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }
}

void challenges_component::SendPlaneInteraction(AFortPlayerControllerAthena* InteractingPlayerController)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()

    if (InteractingPlayerController)
    {
        std::string account_id = player_component::get_account_id(InteractingPlayerController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingPlayerController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingPlayerController->Pawn()->K2_GetActorLocation().Y}
            };

            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:use-plane", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }
}

void challenges_component::SendBoogieBombInteraction(AFortPlayerControllerAthena* InteractingPlayerController)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()

    if (InteractingPlayerController)
    {
        std::string account_id = player_component::get_account_id(InteractingPlayerController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingPlayerController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingPlayerController->Pawn()->K2_GetActorLocation().Y}
            };

            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:boogie-bomb", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }
}

void challenges_component::SendFireworksInteraction(AFortPlayerControllerAthena* InteractingPlayerController, ABP_FrozenFireworkLauncher_Parent* firework, FName Objective)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()

    if (InteractingPlayerController)
    {
        std::string account_id = player_component::get_account_id(InteractingPlayerController);

        std::string sObjective = Objective.sToString();
        
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", firework->K2_GetActorLocation().X},
                {"y", firework->K2_GetActorLocation().Y},
                {"objective", sObjective}
            };

            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:firework", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }
}

void challenges_component::SendIcePuckInteraction(AFortPlayerControllerAthena* InteractingPlayerController)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()

    if (InteractingPlayerController && InteractingPlayerController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingPlayerController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingPlayerController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingPlayerController->Pawn()->K2_GetActorLocation().Y}
            };

            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:ice-puck", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }
}

void challenges_component::SendExplorerOutpostInteraction(AFortPlayerControllerAthena* InteractingPlayerController, FGameplayTag POI)
{
    CHECK_CREATIVE()
    
    if (InteractingPlayerController && InteractingPlayerController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingPlayerController);
        
        // Convert FName to std::string
        std::string sObjective = POI.TagName.sToString();
        
        if (account_id.size() > 0)
        {
            static std::map<std::string, std::string> ObjectivesMap = {
                {"Athena.Quests.Location.ExplorerOutposts.A", "battlepass_visit_athena_expeditionoutpost_a"},
                {"Athena.Quests.Location.ExplorerOutposts.B", "battlepass_visit_athena_expeditionoutpost_b"},
                {"Athena.Quests.Location.ExplorerOutposts.C", "battlepass_visit_athena_expeditionoutpost_c"},
                {"Athena.Quests.Location.ExplorerOutposts.D", "battlepass_visit_athena_expeditionoutpost_d"},
                {"Athena.Quests.Location.ExplorerOutposts.E", "battlepass_visit_athena_expeditionoutpost_e"},
                {"Athena.Quests.Location.ExplorerOutposts.F", "battlepass_visit_athena_expeditionoutpost_f"},
                {"Athena.Quests.Location.ExplorerOutposts.G", "battlepass_visit_athena_expeditionoutpost_g"}
            };

            if (ObjectivesMap.contains(sObjective))
            {
                nlohmann::json parameters{
                    {"x", InteractingPlayerController->Pawn()->K2_GetActorLocation().X},
                    {"y", InteractingPlayerController->Pawn()->K2_GetActorLocation().Y},
                    {"objective", ObjectivesMap[sObjective]}
                };
            
                gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:outpost", parameters);
            }
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    } 
}

void challenges_component::SendPlaceEvent(AFortPlayerController* InteractingPlayerController, std::string Type)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()

    if (InteractingPlayerController && InteractingPlayerController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingPlayerController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingPlayerController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingPlayerController->Pawn()->K2_GetActorLocation().Y},
                {"type", Type}
            };

            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:place", parameters);

            if (log_component::enable_debug_logs)
            {
                spdlog::info("challenges_component::SendPlaceEvent: sent interaction:place event with payload {}", parameters.dump().c_str());
            }
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }
}

void challenges_component::HandleItemAdded(AFortPlayerController* InteractingPlayerController, UFortItemDefinition* ItemDefinition)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()

    static UFortItemDefinition* Athena_Bling_Pack_Definition = FindObject<UFortItemDefinition>(L"/Game/Athena/Playlists/Bling/BlingPack/Athena_Bling_Pack.Athena_Bling_Pack");
    static UFortItemDefinition* AGID_SpookyMist = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Gameplay/SpookyMist/AGID_SpookyMist.AGID_SpookyMist");
    
    if (Athena_Bling_Pack_Definition)
    { 
        if (ItemDefinition == Athena_Bling_Pack_Definition)
        {
            // Log
            if (log_component::enable_debug_logs)
            {
                spdlog::info(L"challenges_component::HandleItemAdded is sending jewel interaction.");
            }

            // Send event
            SendJewelInteraction(InteractingPlayerController);
        }
    }

    if (AGID_SpookyMist)
    {
        if (ItemDefinition == AGID_SpookyMist)
        {
            // Log
            if (log_component::enable_debug_logs)
            {
                spdlog::info(L"challenges_component::HandleItemAdded is sending spooky mist interaction.");
            }

            // Send event
            SendSpookyMistInteraction(InteractingPlayerController);
        }
    }
}

void challenges_component::SendFlamingHoopInteraction(AFortPlayerController* InteractingController,
    AActor_QuestObject_Touch_FlamingHoops_Parent* FlamingHoop, FName BackendObjectiveName)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && FlamingHoop)
    {
        std::string account_id = player_component::get_account_id(InteractingController);
        
        // Convert FName to std::string
        std::string sObjective = BackendObjectiveName.sToString();
        
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", FlamingHoop->K2_GetActorLocation().X},
                {"y", FlamingHoop->K2_GetActorLocation().Y},
                {"objective", sObjective}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:flaming-ring", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }  
}

void challenges_component::SendPopGoldenBalloonInteraction(AFortPlayerController* InteractingController,
    AProp_QuestInteractable_GoldBalloon_Parent_C* GoldenBalloon, FName BackendObjectiveName)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && GoldenBalloon)
    {
        std::string account_id = player_component::get_account_id(InteractingController);
        
        // Convert FName to std::string
        std::string sObjective = BackendObjectiveName.sToString();
        
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                    {"x", GoldenBalloon->K2_GetActorLocation().X},
                    {"y", GoldenBalloon->K2_GetActorLocation().Y},
                    {"objective", sObjective}
            };
            
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:pop-golden-balloon", parameters);
        }
        else
        {
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"Invalid account_id while sending event.");
            }
        }
    }  
}

void challenges_component_hooks::BndEvt__Goal_02_Overlap_K2Node_ComponentBoundEvent_0_ComponentBeginOverlapSignature__DelegateSignature_Hook(UObject* Object, UObject* Function, void* Params)
{
    AAthena_SoccerGame_C* SoccerGame = static_cast<AAthena_SoccerGame_C*>(Object);
    
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"challenges_component_hooks::BndEvt__Goal_02_Overlap_K2Node_ComponentBoundEvent_0_ComponentBeginOverlapSignature__DelegateSignature_Hook: Sending goal interaction.");
    }

    if (SoccerGame)
    {
        if (SoccerGame->PlayerWhoScored())
        {
            if (STATIC_HASPROPERTY(SoccerGame, BackendObjName))
            {
                challenges_component::SendPitchGoalInteraction(SoccerGame->PlayerWhoScored()->Controller(), SoccerGame->BackendObjName());
            } 
        }
    }
}

void challenges_component_hooks::BndEvt__Goal_01_Overlap_K2Node_ComponentBoundEvent_1_ComponentBeginOverlapSignature__DelegateSignature_Hook(UObject* Object, UObject* Function, void* Params)
{
    AAthena_SoccerGame_C* SoccerGame = static_cast<AAthena_SoccerGame_C*>(Object);

    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"challenges_component_hooks::BndEvt__Goal_01_Overlap_K2Node_ComponentBoundEvent_1_ComponentBeginOverlapSignature__DelegateSignature_Hook: Sending goal interaction.");
    }
    
    if (SoccerGame)
    {
        if (SoccerGame->PlayerWhoScored())
        {
            if (STATIC_HASPROPERTY(SoccerGame, BackendObjName))
            {
                challenges_component::SendPitchGoalInteraction(SoccerGame->PlayerWhoScored()->Controller(), SoccerGame->BackendObjName());
            }
        }
    }
}

void challenges_component_hooks::BndEvt__HoleCollision_K2Node_ComponentBoundEvent_0_ComponentBeginOverlapSignature__DelegateSignature_Hook(UObject* Object, UObject* Function, void* Params)
{
    GET_PROCESS_EVENT_PARAMETER(AActor*, OtherActor);

    AB_GolfMinigame_Hole* GolfGame = static_cast<AB_GolfMinigame_Hole*>(Object);

    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"challenges_component_hooks::BndEvt__HoleCollision_K2Node_ComponentBoundEvent_0_ComponentBeginOverlapSignature__DelegateSignature_Hook: Sending golf interaction");
    }

    if (GolfGame)
    {
        if (OtherActor && OtherActor->Owner() && OtherActor->Owner()->IsA(AFortPlayerStateAthena::StaticClass()))
        {
            if (OtherActor->Owner()->Owner()->IsA(AFortPlayerControllerAthena::StaticClass()))
            {
                if (STATIC_HASPROPERTY(GolfGame, ObjectiveBackendName))
                {
                    challenges_component::SendGolfInteraction(static_cast<AFortPlayerController*>(OtherActor->Owner()->Owner()), GolfGame->ObjectiveBackendName());
                }
            }
        }
    }
}

void challenges_component_hooks::BndEvt__HoopTriggerBox_K2Node_ComponentBoundEvent_0_ComponentBeginOverlapSignature__DelegateSignature_Hook(UObject* Object, UObject* Function, void* Params)
{
    AB_BasketballHoopWithMinigame_C* BasketBallGame = static_cast<AB_BasketballHoopWithMinigame_C*>(Object);

    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"challenges_component_hooks::BndEvt__HoopTriggerBox_K2Node_ComponentBoundEvent_0_ComponentBeginOverlapSignature__DelegateSignature_Hook: Sending hoop interaction.");
    }
    
    if (BasketBallGame)
    {
        if (BasketBallGame->PC_That_Scored())
        {
            if (STATIC_HASPROPERTY(BasketBallGame, ObjBackendName))
            {
                challenges_component::SendBasketBallHoopInteraction(BasketBallGame->PC_That_Scored(), BasketBallGame->ObjBackendName());
            }
        }
    }
}

void challenges_component_hooks::PidgeonShot_Hook(UObject* Object, UObject* Function, void* Params)
{
    GET_PROCESS_EVENT_PARAMETER(AFortPlayerController*, InstigatedBy);
    
    AProp_Quest_ClayPidgeonLauncher_C* ClayPidgeonLauncher = static_cast<AProp_Quest_ClayPidgeonLauncher_C*>(Object);

    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"challenges_component_hooks::PidgeonShot_Hook: Sending clay pidgeon interaction.");
    }
    
    if (ClayPidgeonLauncher && InstigatedBy)
    {
        if (STATIC_HASPROPERTY(ClayPidgeonLauncher, ObjBackendName))
        {
            challenges_component::SendClayPidgeonInteraction(InstigatedBy, ClayPidgeonLauncher->ObjBackendName());
        }
    }
}

void challenges_component_hooks::Rising__FinishedFunc_Hook(UObject* Object, UObject* Function, void* Params)
{
    if (Object->IsA(ABA_QuestInteractable_DancePlatforms::StaticClass()))
    {
        ABA_QuestInteractable_DancePlatforms* DancePlatform = static_cast<ABA_QuestInteractable_DancePlatforms*>(Object);

        for (auto Platform1Pawn : DancePlatform->Platform1_Pawns())
        {
            if (Platform1Pawn && Platform1Pawn->Controller())
            {
                challenges_component::SendDiscoBallInteraction(Platform1Pawn->Controller());
            }
        }
        for (auto Platform2Pawn : DancePlatform->Platform2_Pawns())
        {
            if (Platform2Pawn && Platform2Pawn->Controller())
            {
                challenges_component::SendDiscoBallInteraction(Platform2Pawn->Controller());
            }
        }
        for (auto Platform3Pawn : DancePlatform->Platform3_Pawns())
        {
            if (Platform3Pawn && Platform3Pawn->Controller())
            {
                challenges_component::SendDiscoBallInteraction(Platform3Pawn->Controller());
            }
        }
        for (auto Platform4Pawn : DancePlatform->Platform4_Pawns())
        {
            if (Platform4Pawn && Platform4Pawn->Controller())
            {
                challenges_component::SendDiscoBallInteraction(Platform4Pawn->Controller());
            }
        }
    }
}

void challenges_component_hooks::KillPuck_Hook(UObject* Object, UObject* Function, void* Params)
{
    AB_Toy_IcePuck_Base_C* IcePuck = reinterpret_cast<AB_Toy_IcePuck_Base_C*>(Object);

    if (IcePuck && IcePuck->Owner() && IcePuck->Owner()->Owner())
    {
        if (AFortPlayerControllerAthena* OwningPC = IcePuck->Owner()->Owner()->Cast<AFortPlayerControllerAthena>())
        {
            if (IcePuck->TotalDistanceTraveled() >= 15000.0)
            {
                // Send interaction
                challenges_component::SendIcePuckInteraction(OwningPC);
            }
        }
    }
}

void challenges_component_hooks::Athena_GiveQuestUpdateToPlayers_Hook(UObject* Context, FFrame& StackFrame, void* Param)
{
    GET_PARAMETER(UFortQuestItemDefinition*, InQuestItemRef);
    GET_PARAMETER(FName, ObjectiveBackendName);
    GET_PARAMETER(TArray<AFortPlayerController*>, PlayerControllersForUpdate);
    
    if (log_component::enable_debug_logs)
    {
        spdlog::info(L"challenges_component_hooks::Athena_GiveQuestUpdateToPlayers_Hook called with {}.", ObjectiveBackendName.ToString().c_str());
    }

    for (AFortPlayerController* PlayerController : PlayerControllersForUpdate)
    {
        if (PlayerController && PlayerController->Pawn())
        {
            if (challenges_component::QuestToInteractionMatch.contains(InQuestItemRef))
            {
                std::string account_id = player_component::get_account_id(PlayerController);
                std::string Objective = ObjectiveBackendName.sToString().c_str();

                if (account_id.size() > 0)
                {
                    std::string Event = challenges_component::QuestToInteractionMatch[InQuestItemRef];

                    nlohmann::json Payload{
                        {"x", PlayerController->Pawn()->K2_GetActorLocation().X},
                        {"y", PlayerController->Pawn()->K2_GetActorLocation().Y},
                        {"objective", Objective}
                    };

                    if (log_component::enable_debug_logs)
                    {
                        spdlog::info(L"challenges_component_hooks::Athena_GiveQuestUpdateToPlayers_Hook sending {}.", ObjectiveBackendName.ToString().c_str());
                    }

                    gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, Event, Payload);
                }
            }
        }
    }
}