#include "include.hpp"
#include "director_client.hpp"

#include "config_manager.hpp"

director_client::director_client()
{
    // First of all, parse commandline
    LPSTR CommandLine = GetCommandLineA();
    std::string sCommandLine = std::string(CommandLine);

    if (sCommandLine.find("-gsuuid=") != std::string::npos)
    {
        sCommandLine = sCommandLine.substr(sCommandLine.find("-gsuuid=") + 8);
        if (sCommandLine.find(" ") != std::string::npos)
        {
            sCommandLine = sCommandLine.substr(0, sCommandLine.find(" "));
        }

        // Key
        key = sCommandLine;

        // Create new UDP client and connect to the director (localhost:6000)
        client = new eudp_client("127.0.0.1", 6000, key, [](eudp_client* client)
        {
            client->On("config", [](eudp_client* client, nlohmann::json payload)
            {
                // Set config
                config_manager::config = payload;
            });
        });
    }
}
