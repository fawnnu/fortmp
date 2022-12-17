#include "include.hpp"
#include "dll_utils.hpp"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

dll_utils::dll_utils()
{
}

const char* dll_utils::get_current_path()
{
    char dll_path[MAX_PATH];

    ZeroMemory(dll_path, sizeof dll_path);

    GetModuleFileNameA(reinterpret_cast<HINSTANCE>(&__ImageBase), dll_path, _countof(dll_path));

    const std::string dll_path_str(dll_path);
    
    return dll_path_str.substr(0, dll_path_str.find_last_of("/\\")).c_str();
}