#pragma once

class config_manager
{
public:
    config_manager();

    static nlohmann::json get_server_config();
    static void save_server_config(nlohmann::json in_config);

    static inline nlohmann::json current_config;
};
