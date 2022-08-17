#pragma once

class auth
{
public:
    inline static std::string local_username;
    
    inline static nlohmann::json response_client_credentials = R"(
        [{
            "access_token": "gWHSKIlhPMbpZqFmt95MsuB6RH6BgAHC",
            "expires_in": 28800,
            "expires_at": "9999-12-31T23:59:59.999Z",
            "token_type": "bearer",
            "client_id": "FMP",
            "internal_client": true,
            "client_service": "fortnite"
        }]
    )"_json;
    
    auth() = default;

    auth(httplib::Server& in_app)
    {
        in_app.Post("/account/api/oauth/token", [](const httplib::Request& req, httplib::Response& res)
        {
            if (req.has_param("grant_type"))
            {
                if (req.get_param_value("grant_type") == "client_credentials")
                {
                    res.set_content(response_client_credentials.dump(), "application/json");
                }
                else
                {
                    std::string email = req.get_param_value("username");

                    // make the username be the first part of the email
                    local_username = email.substr(0, email.find("@"));

                    const nlohmann::json response{
                            {"access_token", "gWHSKIlhPMbpZqFmt95MsuB6RH6BgAHC"},
                            {"expires_in", 28800},
                            {"expires_at", "9999-12-31T23:59:59.999Z"},
                            {"token_type", "bearer"},
                            {"account_id", "FMP"},
                            {"client_id", "FMP"},
                            {"internal_client", true},
                            {"client_service", "fortnite"},
                            {"displayName", local_username},
                            {"app", "fortnite"},
                            {"in_app_id", local_username},
                            {"device_id", "FMP"}
                    };

                    res.set_content(response.dump(), "application/json");
                }
            }
            else
            {
                res.status = 400;
            }
        });

        in_app.Get(R"(/account/api/public/account/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            std::string account_id = req.matches[1];
            
            const nlohmann::json response = {
                {"id", account_id},
                {"displayName", local_username},
                {"externalAuths", nlohmann::json({})}
            };

            res.set_content(response.dump(), "application/json");
        });
    }
};