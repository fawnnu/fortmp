#pragma once

#include <spdlog/sinks/base_sink.h>

template<typename Mutex>
class fmp_log_sink : public spdlog::sinks::base_sink<Mutex>
{
public:
    
protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    }

    void flush_() override 
    {
        std::cout << std::flush;
    }
};

class log_component
{
public:
    log_component();
};
