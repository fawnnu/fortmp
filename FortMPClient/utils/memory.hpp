#pragma once

#include <map>
#include <vector>

#define INIT_POINTER(PointerName, PointerAddress)                                           \
    if (PointerAddress)                                                                     \
        PointerName = reinterpret_cast<decltype(PointerName)>(PointerAddress);              \
    else                                                                                    \
        spdlog::error(XOR("Failed to setup pointer '{}', invalid pointer address provided"),     \
            XOR(#PointerName));

static std::map<void*, bool> pointer_cache;

class memory_manager
{
public:
    static uintptr_t find_pattern(const char* signature, bool bRelative = false, uint32_t offset = 0);
    static uintptr_t find_function_by_string(const wchar_t* String, std::vector<DWORD> Opcodes = {}, int Offset = 0, bool Backwards = true);

    static bool is_bad_read_pointer(void* pointer_to_read);
};