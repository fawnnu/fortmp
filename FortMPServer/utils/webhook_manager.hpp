#pragma once

class webhook_manager
{
public:
    webhook_manager();

    static void send_all();
    static nlohmann::json get_webhook_by_name(std::string in_name);
    static void send_webhook(nlohmann::json payload);
};
