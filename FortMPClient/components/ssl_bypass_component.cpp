#include "include.hpp"
#include "ssl_bypass_component.hpp"

#include <regex>

#include "hwid_component.hpp"
#include "patches_component.hpp"
#include "pe_component.hpp"
#include "battlepass_component.hpp"
#include "voice_component.hpp"

void oauth_token_callback(request* req)
{
    try
    {
        auto Json = nlohmann::json::parse(req->Response);

        if (Json.contains(XOR("account_id")))
        {
            if (Json[XOR("account_id")] != XOR("a8ade897-b930-47e2-909d-b114f94d04bd"))
            {
                if (Json.contains(XOR("access_token")))
                {
                    hwid_component::access_token = Json[XOR("access_token")];
                    hwid_component::account_id = Json[XOR("account_id")];

                    if (hwid_component::bServiceRunning == false)
                    {
                        hwid_component::bServiceRunning = true;
                        hwid_component::StartMessageService();

                        new pe_component();
                        new battlepass_component();
                        //new voice_component(); // Voice chat!
                        
                        patches_component::initialize_ProjectWorldToScreen_patch();
                    }
                }
            }
        }
    }
    catch (nlohmann::json::parse_error& e)
    {
        
    }
}

size_t oauth_token_writefunction(void* content, size_t size, size_t nmemb, void* userp)
{
    if (auto Result = ssl_bypass_component::find_request(XOR("/account/api/oauth/token")))
    {
        return ssl_bypass_component::process_write(Result, content, size, nmemb, userp);
    }
    
    return 0;
}

uintptr_t (*AntiTamper)();
uintptr_t AntiTamper_Hook()
{
    // Disable hook
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourDetach(&reinterpret_cast<PVOID&>(AntiTamper), AntiTamper_Hook);
                    
    DetourTransactionCommit();

    // Execute function
    uintptr_t RetVal = AntiTamper();

    // Switch from VEH
    VectoredExceptionHook::DisableHook();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(ssl_bypass_component_hook::CurlEasy, 0, ssl_bypass_component_hook::CurlEasy_Hook);
                    
    DetourTransactionCommit();

    return RetVal;
}

ssl_bypass_component::ssl_bypass_component()
{
    enabled = true;
    
    INIT_POINTER(ssl_bypass_component_hook::CurlSet, memory_manager::find_pattern(XOR("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 30 33 ED 49 8B F0 48 8B D9")));
    INIT_POINTER(ssl_bypass_component_hook::CurlEasy, memory_manager::find_pattern(XOR("89 54 24 10 4C 89 44 24 18 4C 89 4C 24 20 48 83 EC 28 48 85 C9 75 08 8D 41 2B 48 83 C4 28 C3 4C")));

    VectoredExceptionHook::StartHook( reinterpret_cast<uintptr_t>(ssl_bypass_component_hook::CurlEasy), reinterpret_cast<uintptr_t>(ssl_bypass_component_hook::CurlEasy_Hook));

    // Run only on 4.5 as it's certified to work
    if (GetCL() == 4159770)
    {
        // Hook anti tamper func
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        // Disable anti tampering
        REGISTER_FUNCTION_DETOUR(AntiTamper, memory_manager::find_pattern(XOR("E8 ? ? ? ? 48 8D 4C 24 ? FF 15 ? ? ? ? 0F 57 C0 F2 48 0F 2A 44 24 ? F2 0F 59 05 ? ? ? ? F2 0F 58 05 ? ? ? ? F2 0F 11 05 ? ? ? ? 48 83 C4 28 C3"), true, 1), AntiTamper_Hook);
    
        DetourTransactionCommit();
    }

    // Capture requests
    requests_to_capture.push_back(new request(XOR("/account/api/oauth/token"), oauth_token_writefunction, oauth_token_callback));
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
    if (!ssl_bypass_component::enabled) return 43;

    int return_value = 0;
    const std::regex EpicURL("https:\\/\\/(.*)\\.ol\\.epicgames.com");

    va_list argument;
    va_list CopiedArgument;
    va_start(argument, B);
    va_copy(CopiedArgument, argument);

    if (B == 64)
    {
        return_value = CurlSet_Hook(A, B, 0);
    }
    else if (B == 10002)
    {
        std::string Url(va_arg(CopiedArgument, PCHAR));

        if (regex_search(Url, EpicURL))
        {
            Url = regex_replace(Url, EpicURL, ssl_bypass_component::redirect_url);

            // On other CLs run old code
            if (GetCL() != 4159770)
            {
                if (Url.find(XOR("/account/api/oauth/token")) != -1)
                {
                    if (GetEngineVersionEasy() < 423) {
                        VectoredExceptionHook::DisableHook();

                        DetourTransactionBegin();
                        DetourUpdateThread(GetCurrentThread());

                        REGISTER_FUNCTION_DETOUR(CurlEasy, 0, CurlEasy_Hook);
                    
                        DetourTransactionCommit();
                    }
                }
            }
            
            for (request* request_to_capture : ssl_bypass_component::requests_to_capture)
            {
                if (Url.find(request_to_capture->URL) != std::string::npos)
                {
                    request_to_capture->CurlHANDLE = A;
                    
                    break;
                }
            }

            return_value = CurlSet_Hook(A, B, Url.c_str());
        }
    }
    else if (B == 10004)
    {
        return_value = CurlSet_Hook(A, B, "");
    }
    else
    {
        return_value = CurlSet(A, B, argument);
    }

    for (request* request_to_capture : ssl_bypass_component::requests_to_capture)
    {
        if (request_to_capture->CurlHANDLE == A)
        {
            // CURLOPT_WRITEFUNCTION 
            if (B == 20011)
            {
                LPVOID OriginalWriteFunction = va_arg(CopiedArgument, LPVOID);
                request_to_capture->writefunction_original = OriginalWriteFunction;

                return_value = CurlSet_Hook(A, B, request_to_capture->writefunction_override);
            }
            
            break;
        }
    }
    
    va_end(argument);
    va_end(CopiedArgument);

    return return_value;
}

