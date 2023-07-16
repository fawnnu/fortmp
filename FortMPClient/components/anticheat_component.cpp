#include "include.hpp"
#include "anticheat_component.hpp"

#include "hwid_component.hpp"
#include "ssl_bypass_component.hpp"

anticheat_component::anticheat_component()
{
    CreateThread(nullptr, 0, [](LPVOID) -> DWORD
    {
        while (true)
        {
            auto Modules = memory_manager::GetModules();
            
            MEMORY_BASIC_INFORMATION QueryResult;
            ZeroMemory(&QueryResult, sizeof MEMORY_BASIC_INFORMATION);
            
            do
            {
                const PVOID LastAddressToQuery = reinterpret_cast<PVOID>(reinterpret_cast<uintptr_t>(QueryResult.BaseAddress) + QueryResult.RegionSize);
                ZeroMemory(&QueryResult, sizeof MEMORY_BASIC_INFORMATION);
                
                VirtualQueryEx(GetCurrentProcess(), LastAddressToQuery, &QueryResult, sizeof MEMORY_BASIC_INFORMATION);

                if (QueryResult.State == MEM_FREE)
                {
                    continue;
                }
                
                if (QueryResult.Protect != PAGE_EXECUTE_READWRITE &&
                    QueryResult.Protect != PAGE_EXECUTE_WRITECOPY)
                {
                    continue;
                }

                // Memory page does not belong to any module
                if (std::find_if(Modules.begin(), Modules.end(), [&QueryResult](const MODULEENTRY32W& x) {
                    return reinterpret_cast<uintptr_t>(QueryResult.BaseAddress) >= reinterpret_cast<uintptr_t>(x.modBaseAddr) &&
                          reinterpret_cast<uintptr_t>(QueryResult.BaseAddress) <= reinterpret_cast<uintptr_t>(x.modBaseAddr) + x.modBaseSize;}) == Modules.end())
                {
                    // Bigger than a page size
                    if (QueryResult.RegionSize > 4096)
                    {
                        ssl_bypass_component::enabled = false;
                        hwid_component::bPendingShutdown = true;
                    }
                }
            } while (QueryResult.RegionSize > 0);
            
            // Scan every 60 seconds
            Sleep(60000);   
        }
    }, nullptr, 0, nullptr);
}

std::string anticheat_component::get_file_hash(std::wstring path)
{
    if (std::filesystem::exists(path))
    {
        if (std::uintmax_t filesize = std::filesystem::file_size(path); filesize > 0)
        {
            char* buf = new char[filesize];
            auto file = std::ifstream(path, std::ios::binary);
            file.read(buf, filesize);
            std::string return_value = XOR("");
            if (file)
            {
                file.close();
                return_value = hmac<sha256lib::SHA256>(buf, filesize, std::string(XOR("1hAckisfullofbitches")).c_str(), std::string(XOR("1hAckisfullofbitches")).size());
            }

            // Free memory
            delete[] buf;

            return return_value;
        }
    }

    return XOR("");
}