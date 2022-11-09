#pragma once

#include <spdlog/sinks/base_sink.h>

#include "imgui/imgui.h"

class log_component
{
public:
    struct log_msg
    {
        log_msg() = default;

        log_msg(std::string in_message, spdlog::level::level_enum in_level);
        
        std::string message;

        unsigned color;

        bool processed;
    };
    
    static inline std::vector<log_msg> messages;

    static void add_message_safe(std::string message, spdlog::level::level_enum level);
};

template<typename Mutex>
class fmp_log_sink : public spdlog::sinks::base_sink<Mutex>
{
public:
    
protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

        log_component::add_message_safe(fmt::to_string(formatted), msg.level);
    }

    void flush_() override 
    {
        std::cout << std::flush;
    }
};
