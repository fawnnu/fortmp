#include "include.hpp"
#include "config_manager.hpp"

#include "director_client.hpp"

nlohmann::json config_manager::get_server_config()
{
    return config;
}

void config_manager::save_server_config(nlohmann::json in_config)
{
    config = in_config;

    // Send config to director
    if (director_client::client)
    {
        director_client::client->Send(eudp_packet({
            {"type", "config_update"},
            {"payload", config}
        }));
    }
}