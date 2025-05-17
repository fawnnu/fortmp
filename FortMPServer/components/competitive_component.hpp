#pragma once

class competitive_component
{
private:
    enum class competitive_payout_type : uint8_t
    {
        elimination,
        placement
    };

    struct competitive_payout_info
    {
        int value;
        int amount;
    };

    static inline std::vector<std::pair<competitive_payout_type, competitive_payout_info>> rewards;
    
public:
    competitive_component();

    static competitive_payout_info get_reward(competitive_payout_type type, int value, bool* success);

    static void on_elimination(AFortPlayerControllerAthena* player);

    static void on_placement(AFortPlayerControllerAthena* player, int placement);
    
    static void on_summary(AFortPlayerControllerAthena* player);
};
