#pragma once

// async thread management
struct FThreadInfo
{
    FThreadInfo() = default;
    
    FThreadInfo(std::string InThreadName, bool InbThreadSuppressed) :
        ThreadName(InThreadName), bThreadSuppressed(InbThreadSuppressed)
    {
        
    }
    
    std::string ThreadName;
    bool bThreadSuppressed;
};

class thread_manager
{
public:
    thread_manager() = default;
    
    static void register_thread(DWORD thread, std::string name);
    static void unregister_thread(DWORD thread);
    static bool is_thread_suppressed(DWORD thread);
    static void suppress_thread(std::string name);
    
    static inline std::map<DWORD, FThreadInfo> threads;
};

class fort_thread_manager : public thread_manager
{
public:
    static inline int GIndex;
};
