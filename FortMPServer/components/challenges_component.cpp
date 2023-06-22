#include "include.hpp"
#include "challenges_component.hpp"

#include "player_component.hpp"
#include "team_component.hpp"

challenges_component::challenges_component()
{
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
                {"shotgun", DamageDealt.ShotgunDamage},
                {"assault_rifle", DamageDealt.AssaultRifleDamage},
                {"sniper", DamageDealt.SniperDamage},
                {"grenade", DamageDealt.GrenadeDamage},
                {"smg", DamageDealt.SMGDamage},
                {"grenade_launcher", DamageDealt.GrenadeLauncherDamage},
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
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"GrenadeLauncher"))
                    {
                        weapon = "grenade_launcher";
                    }
                    else if (DeathCause == death_cause_enum->GetEnumIndex(L"RocketLauncher"))
                    {
                        weapon = "rpg";
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
    }   
}

void challenges_component::SendBattleStarInteraction(AFortPlayerController* InteractingController, AProp_QuestObject_Interactable_Parent_C* BattleStar)
{
    if (InteractingController && BattleStar)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", BattleStar->K2_GetActorLocation().X},
                {"y", BattleStar->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "interaction:star", parameters);
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
                        FAthenaRewardResult* NewReward = new FAthenaRewardResult();
                        TArrayManager Rewards = TArrayManager<FAthenaAwardGroup>(&NewReward->Rewards(), FAthenaAwardGroup::GetStructSize());
                        TArrayManager XpMultipliers = TArrayManager<FAthenaMatchXpMultiplierGroup>(&NewReward->XpMultipliers(), FAthenaMatchXpMultiplierGroup::GetStructSize());

                        if (response.contains("score") &&
                            response.contains("xp") &&
                            response.contains("placement"))
                        {
                            NewReward->TotalSeasonXpGained() = response["xp"];
                            NewReward->PrePenaltySeasonXpGained() = response["xp"];
                            
                            static_cast<AFortPlayerControllerAthena*>(PlayerController)->ClientSendTeamStatsForPlayer(new FAthenaMatchTeamStats(response["placement"], player_component::starting_players));
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
                        
                        static_cast<AFortPlayerControllerAthena*>(PlayerController)->ClientSendEndBattleRoyaleMatchForPlayer(true, NewReward);
                    }
                }
            }
        }
    }
}
