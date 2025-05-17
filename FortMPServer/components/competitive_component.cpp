#include "include.hpp"
#include "competitive_component.hpp"

#include "era_mutators_component.hpp"
#include "log_component.hpp"
#include "player_component.hpp"

competitive_component::competitive_component()
{
    rewards = {
        {
            competitive_payout_type::elimination,
            {-1, 50}
        },
        {
            competitive_payout_type::placement,
            {50, 50}
        },
        {
            competitive_payout_type::placement,
            {40, 50}
        },
        {
            competitive_payout_type::placement,
            {30, 50}
        },
        {
            competitive_payout_type::placement,
            {20, 75}
        },
        {
            competitive_payout_type::placement,
            {15, 100}
        },
        {
            competitive_payout_type::placement,
            {10, 150}
        },
        {
            competitive_payout_type::placement,
            {5, 150}
        },
        {
            competitive_payout_type::placement,
            {4, 200}
        },
        {
            competitive_payout_type::placement,
            {3, 200}
        },
        {
            competitive_payout_type::placement,
            {2, 300}
        },
        {
            competitive_payout_type::placement,
            {1, 500}
        }
    };
}

competitive_component::competitive_payout_info competitive_component::get_reward(competitive_payout_type in_type,
    int value, bool* success)
{
    for (auto& [type, info] : rewards)
    {
        if (type == in_type && info.value == value)
        {
            if (success)
            {
                *success = true;
            }
            
            return info;
        }
    }

    if (success)
    {
        *success = false;
    }
    
    return {};
}

void competitive_component::on_elimination(AFortPlayerControllerAthena* player)
{
    if (era_mutators_component::IsActive("competitive"))
    {
        if (player)
        {
            bool success = false;
            competitive_payout_info reward = get_reward(competitive_payout_type::elimination, -1, &success);

            if (success)
            {
                player->EraPlayerInfo().Competitive_PendingPayoutCurrency += reward.amount;

                if (log_component::enable_debug_logs)
                {
                    spdlog::info(L"competitive_component::on_elimination: Player {} has gained {} currency (Total: {}).", player->GetName().c_str(), reward.amount, player->EraPlayerInfo().Competitive_PendingPayoutCurrency);
                }
            }
        }
    }
}

void competitive_component::on_placement(AFortPlayerControllerAthena* player, int placement)
{
    if (era_mutators_component::IsActive("competitive"))
    {
        if (player)
        {
            bool success = false;
            competitive_payout_info reward = get_reward(competitive_payout_type::placement, placement, &success);

            if (success)
            {
                player->EraPlayerInfo().Competitive_PendingPayoutCurrency += reward.amount;

                if (log_component::enable_debug_logs)
                {
                    spdlog::info(L"competitive_component::on_placement: Player {} has gained {} currency (Total: {}).", player->GetName().c_str(), reward.amount, player->EraPlayerInfo().Competitive_PendingPayoutCurrency);
                }
            }
        }
    }
}

void competitive_component::on_summary(AFortPlayerControllerAthena* player)
{
    if (era_mutators_component::IsActive("competitive"))
    {
        if (auto account_id = player_component::get_account_id(player); account_id.size())
        {
            try
            {
                if (log_component::enable_debug_logs)
                {
                    spdlog::info(L"competitive_component::on_summary: Player {} accumulated {} currency.", player->GetName().c_str(), player->EraPlayerInfo().Competitive_PendingPayoutCurrency);
                }
            
                httplib::Client client(API_URL);
                httplib::Headers headers = {
                    { "x-era-token", "bn8HUsAfhogEq6E57ledvyGReK9n8E7XP53AbRomzvItAexNB09aXiTgFhoPvUD1" }
                };
    
                nlohmann::json Payload = {
                    {"account_id", account_id},
                    {"count", player->EraPlayerInfo().Competitive_PendingPayoutCurrency}
                };

                client.set_read_timeout(15, 0); // 15 seconds
                client.set_write_timeout(15, 0); // 15 seconds
                auto Result = client.Patch("/api/internal/users/currency", headers, Payload.dump(), "application/json");
            }
            catch (std::exception& e)
            {
                spdlog::error("competitive_component::on_placement: Error {}.", e.what());
            }
        }
    }
}

