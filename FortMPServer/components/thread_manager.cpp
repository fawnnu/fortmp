#include "include.hpp"
#include "thread_manager.hpp"

void thread_manager::register_thread(DWORD thread, std::string name)
{
    threads[thread] = FThreadInfo(name, false);
}

void thread_manager::unregister_thread(DWORD thread)
{
    threads.erase(threads.find(thread));
}

bool thread_manager::is_thread_suppressed(DWORD thread)
{
    return threads[thread].bThreadSuppressed;
}

void thread_manager::suppress_thread(std::string thread_name)
{
    for (auto& [key, val] : threads)
    {
        if (val.ThreadName == thread_name)
        {
            val.bThreadSuppressed = true;
        }
    }
}