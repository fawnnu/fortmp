#include "include.hpp"
#include "config_manager.hpp"

config_manager::config_manager()
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
                "loot_pool_changes": [],
                "weapon_hotfixes": {},
                "loaded_mutators": [],
                "override_region": "",
                "enable_debug_logs": false,
                "grants": [],
                "nullrhi": false
            }
        )"_json;
    
    if (!std::filesystem::exists(config_path + "/server.json"))
    {
        std::ofstream new_file;

        new_file.open(config_path + "/server.json");

        new_file << std::setw(4) << default_config << std::endl;

        new_file.close();

        current_config = default_config;
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
        std::ofstream out_updated_config;
        out_updated_config.open(config_path + "/server.json");

        out_updated_config << std::setw(4) << config << std::endl;

        out_updated_config.close();

        current_config = config;
    }

    // Save current_config every 60 seconds
    CreateThread(nullptr, 0, [](LPVOID)->DWORD
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
                "loot_pool_changes": [],
                "weapon_hotfixes": {},
                "loaded_mutators": [],
                "override_region": "",
                "enable_debug_logs": false,
                "grants": [],
                "nullrhi": false
            }
        )"_json;
        
        while (true)
        {
            Sleep(60000);
            
            // make sure the existing config is up-to-date, if not, update it
            for (auto& elem : default_config.items())
            {
                if (!current_config.contains(elem.key()))
                {
                    current_config[elem.key()] = elem.value();
                }
            }

            // Save the config
            std::ofstream out_updated_config;
            out_updated_config.open(config_path + "/server.json");

            out_updated_config << std::setw(4) << current_config << std::endl;

            out_updated_config.close();
        }
    }, nullptr,0 , nullptr);
}

nlohmann::json config_manager::get_server_config()
{
    return current_config;
}

void config_manager::save_server_config(nlohmann::json in_config)
{
    current_config = in_config;
}