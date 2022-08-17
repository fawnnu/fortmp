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
    }
};