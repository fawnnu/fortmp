#include "include.hpp"
#include "anticheat_component.hpp"

anticheat_component::anticheat_component()
{
    // List of blacklisted hashes
    BannedHashes = {
    };
    
    CreateThread(nullptr, 0, [](LPVOID) -> DWORD
    {
        while (true)
        {
            auto Modules = memory_manager::GetModules();
            std::sort(Modules.begin(), Modules.end(), [](MODULEENTRY32W& First, MODULEENTRY32W& Second)
            {
                return reinterpret_cast<uintptr_t>(First.modBaseAddr) < reinterpret_cast<uintptr_t>(Second.modBaseAddr);
            });
            auto FortniteModule = std::find_if(Modules.begin(), Modules.end(), [](MODULEENTRY32W& x) -> bool
            {
               return wcsstr(x.szModule, XOR(TEXT("FortniteClient-Win64-Shipping"))); 
            });
            
            if (FortniteModule != Modules.end())
            {
                auto NextModule = FortniteModule + 1;
                
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
                            TerminateProcess(GetCurrentProcess(), 0);

                            // If some fucker patches TerminateProcess, just crash the process
                            *reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(Pointer) + 0x1) = 1;
                        }
                    }
                } while (QueryResult.RegionSize > 0);
            }
            
            // Scan every 30 seconds
            Sleep(30000);   
        }
    }, nullptr, 0, nullptr);

    CreateThread(nullptr, 0, [](LPVOID) -> DWORD
    {
        while (true)
        {
            // Scan all the processes
            auto Processes = memory_manager::GetProcessesExecutables();
            for (auto Process : Processes)
            {
                if (std::string FileHash = get_file_hash(Process); FileHash.size())
                {
                    // Found banned hash
                    if (std::find_if(BannedHashes.begin(), BannedHashes.end(), [&FileHash](const std::string& x){ return x == FileHash; }) != BannedHashes.end())
                    {
                        // Exit
                        TerminateProcess(GetCurrentProcess(), 0);

                        // If some fucker patches TerminateProcess, just crash the process
                        *reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(Pointer) + 0x1) = 1;
                    }
                }
            }
            
            // Scan every 120 seconds
            Sleep(120000);
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
            if (file)
            {
                file.close();
                return hmac<sha256lib::SHA256>(buf, filesize, std::string(XOR("1hAckisfullofbitches")).c_str(), std::string(XOR("1hAckisfullofbitches")).size());
            }
        }
    }

    return XOR("");
}