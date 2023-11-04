#include "include.hpp"
#include "director_client.hpp"

#include "config_manager.hpp"
#include "loader_component.hpp"
#include "player_component.hpp"
#include "team_component.hpp"

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

            client->On("power_change", [](eudp_client*, nlohmann::json payload)
            {
                try
                {
                    if (payload["status"] == "off_graceful")
                    {
                        loader_component::bPendingShutdown = true;
                    }
                }
                catch (nlohmann::json& exception)
                {
                }
            });

            client->On("entity", [](eudp_client*, nlohmann::json payload)
            {
                try
                {
                    std::string ticket = payload["ticket"];
                    bool fill = payload["fill"];
                    std::string backfill = payload["backfill"];
                    std::string match = payload["match"];

                    for (auto& player : payload["players"].items())
                    {
                        // Get account id
                        std::string account_id = player.value();

                        // Create encryption token
                        std::string sEncryptionToken = std::string(account_id) + ":" + std::string(backfill);
                        std::wstring wEncryptionToken = std::wstring(sEncryptionToken.begin(), sEncryptionToken.end());

                        // Add to the valid EncryptionToken list
                        player_component::EncryptionTokenList.push_back(wEncryptionToken);

                        // Properly handle incoming fill and ticket
                        if (ticket.length() > 0)
                        {
                            const std::wstring wTicket = std::wstring(ticket.begin(), ticket.end());
                        
                            auto& new_team_instance = team_component::TeamsMap[wTicket];
                            new_team_instance.TeamMembers.push_back(wEncryptionToken);
                            new_team_instance.fill = fill;
                        }
                    }
                }
                catch (nlohmann::json& exception)
                {
                }
            });
        });
    }
}
