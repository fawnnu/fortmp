#pragma once

struct FName;
struct FString;

// FName constructor for FString
inline void (*FName_FName)(FName*, const wchar_t*, int);

// FName to FString
inline void (*FName_ToString)(FName*, FString&);

// FMemory::Free
inline void (*FMemory_Free)(void* DataToFree);

// FParse::Command
inline bool (*Parse__CommandInternal)(const TCHAR** Stream, const TCHAR* Match);
inline bool (*Parse__TokenInternal)(const TCHAR*& Stream, FString& Arg, bool bUseEscape);

struct FString : private TArray<wchar_t>
{
    FString() = default;
    
    FString(const wchar_t* InString)
    {
        if (InString)
        {
            Count = std::wcslen(InString) + 1;
            Max = std::wcslen(InString) + 1;

            Data = const_cast<wchar_t*>(InString);
        }
    }

    FString(std::wstring InString)
    {
        if (InString.length())
        {
            Count = std::wcslen(InString.c_str()) + 1;
            Max = std::wcslen(InString.c_str()) + 1;
            Data = (wchar_t*)FMemory_Realloc(Data, Count * sizeof(wchar_t), 0);
            wcscpy_s(Data, Count, InString.c_str());
        }
    }

    bool operator==(FString in_str);
    
    const wchar_t* c_str() const
    {
        return Data;
    }

    std::string AnsiString() const
    {
        const std::wstring wString(Data);
        TO_STRING(wString, sString)

        return sString;
    }

    bool IsValid()
    {
        return Data;
    }
};

struct FName
{
    FName() = default;
    
    FName(int InComparisonIndex, int InNumber) :
        ComparisonIndex(InComparisonIndex), Number(InNumber)
    {
    }

    FName(const wchar_t* InName)
    {
        // Create FName from FString
        FName_FName(this, InName, 1);
    }

    FName(const char* InName)
    {
        wchar_t WideName[1024];

        // Convert char to wide char
        MultiByteToWideChar(CP_ACP, 0, InName, -1, WideName, sizeof(WideName));

        // Create FName from FString
        FName_FName(this, WideName, 1);
    }

    std::wstring ToString()
    {
        FString Name;

        // Convert FName to FString and store it in Name
        FName_ToString(this, Name);

        std::wstring ReturnName(Name.c_str());

        // Free the string as to not cause any memory leaks
        FMemory_Free((void*) Name.c_str());
        
        return ReturnName;
    }

    std::string sToString()
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        
        return converter.to_bytes(ToString());
    }
    
    int ComparisonIndex;
    int Number;

    FORCEINLINE bool operator==(FName in_name) const
    {
        return this->ComparisonIndex == in_name.ComparisonIndex && this->Number == in_name.Number;
    }

    FORCEINLINE bool operator!=(FName in_name) const
    {
        return this->ComparisonIndex != in_name.ComparisonIndex && this->Number != in_name.Number;
    }

    FORCEINLINE bool operator<(FName in_name) const
    {
        return std::tie(this->ComparisonIndex, this->Number) < std::tie(in_name.ComparisonIndex, in_name.Number);
    }
};

struct FURL
{
    // Protocol to use with the URL
    FString Protocol;

    // Hostname to use with the URL
    FString Host;

    // Host port to use with the URL
    int Port;

    // Is the URL valid?
    int Valid;

    // Map name
    FString Map;

    // Optional place to download map from
    FString RedirectUrl;

    // Options to use with the URL
    TArray<FString> Op;

    // Portal to enter through
    FString Portal;

    FURL(FString InMap, int InPort) :
        Map(InMap), Port(InPort), Protocol(L"unreal"), Host(nullptr), RedirectUrl(nullptr), Portal(nullptr)
    {
    }
};

struct FSoftObjectPath
{
    // Path to the asset
    FName AssetPathName;

    // Sub path of the asset
    FString SubPathString;
};

struct FStringAssetReference
{
    // Path to the asset
    FString AssetReference;
};

struct FParse
{
    static bool Command(const TCHAR** Stream, const TCHAR* Match)
    {
        if (Parse__CommandInternal)
        {
            return Parse__CommandInternal(Stream, Match);
        }
        
        return false;
    }

    static bool Token(const TCHAR*& Stream, FString& Arg, bool bUseEscape)
    {
        if (Parse__TokenInternal)
        {
            return Parse__TokenInternal(Stream, Arg, bUseEscape);   
        }
        
        return false;
    }
};