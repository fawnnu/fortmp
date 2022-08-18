#pragma once

class api_utils
{
public:
    api_utils() = default;

    static std::vector<std::string> split_str(std::string target, std::string delimiter);

    static int get_season_number(std::string user_agent);
};
