#pragma once

class fortnitegame
{
public:
    fortnitegame() = default;

    fortnitegame(httplib::Server& in_app)
    {
        in_app.Post(R"(/fortnite/api/game/v2/tryPlayOnPlatform/account/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            res.set_content("true", "text/plain");
        });

        in_app.Get("/fortnite/api/game/v2/enabled_features", [](const httplib::Request& req, httplib::Response& res)
        {
            res.set_content("[]", "application/json"); 
        });

        in_app.Get("/fortnite/api/game/v2/world/info", [](const httplib::Request& req, httplib::Response& res)
        {
            res.set_content("{}", "application/json");
        });

        in_app.Get(R"(/fortnite/api/game/v2/twitch/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            res.set_content("{}", "application/json"); 
        });

        in_app.Get(R"(/fortnite/api/matchmaking/session/findPlayer/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            res.status = 204; 
        });

        in_app.Get(R"(/fortnite/api/v2/versioncheck/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            const nlohmann::json response = {
                {"type", "NO_UPDATE"}
            };

            res.set_content(response.dump(), "application/json");
        });

        in_app.Get(R"(/fortnite/api/v2/versioncheck(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            const nlohmann::json response = {
                {"type", "NO_UPDATE"}
            };
            
            res.set_content(response.dump(), "application/json");
        });

        in_app.Get(R"(/fortnite/api/versioncheck(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            const nlohmann::json response = {
                {"type", "NO_UPDATE"}
            };
            
            res.set_content(response.dump(), "application/json");
        });
    }
};