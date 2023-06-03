#pragma once

#include <map>

#define INIT_POINTER(PointerName, PointerAddress)                                           \
    if (PointerAddress)                                                                     \
        PointerName = reinterpret_cast<decltype(PointerName)>(PointerAddress);              \
    else                                                                                    \
        spdlog::error("Failed to setup pointer '{}', invalid pointer address provided",     \
            #PointerName);

#ifndef WILDCARD
#define WILDCARD 0x100
#endif

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
    static uintptr_t find_function_by_string(const wchar_t* String, std::vector<DWORD> Opcodes = {}, int Offset = 0, bool Backwards = true, bool bIsRelative = true, bool bTakeAddressOfOpcode = false, bool bExactString = false);
    static uintptr_t get_function_start(uintptr_t address);
    static uintptr_t allocate_page_near_address(uintptr_t address, ENewPageLocation location = ENewPageLocation::LOW);
    static uintptr_t get_function_end(uintptr_t function_start);
    static std::vector<uintptr_t> get_all_calls_in_function(uintptr_t function_start);
    static std::vector<uintptr_t> get_all_stripped_calls_in_function(uintptr_t function_start);
    static bool is_valid_function(uintptr_t function);
    static bool is_stripped_function(uintptr_t function);
    static void call_hook(uintptr_t call_instruction_address, uintptr_t hook_address);
    static void lea_hook(uintptr_t lea_instruction_address, uintptr_t hook_address);
    static uintptr_t scan_for(uintptr_t start_address, std::vector<DWORD> Opcodes, uintptr_t optional_end_address = 0, bool Backwards = false);
    static bool does_address_belong_to_func(uintptr_t target_address, uintptr_t function_start);

    template <typename T>
    static void disable_write_to(uintptr_t in_target_address)
    {
        static uintptr_t target_address = in_target_address;
        static bool bCaughtWrite;
        static DWORD OldProtection;
        static T OriginalValue;
	
        static auto Handle = AddVectoredExceptionHandler(true, [](_EXCEPTION_POINTERS* Exception) -> LONG
        {
            if (Exception->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION)
            {
                if (Exception->ExceptionRecord->ExceptionInformation[1] == target_address)
                {
                    bCaughtWrite = true;

                    // Save the original value before writing takes place
                    OriginalValue = *reinterpret_cast<T*>(target_address);
                }
			
                // Restore original protection to prevent a crash
                DWORD OldProtection_;
                VirtualProtect(reinterpret_cast<PVOID>(Exception->ExceptionRecord->ExceptionInformation[1]), 1, OldProtection, &OldProtection_);
			
                Exception->ContextRecord->EFlags |= 0x100ui32;
			
                return EXCEPTION_CONTINUE_EXECUTION;
            }
            if (Exception->ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP)
            {
                if (bCaughtWrite)
                {
                    // Invalidate...
                    bCaughtWrite = false;

                    // Re-set old value
                    *reinterpret_cast<T*>(target_address) = OriginalValue;
                }

                DWORD OldProtection;
                VirtualProtect(reinterpret_cast<PVOID>(target_address), 1, PAGE_READONLY, &OldProtection);
                
                return EXCEPTION_CONTINUE_EXECUTION;
            }
		
            return EXCEPTION_CONTINUE_SEARCH;
        });

        // Make the page in which our address is in read-only
        VirtualProtect(reinterpret_cast<PVOID>(target_address), 1, PAGE_READONLY, &OldProtection);
    }
    
    static bool is_bad_read_pointer(void* pointer_to_read);
};

// small UNWIND_INFO implementation
typedef struct _UNWIND_INFO {
    unsigned char Version : 3;
    unsigned char Flags : 5;
} UNWIND_INFO, *PUNWIND_INFO;