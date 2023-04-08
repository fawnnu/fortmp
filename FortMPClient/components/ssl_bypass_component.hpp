#pragma once

struct request
{
    request() = default;

    request(std::string inURL, void* inwritefunction_override, void* infinal_callback)
    {
        URL = inURL;
        CurlHANDLE = nullptr;
        POSTFIELDS = "";
        Response = "";
        writefunction_original = nullptr;
        writefunction_override = inwritefunction_override;
        final_callback = infinal_callback;
    }
    
    request(std::string inURL)
    {
        URL = inURL;
        CurlHANDLE = nullptr;
        POSTFIELDS = "";
        Response = "";
        writefunction_original = nullptr;
        writefunction_override = nullptr;
        final_callback = nullptr;
    }

    bool operator==(request* cmp)
    {
        return URL == cmp->URL;
    }
    
    LPVOID CurlHANDLE;
    std::string URL;
    std::string POSTFIELDS;
    std::string Response;

    void* writefunction_override;
    void* writefunction_original;
    void* final_callback;
};

class ssl_bypass_component
{
public:
    inline static std::string redirect_url;
    inline static std::vector<request*> requests_to_capture;
    
    ssl_bypass_component();
};

namespace ssl_bypass_component_hook
{
    DEFINE_FUNCTION_HOOK(int, CurlSet, (LPVOID A, int B, ...));
    DEFINE_FUNCTION_HOOK(int, CurlEasy, (LPVOID A, int B, ...));
}