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
        DamageDealtMapValue DamageDealt = DamageDealtMapValue();

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
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "summary", parameters);

            // Log
            if (!memory_manager::is_bad_read_pointer(PlayerController->MyPlayerInfo()) && PlayerController->MyPlayerInfo())
            {
                std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
                std::wstring wAccount_id = converter.from_bytes(account_id);

                float TotalDamageDealt = DamageDealt.PistolDamage + DamageDealt.ShotgunDamage + DamageDealt.AssaultRifleDamage + DamageDealt.SniperDamage + DamageDealt.GrenadeDamage + DamageDealt.SMGDamage + DamageDealt.GrenadeLauncherDamage + DamageDealt.RPGDamage;
                
                spdlog::info(L"Sent summary event for {} [ACCOUNT_ID: {}, PLACEMENT: {}, DAMAGEDEALT: {}]", PlayerController->MyPlayerInfo()->PlayerName().c_str(), wAccount_id.c_str(), Placement, TotalDamageDealt);
            }
        }
    }
}

void challenges_component::SendElimination(AFortPlayerController* KillerPlayerController, AFortPlayerController* VictimPlayerController, uint8_t DeathCause)
{
    if (KillerPlayerController && VictimPlayerController)
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
                    {"x", std::to_string(VictimPlayerController->K2_GetActorLocation().X)},
                    {"y", std::to_string(VictimPlayerController->K2_GetActorLocation().Y)},
                    {"weapon", weapon},
                    {"victim", victim_account_id}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + killer_account_id, "event:elimination", parameters);
        }
    }
}

void challenges_component::SendLetterInteraction(AFortPlayerController* InteractingController, AProp_QuestInteractable_Letter_Parent_C* Letter)
{
    if (InteractingController && Letter)
    {
        std::string account_id = player_component::get_account_id(InteractingController);

        if (account_id.size() > 0)
        {
            nlohmann::json parameters{
                {"x", Letter->K2_GetActorLocation().X},
                {"y", Letter->K2_GetActorLocation().Y}
            };
            
            gameserver_client::Client->SendEventToChannel("player:" + account_id, "interaction:letter", parameters);
        }
    }   
}
