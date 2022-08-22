#pragma once

class dll_utils
{
public:
    static const char* get_current_path();

    static void create_spdlog_sink();

    static nlohmann::json get_banlist();
};