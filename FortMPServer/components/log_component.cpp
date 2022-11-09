#include "include.hpp"
#include "log_component.hpp"

log_component::log_msg::log_msg(std::string in_message, spdlog::level::level_enum in_level)
{
    processed = false;
    
    message = in_message;

    switch (in_level)
    {
    case spdlog::level::level_enum::info:
        {
            color = IM_COL32(255,255,255,255);

            break;
        }
    case spdlog::level::level_enum::warn:
        {
            color = IM_COL32(255,200,85,255);

            break;
        }
    case spdlog::level::level_enum::err:
        {
            color = IM_COL32(241,78,86,255);

            break;
        }
    default:
        {
            break;
        }
    }
}

void log_component::add_message_safe(std::string message, spdlog::level::level_enum level)
{
    if (messages.size() > 100)
    {
        messages.erase(messages.begin());
    }

    messages.push_back(log_msg(message, level));
}
