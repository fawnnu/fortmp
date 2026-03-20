#include "include.hpp"
#include "challenges_component.hpp"

#include "competitive_component.hpp"
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
        {FindObject<UFortQuestItemDefinition>(L"/Game/Athena/Items/Quests/BattlePass/Quest_BR_PianoChain_Play_Piano2.Quest_BR_PianoChain_Play_Piano2"), "interaction:piano"},
        {FindObject<UFortQuestItemDefinition>(L"/Game/Athena/Items/Quests/BattlePass/Quest_BR_Dance_ScavengerHunt_DanceOff_Abandoned_Mansion.Quest_BR_Dance_ScavengerHunt_DanceOff_Abandoned_Mansion"), "interaction:dance-off-competition"}
    };


    DestroyTagsToInteractionMatch = {
        { {FGameplayTag(L"Athena.Quests.Building.Chair")}, "interaction:chair" },
        { {FGameplayTag(L"Athena.Quests.Building.TelephonePoles")}, "interaction:pole" },
        { {FGameplayTag(L"Athena.Quests.Building.Wood.Palettes")}, "interaction:pallet" },
        { {FGameplayTag(L"Athena.Quests.Building.Cactus")}, "interaction:cactus" }
    };

    AssetNameFilterForInteraction = {
        { {L"Tree"}, "interaction:tree" },
        { {L"Car"}, "interaction:car" },
        { {L"Rocks"}, "interaction:rock" }
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

void challenges_component::AutocompleteChallenges(AFortPlayerController* PlayerController)
{
    // CL-based implementation
    if (GetCL() == 4363240)
    {
        {
            SendCustomEventWithObjective(PlayerController, "interaction:timed-trial", TEXT("battlepass_complete_timed_trials_01"));
            SendCustomEventWithObjective(PlayerController, "interaction:timed-trial", TEXT("battlepass_complete_timed_trials_02"));
            SendCustomEventWithObjective(PlayerController, "interaction:timed-trial", TEXT("battlepass_complete_timed_trials_03"));
            SendCustomEventWithObjective(PlayerController, "interaction:timed-trial", TEXT("battlepass_complete_timed_trials_04"));
            SendCustomEventWithObjective(PlayerController, "interaction:timed-trial", TEXT("battlepass_complete_timed_trials_05"));
        }

        {
            SendCustomEventWithObjective(PlayerController, "interaction:golf", TEXT("battlepass_athena_score_hole_01"));
            SendCustomEventWithObjective(PlayerController, "interaction:golf", TEXT("battlepass_athena_score_hole_02"));
            SendCustomEventWithObjective(PlayerController, "interaction:golf", TEXT("battlepass_athena_score_hole_03"));
            SendCustomEventWithObjective(PlayerController, "interaction:golf", TEXT("battlepass_athena_score_hole_04"));
            SendCustomEventWithObjective(PlayerController, "interaction:golf", TEXT("battlepass_athena_score_hole_05"));
        }

        {
            SendCustomEventWithObjective(PlayerController, "interaction:basket", TEXT("battlepass_athena_score_basket_01"));
            SendCustomEventWithObjective(PlayerController, "interaction:basket", TEXT("battlepass_athena_score_basket_02"));
            SendCustomEventWithObjective(PlayerController, "interaction:basket", TEXT("battlepass_athena_score_basket_03"));
            SendCustomEventWithObjective(PlayerController, "interaction:basket", TEXT("battlepass_athena_score_basket_04"));
            SendCustomEventWithObjective(PlayerController, "interaction:basket", TEXT("battlepass_athena_score_basket_05"));
        }

        {
            SendPOIInteraction(PlayerController, TEXT("Athena.Location.POI.RiskyReels"));
            SendPOIInteraction(PlayerController, TEXT("Athena.Location.POI.ParadisePalms"));
            SendPOIInteraction(PlayerController, TEXT("Athena.Location.POI.ShiftyShafts"));
            SendPOIInteraction(PlayerController, TEXT("Athena.Location.POI.TiltedTowers"));
        }

        {
            SendCustomEventWithObjective(PlayerController, "interaction:rift-spawn", TEXT("battlepass_athena_use_location_riftportal_01"));
            SendCustomEventWithObjective(PlayerController, "interaction:rift-spawn", TEXT("battlepass_athena_use_location_riftportal_02"));
            SendCustomEventWithObjective(PlayerController, "interaction:rift-spawn", TEXT("battlepass_athena_use_location_riftportal_03"));
            SendCustomEventWithObjective(PlayerController, "interaction:rift-spawn", TEXT("battlepass_athena_use_location_riftportal_04"));
            SendCustomEventWithObjective(PlayerController, "interaction:rift-spawn", TEXT("battlepass_athena_use_location_riftportal_05"));
            SendCustomEventWithObjective(PlayerController, "interaction:rift-spawn", TEXT("battlepass_athena_use_location_riftportal_06"));
            SendCustomEventWithObjective(PlayerController, "interaction:rift-spawn", TEXT("battlepass_athena_use_location_riftportal_07"));
            SendCustomEventWithObjective(PlayerController, "interaction:rift-spawn", TEXT("battlepass_athena_use_location_riftportal_08"));
            SendCustomEventWithObjective(PlayerController, "interaction:rift-spawn", TEXT("battlepass_athena_use_location_riftportal_09"));
            SendCustomEventWithObjective(PlayerController, "interaction:rift-spawn", TEXT("battlepass_athena_use_location_riftportal_10"));
        }

        {
            SendCustomEventWithObjective(PlayerController, "interaction:stone-head", TEXT("battlepass_visit_athena_location_stonehead_01"));
            SendCustomEventWithObjective(PlayerController, "interaction:stone-head", TEXT("battlepass_visit_athena_location_stonehead_02"));
            SendCustomEventWithObjective(PlayerController, "interaction:stone-head", TEXT("battlepass_visit_athena_location_stonehead_03"));
            SendCustomEventWithObjective(PlayerController, "interaction:stone-head", TEXT("battlepass_visit_athena_location_stonehead_04"));
            SendCustomEventWithObjective(PlayerController, "interaction:stone-head", TEXT("battlepass_visit_athena_location_stonehead_05"));
            SendCustomEventWithObjective(PlayerController, "interaction:stone-head", TEXT("battlepass_visit_athena_location_stonehead_06"));
            SendCustomEventWithObjective(PlayerController, "interaction:stone-head", TEXT("battlepass_visit_athena_location_stonehead_07"));
        }
    }
    else if (GetCL() == 4526925)
    {
        {
            SendCustomEventWithPayload(PlayerController, "interaction:tomato", nlohmann::json{
                {"x", 0.1f},
                {"y", 0.1f},
                {"distance", 16.0f}
                });
        }
    }
    else if (GetCL() == 4727874)
    {
        {
            SendCustomEvent(PlayerController, "interaction:ice-puck");
        }
        
        {
            SendCustomEventWithObjective(PlayerController, "interaction:autocomplete", TEXT("battlepass_destroy_goldballoon_01"));
            SendCustomEventWithObjective(PlayerController, "interaction:autocomplete", TEXT("battlepass_destroy_goldballoon_02"));
            SendCustomEventWithObjective(PlayerController, "interaction:autocomplete", TEXT("battlepass_destroy_goldballoon_03"));
            SendCustomEventWithObjective(PlayerController, "interaction:autocomplete", TEXT("battlepass_destroy_goldballoon_04"));
            SendCustomEventWithObjective(PlayerController, "interaction:autocomplete", TEXT("battlepass_destroy_goldballoon_05"));
            SendCustomEventWithObjective(PlayerController, "interaction:autocomplete", TEXT("battlepass_destroy_goldballoon_06"));
            SendCustomEventWithObjective(PlayerController, "interaction:autocomplete", TEXT("battlepass_destroy_goldballoon_07"));
            SendCustomEventWithObjective(PlayerController, "interaction:autocomplete", TEXT("battlepass_destroy_goldballoon_08"));
            SendCustomEventWithObjective(PlayerController, "interaction:autocomplete", TEXT("battlepass_destroy_goldballoon_09"));
            SendCustomEventWithObjective(PlayerController, "interaction:autocomplete", TEXT("battlepass_destroy_goldballoon_10"));
            SendCustomEventWithObjective(PlayerController, "interaction:autocomplete", TEXT("complete_timed_trial_stormwing_01"));
            SendCustomEventWithObjective(PlayerController, "interaction:autocomplete", TEXT("complete_timed_trial_stormwing_02"));
            SendCustomEventWithObjective(PlayerController, "interaction:autocomplete", TEXT("battlepass_visit_athena_expeditionoutpost_singlematch_a"));
            SendCustomEventWithObjective(PlayerController, "interaction:autocomplete", TEXT("battlepass_visit_athena_expeditionoutpost_singlematch_b"));
            SendCustomEventWithObjective(PlayerController, "interaction:autocomplete", TEXT("battlepass_visit_athena_expeditionoutpost_singlematch_c"));
            SendCustomEventWithObjective(PlayerController, "interaction:autocomplete", TEXT("battlepass_visit_athena_expeditionoutpost_singlematch_d"));
        }

        {
            SendCustomEvent(PlayerController, "interaction:destroy-plane");
        }
    }
    else if (GetCL() == 6165369)
    {
        {
            SendCustomEventWithCount(PlayerController, "interaction:destroy-structures-with-cannon-ball", 25);
        }
    }
}

void challenges_component::SendSummary(AFortPlayerController* PlayerController, int Placement)
{
    CHECK_PAST_WARMUP()
    
    if (PlayerController)
    {
        if (player_component::is_live_spectator(PlayerController))
        {
            spdlog::error(L"Player is a live spectator, not sending summary.");
            return;
        }
        
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
                    if (DeathCause == death_cause_enum->GetEnumIndex(L"Melee"))
                    {
                        /*
                         * Temporary: backend does not know how to handle "pickaxe", so we now use "trap". See 33bce082 for reference.
                         */
                        weapon = "trap"; 
                    }
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"Pistol"))
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
                
                static std::vector<std::string> POITagsForPirateCamps = {
                    "Athena.Quests.Location.PirateCamp.01",
                    "Athena.Quests.Location.PirateCamp.02",
                    "Athena.Quests.Location.PirateCamp.03",
                    "Athena.Quests.Location.PirateCamp.04",
                    "Athena.Quests.Location.PirateCamp.05",
                    "Athena.Quests.Location.PirateCamp.06",
                    "Athena.Quests.Location.PirateCamp.07"
                };

                // Accumulate rewards for competitive
                competitive_component::on_elimination(static_cast<AFortPlayerControllerAthena*>(KillerPlayerController));
                
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
                for (std::string POITag : POITagsForPirateCamps)
                {
                    TO_WSTRING(POITag, wPOITag);
                    FGameplayTag POIGameplayTag(wPOITag.c_str());
                    if (std::find(POIList.begin(), POIList.end(), POIGameplayTag) != POIList.end())
                    {
                        // Found one of the POI tags: break and add to elimination payload
                        parameters["meta"]["in_pirate_camp"] = true;
                        
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

void challenges_component::SendDanceInteraction(AFortPlayerController* InteractingController, UAthenaDanceItemDefinition* EmoteAsset)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()

    if (EmoteAsset == nullptr) return;
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingController->Pawn()->K2_GetActorLocation().Y},
                {"z", InteractingController->Pawn()->K2_GetActorLocation().Z},
                {"asset", EmoteAsset->GetName().c_str()}
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
                  {"Athena.Quests.DanceLocation.Generic.21", "battlepass_dance_chain_athena_scavengerhunt_locations_03_a"},
                  {"Athena.Quests.DanceLocation.Generic.22", "battlepass_dance_chain_athena_scavengerhunt_locations_03_b"},
                  {"Athena.Quests.DanceLocation.Generic.23", "battlepass_dance_chain_athena_scavengerhunt_locations_03_c"},
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

void challenges_component::HandleSummaryReply(std::string Channel, nlohmann::json Payload)
{
    if (Channel.find("player:v2:") != std::string::npos)
    {
        Channel = Channel.substr(Channel.find_last_of(":") + 1);
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

void challenges_component::SendPOIInteraction(AFortPlayerController* InteractingController, FName BackendObjectiveName)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = static_cast<AFortPlayerControllerAthena*>(InteractingController)->EraPlayerInfo().sAccountId;
        
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
            
            gameserver_client::Client->SendEventToPlayerChannel(InteractingController, "player:v2:" + account_id, "interaction:poi", parameters);
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

void challenges_component::SendCustomEvent(AFortPlayerController* InteractingController, std::string event_type)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()

    float x_location = 0.0f;
    float y_location = 0.0f;

    // If the pawn exists, apply the location. Otherwise keep it as 0 for both axis, for interactions such as thanking the bus driver.
    if (AFortPlayerPawn* player_pawn = InteractingController->Pawn())
    {
        x_location = player_pawn->K2_GetActorLocation().X;
        y_location = player_pawn->K2_GetActorLocation().Y;
    }
    
    if (InteractingController)
    {
        std::string account_id = static_cast<AFortPlayerControllerAthena*>(InteractingController)->EraPlayerInfo().sAccountId;
         
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                     {"x", x_location},
                     {"y", y_location}
            };
             
            gameserver_client::Client->SendEventToPlayerChannel(InteractingController, "player:v2:" + account_id, event_type, parameters);
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

void challenges_component::SendCustomEventWithPayload(AFortPlayerController* InteractingController, std::string event_type, nlohmann::json payload)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = static_cast<AFortPlayerControllerAthena*>(InteractingController)->EraPlayerInfo().sAccountId;
        
        if (account_id.size() > 0)
        {
            gameserver_client::Client->SendEventToPlayerChannel(InteractingController, "player:v2:" + account_id, event_type, payload);
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

void challenges_component::SendCustomEventWithObjective(AFortPlayerController* InteractingController, std::string event_type, FName ObjBackendName)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = static_cast<AFortPlayerControllerAthena*>(InteractingController)->EraPlayerInfo().sAccountId;
        
        // Convert FName to std::string
        std::string sObjective = ObjBackendName.sToString();
        
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                    {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                    {"y", InteractingController->Pawn()->K2_GetActorLocation().Y},
                    {"objective", sObjective}
            };
            
            if (log_component::enable_debug_logs)
            {
                spdlog::error(L"SendCustomEventWithObjective is sending ObjBackendName [{}].", ObjBackendName.ToString());
            }
            
            gameserver_client::Client->SendEventToPlayerChannel(InteractingController, "player:v2:" + account_id, event_type, parameters);
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

void challenges_component::SendCustomEventWithCount(AFortPlayerController* InteractingController, std::string event_type, int Count)
{
    CHECK_PAST_WARMUP()
    CHECK_CREATIVE()
    
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = static_cast<AFortPlayerControllerAthena*>(InteractingController)->EraPlayerInfo().sAccountId;
        
        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                        {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                        {"y", InteractingController->Pawn()->K2_GetActorLocation().Y},
                        {"count", Count}
            };
            
            gameserver_client::Client->SendEventToPlayerChannel(InteractingController, "player:v2:" + account_id, event_type, parameters);
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
    
    std::string account_id = static_cast<AFortPlayerControllerAthena*>(InteractingPlayerController)->EraPlayerInfo().sAccountId;

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
            
        gameserver_client::Client->SendEventToPlayerChannel(InteractingPlayerController, "player:v2:" + account_id, "interaction:pickup", parameters);
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

void challenges_component::SendVisitInteraction(AFortPlayerController* InteractingPlayerController,
    FGameplayTag POI)
{
    CHECK_CREATIVE()
    
    if (InteractingPlayerController && InteractingPlayerController->Pawn())
    {
        std::string account_id = static_cast<AFortPlayerControllerAthena*>(InteractingPlayerController)->EraPlayerInfo().sAccountId;
        
        // Convert FName to std::string
        std::string sObjective = POI.TagName.sToString();
        
        if (account_id.size() > 0)
        {
            static std::map<std::string, std::string> ObjectivesMap = {
                {"Athena.Quests.Location.PirateCamp.01", "battlepass_visit_athena_piratecamp_01"},
                {"Athena.Quests.Location.PirateCamp.02", "battlepass_visit_athena_piratecamp_02"},
                {"Athena.Quests.Location.PirateCamp.03", "battlepass_visit_athena_piratecamp_03"},
                {"Athena.Quests.Location.PirateCamp.04", "battlepass_visit_athena_piratecamp_04"},
                {"Athena.Quests.Location.PirateCamp.05", "battlepass_visit_athena_piratecamp_05"},
                {"Athena.Quests.Location.PirateCamp.06", "battlepass_visit_athena_piratecamp_06"},
                {"Athena.Quests.Location.PirateCamp.07", "battlepass_visit_athena_piratecamp_07"},
                {"Athena.Quests.Location.GiantFace.Desert", "battlepass_visit_athena_location_cliff_face_desert"},
                {"Athena.Quests.Location.GiantFace.Jungle", "battlepass_visit_athena_location_cliff_face_jungle"},
                {"Athena.Quests.Location.GiantFace.Snow", "battlepass_visit_athena_location_cliff_face_snow"},
                {"Athena.Quests.Location.Farthest.North", "athena_battlepass_visit_far_north"},
                {"Athena.Quests.Location.Farthest.South", "athena_battlepass_visit_far_south"},
                {"Athena.Quests.Location.Farthest.East", "athena_battlepass_visit_far_east"},
                {"Athena.Quests.Location.Farthest.West", "athena_battlepass_visit_far_west"},
                {"Athena.Location.Meta.SevenSeals.01", "battlepass_visit_athena_location_seal_01"},
                {"Athena.Location.Meta.SevenSeals.02", "battlepass_visit_athena_location_seal_02"},
                {"Athena.Location.Meta.SevenSeals.03", "battlepass_visit_athena_location_seal_03"},
                {"Athena.Location.Meta.SevenSeals.04", "battlepass_visit_athena_location_seal_04"},
                {"Athena.Location.Meta.SevenSeals.05", "battlepass_visit_athena_location_seal_05"},
                {"Athena.Location.Meta.SevenSeals.06", "battlepass_visit_athena_location_seal_06"},
                {"Athena.Location.Meta.SevenSeals.07", "battlepass_visit_athena_location_seal_07"},
                {"Athena.Location.Visit.Quest.NOMS", "athena_battlepass_visit_noms_05"},
                {"Athena.Quests.Location.ExplorerOutposts.A", "battlepass_visit_athena_expeditionoutpost_a"},
                {"Athena.Quests.Location.ExplorerOutposts.B", "battlepass_visit_athena_expeditionoutpost_b"},
                {"Athena.Quests.Location.ExplorerOutposts.C", "battlepass_visit_athena_expeditionoutpost_c"},
                {"Athena.Quests.Location.ExplorerOutposts.D", "battlepass_visit_athena_expeditionoutpost_d"},
                {"Athena.Quests.Location.ExplorerOutposts.E", "battlepass_visit_athena_expeditionoutpost_e"},
                {"Athena.Quests.Location.ExplorerOutposts.F", "battlepass_visit_athena_expeditionoutpost_f"},
                {"Athena.Quests.Location.ExplorerOutposts.G", "battlepass_visit_athena_expeditionoutpost_g"},
                {"Athena.Location.Meta.AnimalBuilding.Rabbit", "battlepass_visit_athena_location_wooden_rabbit"},
                {"Athena.Location.Meta.AnimalBuilding.Llama", "battlepass_visit_athena_location_metal_llama"},
                {"Athena.Location.Meta.AnimalBuilding.Pig", "battlepass_visit_athena_location_stone_pig"},
                {"Athena.Quests.Location.HighestElevation.01", "battlepass_visit_highest_elevation_01"},
                {"Athena.Quests.Location.HighestElevation.02", "battlepass_visit_highest_elevation_02"},
                {"Athena.Quests.Location.HighestElevation.03", "battlepass_visit_highest_elevation_03"},
                {"Athena.Quests.Location.HighestElevation.04", "battlepass_visit_highest_elevation_04"},
                {"Athena.Quests.Location.HighestElevation.05", "battlepass_visit_highest_elevation_05"}
            };

            if (ObjectivesMap.contains(sObjective))
            {
                nlohmann::json parameters{
                    {"x", InteractingPlayerController->Pawn()->K2_GetActorLocation().X},
                    {"y", InteractingPlayerController->Pawn()->K2_GetActorLocation().Y},
                    {"objective", ObjectivesMap[sObjective]}
                };
            
                gameserver_client::Client->SendEventToPlayerChannel(InteractingPlayerController, "player:v2:" + account_id, "interaction:visit", parameters);
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

void challenges_component::SendGrenadeHitInteraction(AFortPlayerControllerAthena* InteractingPlayerController, std::string type)
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
                        {"type", type}
            };
    
            gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:hit", parameters);
    
            if (log_component::enable_debug_logs)
            {
                spdlog::info("challenges_component::SendGrenadeHitInteraction: sent interaction:hit event with payload {}", parameters.dump().c_str());
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

void challenges_component::SendHealInteraction(AFortPlayerControllerAthena* Self, std::string source, int count)
{
    CHECK_CREATIVE()
    
        if (!memory_manager::is_bad_read_pointer(Self) && Self && !memory_manager::is_bad_read_pointer(Self->Pawn()) && Self->Pawn())
        {
            std::string account_id = player_component::get_account_id(Self);

            if (account_id.size() > 0)
            {
                nlohmann::json parameters{
                    {"x", Self->Pawn()->K2_GetActorLocation().X},
                    {"y", Self->Pawn()->K2_GetActorLocation().Y},
                    {"source", source},
                    {"count", count}
                };

                if (log_component::enable_debug_logs)
                {
                    spdlog::info("challenges_component::SendHealInteraction is sending heal for {} ({})", source, count);
                }

                gameserver_client::Client->SendEventToChannel("player:v2:" + account_id, "interaction:heal", parameters);
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
            SendCustomEvent(InteractingPlayerController, "interaction:jewel");
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
            SendCustomEvent(InteractingPlayerController, "interaction:shadow-stone");
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
                challenges_component::SendCustomEventWithObjective(SoccerGame->PlayerWhoScored()->Controller(), "interaction:goal", SoccerGame->BackendObjName());
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
                challenges_component::SendCustomEventWithObjective(SoccerGame->PlayerWhoScored()->Controller(), "interaction:goal", SoccerGame->BackendObjName());
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
                    challenges_component::SendCustomEventWithObjective(static_cast<AFortPlayerController*>(OtherActor->Owner()->Owner()), "interaction:golf",GolfGame->ObjectiveBackendName());
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
                challenges_component::SendCustomEventWithObjective(BasketBallGame->PC_That_Scored(), "interaction:basket", BasketBallGame->ObjBackendName());
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
            challenges_component::SendCustomEventWithObjective(InstigatedBy, "interaction:pigeon", ClayPidgeonLauncher->ObjBackendName());
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
                challenges_component::SendCustomEvent(Platform1Pawn->Controller(), "interaction:disco-ball");
            }
        }
        for (auto Platform2Pawn : DancePlatform->Platform2_Pawns())
        {
            if (Platform2Pawn && Platform2Pawn->Controller())
            {
                challenges_component::SendCustomEvent(Platform2Pawn->Controller(), "interaction:disco-ball");
            }
        }
        for (auto Platform3Pawn : DancePlatform->Platform3_Pawns())
        {
            if (Platform3Pawn && Platform3Pawn->Controller())
            {
                challenges_component::SendCustomEvent(Platform3Pawn->Controller(), "interaction:disco-ball");
            }
        }
        for (auto Platform4Pawn : DancePlatform->Platform4_Pawns())
        {
            if (Platform4Pawn && Platform4Pawn->Controller())
            {
                challenges_component::SendCustomEvent(Platform4Pawn->Controller(), "interaction:disco-ball");
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
                challenges_component::SendCustomEvent(OwningPC, "interaction:ice-puck");
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