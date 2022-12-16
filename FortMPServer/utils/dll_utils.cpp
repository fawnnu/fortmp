#include "include.hpp"
#include "dll_utils.hpp"

#include "spdlog/sinks/basic_file_sink.h"
#include "../components/log_component.hpp"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

dll_utils::dll_utils()
{
    general_chat_webhook = "/api/webhooks/1012005016950812722/3XTunDV6Hk-CuaT-KRL45MNVgQxHYbWz9wXGUlLaS8OY5HfhjImEkufnbMeT9SLreYeL";   
    multiplayer_news_webhook = "/api/webhooks/1012756591176589374/TnSPZPBx-LLp-WR74Q6xppZ4KfzVia4qO18UWWBgbk5598L83KmZOeODMvxnCy6ynzQs";   
}

const char* dll_utils::get_current_path()
{
    char dll_path[MAX_PATH];

    ZeroMemory(dll_path, sizeof dll_path);

    GetModuleFileNameA(reinterpret_cast<HINSTANCE>(&__ImageBase), dll_path, _countof(dll_path));

    const std::string dll_path_str(dll_path);
    
    return dll_path_str.substr(0, dll_path_str.find_last_of("/\\")).c_str();
}

void dll_utils::create_spdlog_sink()
{
    // Setup file sink for logging
    const time_t current_time = std::time(nullptr);
    
    tm local_time;

    localtime_s(&local_time, &current_time);
    
    std::string log_file_path(get_current_path());
    log_file_path.append("/FortMP/FortMP.log");

    if (std::filesystem::exists(log_file_path))
    {
        std::string backup_log_path = log_file_path.substr(0, log_file_path.find("FortMP.log"));
        backup_log_path.append("FortMPLog_Backup_%m-%d_%H-%M-%S.log");

        std::stringstream backup_log_filename;
        backup_log_filename << std::put_time(&local_time, backup_log_path.c_str());

        std::filesystem::rename(log_file_path, backup_log_filename.str());
    }

    // Add a file sink
    spdlog::default_logger()->sinks().push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_path, false));

    // Add a custom sink
    spdlog::default_logger()->sinks().push_back(std::make_shared<fmp_log_sink<std::mutex>>());

    spdlog::default_logger()->flush_on(spdlog::level::debug);
}

nlohmann::json dll_utils::get_banlist()
{
    std::string ban_list_path = std::string(get_current_path()) + "/FortMP";

    // Create directory if it doesn't exist
    std::filesystem::create_directories(ban_list_path);

    if (!std::filesystem::exists(ban_list_path + "/Bans.json"))
    {
        std::ofstream new_file;

        new_file.open(ban_list_path + "/Bans.json");

        nlohmann::json new_list;
        new_list["Bans"] = {""};
        
        new_file << std::setw(4) << new_list << std::endl;

        new_file.close();
    }

    std::ifstream table(ban_list_path + "/Bans.json");
    
    return nlohmann::json::parse(table);
}

void dll_utils::send_webhook_message(std::string target_webhook, std::string message)
{
#ifdef _RELEASE
    httplib::Client client("https://discord.com");
    
    client.Post(target_webhook, message, "application/json");
#endif
}