#pragma once

class auth
{
public:
    inline static std::string local_username;
    
    auth() = default;

    auth(httplib::Server& in_app)
    {
        in_app.Post("/account/api/oauth/token", [](const httplib::Request& req, httplib::Response& res)
        {
            // we'll use the current timestamp to generate random IDs
            const int64_t current_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            
            const nlohmann::json response{
                            {"access_token", "gWHSKIlhPMbpZqFmt95MsuB6RH6BgAHC"},
                            {"expires_in", 28800},
                            {"expires_at", "9999-12-31T23:59:59.999Z"},
                            {"token_type", "bearer"},
                            {"account_id", std::to_string(std::hash<std::string>{}(std::to_string(current_timestamp)))},
                            {"client_id", std::to_string(std::hash<std::string>{}(std::to_string(current_timestamp + 1)))},
                            {"internal_client", true},
                            {"client_service", "fortnite"},
                            {"displayName", local_username},
                            {"app", "fortnite"},
                            {"in_app_id", local_username},
                            {"device_id", std::to_string(std::hash<std::string>{}(std::to_string(current_timestamp + 2)))}
                    };

            res.set_content(response.dump(), "application/json");
        });

        in_app.Get(R"(/account/api/public/account/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            std::string account_id = req.matches[1];
            
            const nlohmann::json response = {
                {"id", account_id},
                {"displayName", local_username},
                {"name", "FortMP"},
                {"lastName", "User"},
                {"email", "fortmp@erafn.org"},
                {"failedLoginAttempts", 0},
                {"numberOfDisplayNameChanges", 0},
                {"ageGroup", "UNKNOWN"},
                {"headless", false},
                {"country", "GB"},
                {"preferredLanguage", "en"},
                {"tfaEnabled", false}
            };

            res.set_content(response.dump(), "application/json");
        });

        in_app.Get(R"(/account/api/public/account/(.*)/externalAuths)", [](const httplib::Request& req, httplib::Response& res)
        {
           res.set_content("[]", "application/json"); 
        });

        in_app.Get("/account/api/public/account", [](const httplib::Request& req, httplib::Response& res)
        {
            std::string ID;

            if (req.has_param("accountId"))
            {
                ID = req.get_param_value("accountId");
            }

            const nlohmann::json response = {{
                {"id", ID},
                {"displayName", local_username},
                {"externalAuths", nlohmann::json({})}
            }};

            res.set_content(response.dump(), "application/json");
        });

        in_app.Delete("/account/api/oauth/sessions/kill", [](const httplib::Request&, httplib::Response& res)
        {
            res.status = 204;
        });
        
        in_app.Delete(R"(/account/api/oauth/sessions/kill/(.*))", [](const httplib::Request&, httplib::Response& res)
        {
            res.status = 204;
        });
    }
};