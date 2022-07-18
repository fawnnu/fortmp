#include "include.hpp"
#include "ssl_bypass_component.hpp"

#include <regex>

ssl_bypass_component::ssl_bypass_component()
{
    INIT_POINTER(ssl_bypass_component_hook::CurlSet, memory_manager::find_pattern(XOR("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 30 33 ED 49 8B F0 48 8B D9")));
    INIT_POINTER(ssl_bypass_component_hook::CurlEasy, memory_manager::find_pattern(XOR("89 54 24 10 4C 89 44 24 18 4C 89 4C 24 20 48 83 EC 28 48 85 C9 75 08 8D 41 2B 48 83 C4 28 C3 4C")));
    
    VectoredExceptionHook::StartHook( reinterpret_cast<uintptr_t>(ssl_bypass_component_hook::CurlEasy), reinterpret_cast<uintptr_t>(ssl_bypass_component_hook::CurlEasy_Hook));
}

int ssl_bypass_component_hook::CurlSet_Hook(LPVOID A, int B, ...)
{
    va_list argument;
    va_start(argument, B);

    int result = CurlSet(A, B, argument);

    va_end(argument);

    return result;
}

int ssl_bypass_component_hook::CurlEasy_Hook(LPVOID A, int B, ...)
{
    if (!A) return 43;

    int return_value = 0;
    const std::regex EpicURL("https:\\/\\/(.*)\\.ol\\.epicgames.com");

    va_list argument;
    va_list CopiedArgument;
    va_start(argument, B);
    va_copy(CopiedArgument, argument);

    if (B == 64) return_value = CurlSet_Hook(A, B, 0);
    else if (B == 10002)
    {
        std::string Url(va_arg(CopiedArgument, PCHAR));

        if (regex_search(Url, EpicURL))
        {
            Url = regex_replace(Url, EpicURL, ssl_bypass_component::redirect_url);

            if (Url.find("/account/api/oauth/token") != -1)
            {
                if (GetEngineVersionEasy() < 423) {
                    VectoredExceptionHook::DisableHook();

                    DetourTransactionBegin();
                    DetourUpdateThread(GetCurrentThread());

                    REGISTER_FUNCTION_DETOUR(CurlEasy, 0, CurlEasy_Hook);
                    
                    DetourTransactionCommit();
                }
            }

            return_value = CurlSet_Hook(A, B, Url.c_str());
        }
    }

    
    else if (B == 10004) return_value = CurlSet_Hook(A, B, "");
    else return_value = CurlSet(A, B, argument);

    va_end(argument);
    va_end(CopiedArgument);

    return return_value;
}

