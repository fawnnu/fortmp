#include "include.hpp"
#include "api_utils.hpp"

std::vector<std::string> api_utils::split_str(std::string target, std::string delimiter)
{
    std::vector<std::string> result;

    while (target.find(delimiter) != std::string::npos)
    {
        uint8_t position = target.find(delimiter);

        result.push_back(target.substr(0, position));

        target.erase(0, position + delimiter.length());
    }

    result.push_back(target);

    return result;
}

int api_utils::get_season_number(std::string user_agent)
{
    int season_number = 0;
    int CL = -1;
    
    auto splitted_string = split_str(user_agent, "-");

    if (splitted_string.size() > 3)
    {
        auto local_splitted_string = split_str(splitted_string[3], ",");

        if (local_splitted_string.size() > 0)
        {
            try {
                CL = std::stoi(local_splitted_string[0]);
            }
            catch (...)
            {
                CL = -1;
            }
        }

        local_splitted_string = split_str(splitted_string[3], " ");

        if (local_splitted_string.size() > 0 && !CL)
        {
            try {
                CL = std::stoi(local_splitted_string[0]);
            }
            catch (...)
            {
                CL = -1;
            }
        }
    }
    
    if (splitted_string.size() > 1 && !CL)
    {
        auto local_splitted_string = split_str(splitted_string[1], "+");

        if (local_splitted_string.size() > 0)
        {
            try
            {
                CL = std::stoi(local_splitted_string[0]);
            }
            catch (...)
            {
                CL = -1;
            }
        }
    }

    if (CL)
    {
        if (CL < 3807424)
        {
            season_number = 1;
        }
        else if (CL < 3901517)
        {
            season_number = 2;
        }
        else
        {
            splitted_string = split_str(user_agent, "Release-");

            if (splitted_string.size() > 1)
            {
                auto local_splitted_string = split_str(splitted_string[1], "-");

                if (local_splitted_string.size() > 0)
                {
                    auto build_split = split_str(local_splitted_string[0], ".");

                    if (build_split.size() > 0)
                    {
                        try
                        {
                            season_number = std::stoi(build_split[0]);
                        }
                        catch (...)
                        {
                            season_number = 0;
                        }
                    }
                }
            }
        }
    }
    
    return season_number;
}

std::string api_utils::get_ip_from_hostname(std::string hostname_str)
{
    hostent* hostname = gethostbyname(hostname_str.c_str());
    
    if(hostname)
    {
        return std::string(inet_ntoa(**(in_addr**)hostname->h_addr_list));
    }

    return hostname_str;
}