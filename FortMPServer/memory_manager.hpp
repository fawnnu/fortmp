#pragma once

class memory_manager
{
public:
    static intptr find_pattern(const char* signature, bool bRelative = false, uint32_t offset = 0);

    static intptr find_function_by_string(const wchar_t* String, std::vector<DWORD> Opcodes = {}, int Offset = 0, bool Backwards = true);
};
