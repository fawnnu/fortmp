#include "include.hpp"
#include "config_manager.hpp"

config_manager::config_manager()
{
    
}

nlohmann::json config_manager::get_server_config()
{
    const std::string config_path = std::string(dll_utils::get_current_path()) + "/Config";

    std::filesystem::create_directories(config_path);

    if (!std::filesystem::exists(config_path + "/server.json"))
    {
        std::ofstream new_file;

        new_file.open(config_path + "/server.json");

        const nlohmann::json new_file_body = R"(
            {
                "server_admins": [],
                "opped_players": [],
                "server_key": "",
                "override_playlist": "",
                "player_cap_override": 100
            }
        )"_json;

        new_file << std::setw(4) << new_file_body << std::endl;

        new_file.close();
    }

    std::ifstream table(config_path+"/server.json");
        
    return nlohmann::json::parse(table);
}

void config_manager::save_server_config(nlohmann::json in_config)
{
    std::ofstream out_config;

    out_config.open(std::string(dll_utils::get_current_path()) + "/Config/server.json");

    out_config << std::setw(4) << in_config << std::endl;
    
    out_config.close();
}