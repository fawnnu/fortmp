#pragma once

class anticheat_component
{
public:
    anticheat_component();

    static std::string get_file_hash(std::wstring path);
    
    inline static void* Pointer;

    inline static std::vector<std::string> BannedHashes;
};

namespace anticheat_component_hooks
{
}
