#pragma once

#include <map>

#define INIT_POINTER(PointerName, PointerAddress)                                           \
    if (PointerAddress)                                                                     \
        PointerName = reinterpret_cast<decltype(PointerName)>(PointerAddress);              \
    else                                                                                    \
        spdlog::error("Failed to setup pointer '{}', invalid pointer address provided",     \
            #PointerName);

static std::map<void*, bool> pointer_cache;

class memory_manager
{
public:
    enum class ENewPageLocation
    {
        LOW = 0,
        HIGH = 1,
        BOTH = 2
    };
    
    static uintptr_t find_pattern(const char* signature, bool bRelative = false, uint32_t offset = 0);
    static uintptr_t find_function_by_string(const wchar_t* String, std::vector<DWORD> Opcodes = {}, int Offset = 0, bool Backwards = true, bool bIsRelative = true);
    static uintptr_t get_function_start(uintptr_t address);
    static uintptr_t allocate_page_near_address(uintptr_t address, ENewPageLocation location = ENewPageLocation::LOW);
    static uintptr_t get_function_end(uintptr_t function_start);
    static std::vector<uintptr_t> get_all_calls_in_function(uintptr_t function_start);
    static bool is_valid_function(uintptr_t function);
    static void call_hook(uintptr_t call_instruction_address, uintptr_t hook_address);
    
    static bool is_bad_read_pointer(void* pointer_to_read);
};

// small UNWIND_INFO implementation
typedef struct _UNWIND_INFO {
    unsigned char Version : 3;
    unsigned char Flags : 5;
} UNWIND_INFO, *PUNWIND_INFO;