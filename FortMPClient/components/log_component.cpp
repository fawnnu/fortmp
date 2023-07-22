#include "include.hpp"
#include "log_component.hpp"

#include <spdlog/sinks/basic_file_sink.h>

log_component::log_component()
{
    // Setup file sink for logging
    const time_t current_time = std::time(nullptr);
    
    tm local_time;

    localtime_s(&local_time, &current_time);
    
    std::string log_file_path(dll_utils::get_current_path());
    log_file_path.append(XOR("/FortMP/FortMP.log"));

    if (std::filesystem::exists(log_file_path))
    {
        std::string backup_log_path = log_file_path.substr(0, log_file_path.find(XOR("FortMP.log")));
        backup_log_path.append(XOR("FortMPLog_Backup_%m-%d_%H-%M-%S.log"));

        std::stringstream backup_log_filename;
        backup_log_filename << std::put_time(&local_time, backup_log_path.c_str());

        std::error_code ec;
        std::filesystem::rename(log_file_path, backup_log_filename.str(), ec);
        if (ec)
        {
            int FileIndex = 2;
            bool bLastExists = false;
            while (std::filesystem::exists(std::string(dll_utils::get_current_path()).append(XOR("/FortMP/FortMP-")).append(std::to_string(FileIndex)).append(XOR(".log"))))
            {
                std::string log_file_path_ = std::string(dll_utils::get_current_path()).append(XOR("/FortMP/FortMP-")).append(std::to_string(FileIndex)).append(XOR(".log"));
                std::string backup_log_path_ = log_file_path_.substr(0, log_file_path_.find(XOR("FortMP-")));
                backup_log_path_.append(XOR("FortMPLog_")).append(std::to_string(FileIndex)).append(XOR("_Backup_%m-%d_%H-%M-%S.log"));

                std::stringstream backup_log_filename_;
                backup_log_filename_ << std::put_time(&local_time, backup_log_path_.c_str());
                
                std::error_code ec_;
                std::filesystem::rename(log_file_path_, backup_log_filename_.str(), ec_);
                if (ec_)
                {
                    FileIndex++;
                }
                else
                {
                    log_file_path = log_file_path_;
                    bLastExists = true;
                    break;
                }
            }

            if (!bLastExists)
            {
                log_file_path = std::string(dll_utils::get_current_path()).append(XOR("/FortMP/FortMP-")).append(std::to_string(FileIndex)).append(XOR(".log"));
            }
        }
    }

    // Add a file sink
    spdlog::default_logger()->sinks().push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_path, false));

    // Add a custom sink
    spdlog::default_logger()->sinks().push_back(std::make_shared<fmp_log_sink<std::mutex>>());

    spdlog::default_logger()->flush_on(spdlog::level::debug);
}

