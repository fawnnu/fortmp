#pragma once

class config_manager
{
public:
    static nlohmann::json get_server_config();
    static void save_server_config(nlohmann::json in_config);

    static inline nlohmann::json config;
    static inline bool bReceivedConfig = false;
};
