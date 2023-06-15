#include "include.hpp"
#include "config_manager.hpp"

config_manager::config_manager()
{
    
}

nlohmann::json config_manager::get_server_config()
{
    const std::string config_path = std::string(dll_utils::get_current_path()) + "/Config";

    std::filesystem::create_directories(config_path);
    
    nlohmann::json default_config = R"(
            {
                "server_admins": [],
                "opped_players": [],
                "server_key": "",
                "override_playlist": "",
                "player_cap_override": 100,
                "banned_players": [],
                "admin_passwords": [],
                "scp_enabled": false,
                "warmup_duration": 90,
                "name": "",
                "scheduler_port": -1,
                "loot_pool_changes": []
            }
        )"_json;
    
    if (!std::filesystem::exists(config_path + "/server.json"))
    {
        std::ofstream new_file;

        new_file.open(config_path + "/server.json");

        new_file << std::setw(4) << default_config << std::endl;

        new_file.close();

        return default_config;
    }
    else
    {
        // Get the current config
        std::ifstream table(config_path+"/server.json");
        
        auto config = nlohmann::json::parse(table);

        // make sure the existing config is up-to-date, if not, update it
        for (auto& elem : default_config.items())
        {
            if (!config.contains(elem.key()))
            {
                config[elem.key()] = elem.value();
            }
        }

        // Save the config
        save_server_config(config);

        return config;
    }
}

void config_manager::save_server_config(nlohmann::json in_config)
{
    std::ofstream out_config;

    out_config.open(std::string(dll_utils::get_current_path()) + "/Config/server.json");

    out_config << std::setw(4) << in_config << std::endl;
    
    out_config.close();
}