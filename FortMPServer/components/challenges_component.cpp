#include "include.hpp"
#include "challenges_component.hpp"

#include "log_component.hpp"
#include "player_component.hpp"
#include "team_component.hpp"

challenges_component::challenges_component()
{
    REGISTER_PROCESS_EVENT_DETOUR(L"/Game/Athena/Items/QuestInteractables/DancePlatforms/BA_QuestInteractable_DancePlatforms.BA_QuestInteractable_DancePlatforms_C.Rising__FinishedFunc", challenges_component_hooks::Rising__FinishedFunc_Hook);
    REGISTER_POST_PROCESS_EVENT_DETOUR(L"/Game/Athena/Prototype/SoccerGame/Athena_SoccerGame.Athena_SoccerGame_C.BndEvt__Goal_02_Overlap_K2Node_ComponentBoundEvent_0_ComponentBeginOverlapSignature__DelegateSignature", challenges_component_hooks::BndEvt__Goal_02_Overlap_K2Node_ComponentBoundEvent_0_ComponentBeginOverlapSignature__DelegateSignature_Hook);
    REGISTER_POST_PROCESS_EVENT_DETOUR(L"/Game/Athena/Prototype/SoccerGame/Athena_SoccerGame.Athena_SoccerGame_C.BndEvt__Goal_01_Overlap_K2Node_ComponentBoundEvent_1_ComponentBeginOverlapSignature__DelegateSignature", challenges_component_hooks::BndEvt__Goal_01_Overlap_K2Node_ComponentBoundEvent_1_ComponentBeginOverlapSignature__DelegateSignature_Hook);
}

void challenges_component::SendChestEvent(AFortPlayerController* InteractingController, ABuildingContainer* TargetContainer)
{
    if (InteractingController && TargetContainer)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            std::map<std::string, std::string> parameters{
                {"x", std::to_string(TargetContainer->K2_GetActorLocation().X)},
                {"y", std::to_string(TargetContainer->K2_GetActorLocation().Y)}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "event:chest", parameters);
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
    if (InteractingController && SupplyDrop)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            std::map<std::string, std::string> parameters{
                {"x", std::to_string(SupplyDrop->K2_GetActorLocation().X)},
                {"y", std::to_string(SupplyDrop->K2_GetActorLocation().Y)}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "event:supply", parameters);
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
    if (InteractingController && TargetContainer)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            std::map<std::string, std::string> parameters{
                {"x", std::to_string(TargetContainer->K2_GetActorLocation().X)},
                {"y", std::to_string(TargetContainer->K2_GetActorLocation().Y)}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "event:ammo", parameters);
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
    if (OwnerPC && Self)
    {
        std::string account_id = player_component::get_account_id(OwnerPC);

        if (account_id.size() > 0)
        {
            std::map<std::string, std::string> parameters{
                {"x", std::to_string(Self->K2_GetActorLocation().X)},
                {"y", std::to_string(Self->K2_GetActorLocation().Y)}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "event:landing", parameters);
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
    if (InteractingController && Llama)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            std::map<std::string, std::string> parameters{
                {"x", std::to_string(Llama->K2_GetActorLocation().X)},
                {"y", std::to_string(Llama->K2_GetActorLocation().Y)}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "event:llama", parameters);
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
    if (PlayerController)
    {
        std::string account_id = player_component::get_account_id(PlayerController);
        FDamageDealtMapValue DamageDealt = FDamageDealtMapValue();

        if (player_component::DamageDealtMap.contains(PlayerController))
        {
            DamageDealt = player_component::DamageDealtMap[PlayerController];
        }

        if (account_id.size() > 0)
        {
            nlohmann::json damage_enum = {
                {"pistol", DamageDealt.PistolDamage},
                {"suppressed_pistol", DamageDealt.SuppressedPistolDamage},
                {"shotgun", DamageDealt.ShotgunDamage},
                {"assault_rifle", DamageDealt.AssaultRifleDamage},
                {"sniper", DamageDealt.SniperDamage},
                {"grenade", DamageDealt.GrenadeDamage},
                {"smg", DamageDealt.SMGDamage},
                {"suppressed_smg", DamageDealt.SuppressedSMGDamage},
                {"grenade_launcher", DamageDealt.GrenadeLauncherDamage},
                {"lmg", DamageDealt.LMGDamage},
                {"minigun", DamageDealt.MinigunDamage},
                {"rpg", DamageDealt.RPGDamage}
            };

            nlohmann::json parameters = {
                {"placement", Placement},
                {"damage", damage_enum}
            };
            
            gameserver_client::WaitForReply(gameserver_client::Client->SendEventToChannel("player:" + account_id, "summary", parameters), HandleSummaryReply);

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

void challenges_component::SendElimination(AFortPlayerController* KillerPlayerController, AFortPlayerController* VictimPlayerController, uint8_t DeathCause)
{
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
                    }
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"Shotgun"))
                    {
                        weapon = "shotgun";
                    }
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"Rifle"))
                    {
                        weapon = "assault_rifle";
                    }
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"Sniper") ||
                             DeathCause == death_cause_enum->GetEnumIndex(L"SniperNoScope") )
                    {
                        weapon = "sniper";
                    }
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"Grenade"))
                    {
                        weapon = "grenade";
                    }
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"SMG"))
                    {
                        weapon = "smg";
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
                }
            
                std::map<std::string, std::string> parameters{
                        {"x", std::to_string(VictimPlayerController->Pawn()->K2_GetActorLocation().X)},
                        {"y", std::to_string(VictimPlayerController->Pawn()->K2_GetActorLocation().Y)},
                        {"weapon", weapon},
                        {"victim", victim_account_id}
                };
            
                gameserver_client::Client->SendEventToChannel("player:" + killer_account_id, "event:elimination", parameters);
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
                    {"victim", victim_account_id}
                };
            
                gameserver_client::Client->SendEventToChannel("player:" + executing_account_id, "event:revive", parameters);
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

void challenges_component::SendLetterInteraction(AFortPlayerController* InteractingController, AProp_QuestInteractable_Letter_Parent_C* Letter, FName Objective)
{
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
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "interaction:letter", parameters);
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
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "interaction:gnome", parameters);
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
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "interaction:star", parameters);
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
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                {"y", InteractingController->Pawn()->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "interaction:port-a-fort", parameters);
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
    if (InteractingController && HopRock)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                    {"x", HopRock->K2_GetActorLocation().X},
                    {"y", HopRock->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "interaction:hop-rock", parameters);
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
    if (InteractingController && StormCenter)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", StormCenter->K2_GetActorLocation().X},
                {"y", StormCenter->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "interaction:storm-center", parameters);
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
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                    {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                    {"y", InteractingController->Pawn()->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "interaction:dance", parameters);
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
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                    {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                    {"y", InteractingController->Pawn()->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "interaction:spray", parameters);
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
    if (InteractingController && VendingMachine)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", VendingMachine->K2_GetActorLocation().X},
                {"y", VendingMachine->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "interaction:vending", parameters);
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
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "interaction:rubber-duck", parameters);
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
    if (Channel.find("player:") != std::string::npos)
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
                                        
                                        if (player_component::PlayerStatsMap.contains(PlayerController))
                                        {
                                            static_cast<AFortPlayerControllerAthena*>(PlayerController)->ClientSendTeamStatsForPlayer(new FAthenaMatchTeamStats(player_component::PlayerStatsMap[PlayerController].Placement, player_component::starting_players));
                                        }
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
    if (InteractingController && InteractingController->Pawn())
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                    {"x", InteractingController->Pawn()->K2_GetActorLocation().X},
                    {"y", InteractingController->Pawn()->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "interaction:disco-ball", parameters);
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
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "interaction:goal", parameters);
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
    GET_PROCESS_EVENT_PARAMETER(AActor*, OtherActor); 
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
    GET_PROCESS_EVENT_PARAMETER(AActor*, OtherActor); 
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