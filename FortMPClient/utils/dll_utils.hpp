#pragma once

class dll_utils
{
public:
    static const char* get_current_path();

    static toml::table get_current_config();

    static std::string get_redirect_url();

    static std::string get_local_username();
    static std::string get_gameserver_addr();
    static int get_gameserver_port();
};