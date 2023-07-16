#pragma once

struct request
{
    request() = default;

    request(std::string inURL, void* inwritefunction_override, void* infinal_callback)
    {
        URL = inURL;
        CurlHANDLE = nullptr;
        Response = "";
        writefunction_original = nullptr;
        writefunction_override = inwritefunction_override;
        final_callback = infinal_callback;
    }
    
    request(std::string inURL)
    {
        URL = inURL;
        CurlHANDLE = nullptr;
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
    inline static bool enabled;

    static request* find_request(std::string inURL)
    {
        for (request* request_to_capture : requests_to_capture)
        {
            if (request_to_capture->URL == inURL)
            {
                return request_to_capture;
            }
        }

        return nullptr;
    }

    static size_t process_write(request* req, void* content, size_t size, size_t nmemb, void* userp)
    {
        req->Response.clear();
        req->Response.append(static_cast<char*>(content), size * nmemb);
        reinterpret_cast<void (*)(request*)>(req->final_callback)(req);
        
        return reinterpret_cast<size_t (*)(void*, size_t, size_t, void*)>(req->writefunction_original)(content, size, nmemb, userp);
    }
    
    ssl_bypass_component();
};

namespace ssl_bypass_component_hook
{
    DEFINE_FUNCTION_HOOK(int, CurlSet, (LPVOID A, int B, ...));
    DEFINE_FUNCTION_HOOK(int, CurlEasy, (LPVOID A, int B, ...));
}