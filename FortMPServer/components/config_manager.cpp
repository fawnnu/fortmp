#include "include.hpp"
#include "config_manager.hpp"

nlohmann::json config_manager::get_server_config()
{
    return config;
}

void config_manager::save_server_config(nlohmann::json in_config)
{
    config = in_config;
}