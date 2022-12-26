#include "include.hpp"
#include "webhook_manager.hpp"

webhook_manager::webhook_manager()
{
#ifdef _RELEASE
    // Create the webhook config file if it doesn't already exist
    const std::string webhook_config_path = std::string(dll_utils::get_current_path()) + "/Config";

    std::filesystem::create_directories(webhook_config_path);

    if (!std::filesystem::exists(webhook_config_path + "/webhooks.json"))
    {
        std::ofstream new_file;

        new_file.open(webhook_config_path + "/webhooks.json");

        const nlohmann::json new_file_body = R"(
            [
                {
                    "name": "webhook-general",
                    "URL": "",
                    "body": "Servers are up, press play or open erafn.org to join :white_check_mark:!"
                },
                {
                    "name": "webhook-mpfeed",
                    "URL": "",
                    "body": "Servers are up, press play or open erafn.org to join :white_check_mark:!\n\n<@&1015696679585976351>"
                },
                {
                    "name": "webhook-adminlogin",
                    "URL": "https://discord.com/api/webhooks/1056707019421257748/Kp04r0HVkHgBKNC-UqLcVAWGyERn-penl3Bdsuw39pitNe5L_U7txEF1K33Fhc7iCMhi",
                    "body": ""
                }
            ]
        )"_json;

        new_file << std::setw(4) << new_file_body << std::endl;

        new_file.close();
    }
#endif
}

void webhook_manager::send_all()
{
#ifdef _RELEASE
    const std::string webhook_config_path = std::string(dll_utils::get_current_path()) + "/Config/webhooks.json";

    if (std::filesystem::exists(webhook_config_path))
    {
        std::ifstream table(webhook_config_path);
        
        nlohmann::json webhook_config = nlohmann::json::parse(table);

        for (auto& elem : webhook_config.items())
        {
            std::string URL = elem.value()["URL"];

            if (URL.find("https://discord.com") != std::string::npos)
            {
                URL = URL.substr(strlen("https://discord.com"));
            }
            
            std::string Body = elem.value()["body"];

            nlohmann::json req_body = R"({
                "content": null,
                "embeds": null,
                "attachments": []
            })"_json;

            req_body["content"] = Body;

            httplib::Client client("https://discord.com");
            client.Post(URL, req_body.dump(), "application/json");
        }
    }
#endif
}

nlohmann::json webhook_manager::get_webhook_by_name(std::string in_name)
{
#ifdef _RELEASE
    const std::string webhook_config_path = std::string(dll_utils::get_current_path()) + "/Config/webhooks.json";

    if (std::filesystem::exists(webhook_config_path))
    {
        std::ifstream table(webhook_config_path);
        
        nlohmann::json webhook_config = nlohmann::json::parse(table);

        for (auto& elem : webhook_config.items())
        {
            std::string name = elem.value()["name"];

            if (name == in_name)
            {
                return elem.value();
            }
        }
    }

    // Return default json
    nlohmann::json webhook_to_return = R"({
                    "name": "",
                    "URL": "",
                    "body": ""
                })"_json;

    webhook_to_return["name"] = in_name;
    
    return webhook_to_return;
#endif
}

void webhook_manager::send_webhook(nlohmann::json payload)
{
#ifdef _RELEASE
    std::string URL = payload["URL"];

    if (URL.find("https://discord.com") != std::string::npos)
    {
        URL = URL.substr(strlen("https://discord.com"));
    }
            
    std::string Body = payload["body"];

    nlohmann::json req_body = R"({
                "content": null,
                "embeds": null,
                "attachments": []
            })"_json;

    req_body["content"] = Body;

    httplib::Client client("https://discord.com");
    client.Post(URL, req_body.dump(), "application/json");
#endif
}


