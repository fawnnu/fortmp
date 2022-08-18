#pragma once

class cloudstorage
{
public:
    cloudstorage() = default;

    cloudstorage(httplib::Server& in_app)
    {
        in_app.Get(R"(/fortnite/api/cloudstorage/user/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            res.set_content("[]", "application/json");
        });
        
        in_app.Get("/fortnite/api/cloudstorage/system", [](const httplib::Request& req, httplib::Response& res)
        {
            res.set_content("[]", "application/json");
        });
    }
};