#pragma once

class matchmaking
{
public:
    struct server_info
    {
        std::string ip_address;
        int port;
        int alive_players;
    };

    inline static std::vector<server_info> servers;
    
    matchmaking() = default;

    matchmaking(httplib::Server& in_app)
    {
        in_app.Post("/RegisterServer", [](const httplib::Request& req, httplib::Response& res)
        {
            // parse request
            nlohmann::json req_body = nlohmann::json::parse(req.body);

            std::string port = req_body["port"];
            std::string alive_players = req_body["alive_players"];

            server_info new_server;
            new_server.ip_address = req.remote_addr;
            new_server.port = std::stoi(port);
            new_server.alive_players = std::stoi(alive_players);

            servers.push_back(new_server);

            res.status = 200;

            spdlog::info("Server with IP address '{}' registered", req.remote_addr.c_str());
        });

        in_app.Get("/FindServer", [](const httplib::Request& req, httplib::Response& res)
        {
            spdlog::info("Request to find server received from '{}'", req.remote_addr);
            
            for (auto& server : servers)
            {
                if (server.alive_players < 100)
                {
                    const nlohmann::json response = {
                        {"ip_address", server.ip_address},
                        {"port", server.port}
                    };

                    res.status = 200;
                    res.set_content(response.dump(), "application/json");

                    return;
                }
            }

            res.status = 200;
            res.set_content("[]", "application/json");
        });

        in_app.Post("/UpdateServer", [](const httplib::Request& req, httplib::Response& res)
        {
            spdlog::info("Updating server '{}' data", req.remote_addr.c_str());
            
            for (auto& server : servers)
            {
                if (server.ip_address == req.remote_addr)
                {
                    // parse request
                    nlohmann::json req_body = nlohmann::json::parse(req.body);

                    std::string alive_players = req_body["alive_players"];

                    server.alive_players = std::stoi(alive_players);
                    
                    break;
                }
            }
            
            res.status = 200;
        });

        in_app.Delete("/RemoveServer", [](const httplib::Request& req, httplib::Response& res)
        {
            spdlog::info("Removing server '{}' from server list", req.remote_addr.c_str());
            
            for (auto i = 0; i < servers.size(); i++)
            {
                if (servers[i].ip_address == req.remote_addr)
                {
                    servers.erase(servers.begin() + i);
                    
                    break;
                }
            }

            res.status = 200;
        });
    }
};
