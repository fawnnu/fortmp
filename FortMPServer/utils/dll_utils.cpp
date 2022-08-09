#include "include.hpp"
#include "dll_utils.hpp"

#include "spdlog/sinks/basic_file_sink.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

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

    std::string current_dll_path(get_current_path());
    current_dll_path.append("/FortMP/FortMPLog_%m-%d_%H-%M-%S.log");

    std::stringstream local_time_str;
    local_time_str << std::put_time(&local_time, current_dll_path.c_str());

    // Add a file sink
    spdlog::default_logger()->sinks().push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(local_time_str.str(), false));

    spdlog::default_logger()->flush_on(spdlog::level::debug);
}
