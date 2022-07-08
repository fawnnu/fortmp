#pragma once

class ssl_bypass_component
{
public:
    inline static std::string redirect_url;
    
    ssl_bypass_component();
};

namespace ssl_bypass_component_hook
{
    DEFINE_FUNCTION_HOOK(int, CurlSet, (LPVOID A, int B, ...));
    DEFINE_FUNCTION_HOOK(int, CurlEasy, (LPVOID A, int B, ...));
}