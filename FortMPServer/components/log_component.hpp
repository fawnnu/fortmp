#pragma once

#include <spdlog/sinks/base_sink.h>

#include "imgui/imgui.h"

class log_component
{
public:
    log_component();
    
    struct log_msg
    {
        log_msg() = default;

        log_msg(std::string in_message, spdlog::level::level_enum in_level);
        
        std::string message;

        unsigned color;

        bool processed;
    };
    
    static inline std::vector<log_msg> messages;
    static inline std::vector<log_msg> bp_messages;

    static void add_message_safe(std::string message, spdlog::level::level_enum level);
    static void add_message_safe_bp(std::string message, spdlog::level::level_enum level);

    static inline bool enable_debug_logs;

    static inline std::shared_ptr<spdlog::logger> bp_logger;
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

template<typename Mutex>
class bp_log_sink : public spdlog::sinks::base_sink<Mutex>
{
public:
    
protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

        log_component::add_message_safe_bp(fmt::to_string(formatted), msg.level);
    }

    void flush_() override 
    {
        std::cout << std::flush;
    }
};
