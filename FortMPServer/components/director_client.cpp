#include "include.hpp"
#include "director_client.hpp"

#include "config_manager.hpp"
#include "loader_component.hpp"
#include "log_component.hpp"
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
                config_manager::bReceivedConfig = true;
                config_manager::config = payload;
            });

            client->On("power_change", [](eudp_client*, nlohmann::json payload)
            {
                try
                {
                    if (payload["status"] == "off_graceful")
                    {
                        spdlog::warn("Received power_change [off_graceful] from director. bPendingShutdown = true.");
                        loader_component::bPendingShutdown = true;
                    }
                }
                catch (nlohmann::json& exception)
                {
                }
            });

            client->On("entity", [](eudp_client*, nlohmann::json payload)
            {
                if (log_component::enable_debug_logs)
                {
                    spdlog::info("Received entity control message. Unpacking it and handling team.");
                }
                
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

                        if (log_component::enable_debug_logs)
                        {
                            spdlog::info("Received entity control message for player '{}'. Message: {}", account_id.c_str(), payload.dump().c_str());
                        }
                        
                        // Create encryption token
                        std::string sEncryptionToken = std::string(account_id) + ":" + std::string(backfill);
                        std::wstring wEncryptionToken = std::wstring(sEncryptionToken.begin(), sEncryptionToken.end());

                        // Add to the valid EncryptionToken list
                        player_component::EncryptionTokenList.push_back(wEncryptionToken);
                        
                        // set match id for player
                        player_component::Matches[account_id] = match;
                        
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
                catch (std::exception& exception)
                {
                    if (log_component::enable_debug_logs)
                    {
                        spdlog::error("Client received entity message with error: {}", exception.what());
                    }
                }
            });
        });
    }
}

void director_client::send_ready_event()
{
    if (client)
    {
        client->Send(eudp_packet({
                {"type", "ready"},
                {"payload", {
                    {"port", gameserver_client::Client->GameServer_Status.port}
                }}
            }));
        client->WaitFor("ready_ack", [](class eudp_client* client, nlohmann::json payload)
        {
            spdlog::info("Director acknowledged ready event.");
        });

        // Make sure the ready event is acknowledged
        CreateThread(nullptr, 0, [](LPVOID)
        {
            // Wait 5 seconds
            Sleep(5000);

            // Check if we're still waiting for ready_ack, if so, clear the request
            if (auto result = client->OneTimeEvents.find("ready_ack"); result != director_client::client->OneTimeEvents.end())
            {
                client->OneTimeEvents.erase(result);

                // Log
                spdlog::error("Director did NOT acknowledge the 'ready' event [Timeout: 5s]. Re-sending...");

                // Recall send_ready_event
                send_ready_event();
            }
            
            return static_cast<DWORD>(0);
        }, nullptr, 0, nullptr);
    }
}

void director_client::wait_for_config()
{
    static int FailureCount = 0;
    
    while (!config_manager::bReceivedConfig)
    {
        spdlog::error("director_client did not receive 'config' event from director. Retrying in 100ms...");

        FailureCount++;
        if (FailureCount >= 100)
        {
            // Close the gameserver
            if (client)
            {
                client->Close();
            }
            RequestExitWithStatus(true, 0);
        }
        
        Sleep(100);
    }
}
