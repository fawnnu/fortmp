#pragma once

class storefront
{
public:    
    storefront() = default;

    storefront(httplib::Server& in_app)
    {
        in_app.Get("/fortnite/api/storefront/v2/keychain", [](const httplib::Request& req, httplib::Response& res)
        {
            const nlohmann::json response = R"([
                "46159C748694298198A52DC07476FDA3:4CLHOBqSrmS1RkG/SxZYi8Rc0zCmAKxXIBMMUHDl2ag="
            ])"_json;
            
            res.set_content(response.dump(), "application/json"); 
        });
    }
};