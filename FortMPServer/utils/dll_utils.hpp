#pragma once

class dll_utils
{
public:
    dll_utils();
    
    static const char* get_current_path();

    static void create_spdlog_sink();

    static nlohmann::json get_banlist();

    static void send_webhook_message(std::string target_webhook, std::string message);

    inline static std::string general_chat_webhook;
    inline static std::string multiplayer_news_webhook;
};