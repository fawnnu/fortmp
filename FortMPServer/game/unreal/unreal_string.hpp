#pragma once

struct FName;
struct FString;

// FName constructor for FString
inline void (*FName_FName)(FName*, const wchar_t*, int);

// FName to FString
inline void (*FName_ToString)(FName*, FString&);

// FMemory::Free
inline void (*FMemory_Free)(void* DataToFree);

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

    bool operator==(FString in_str);
    
    const wchar_t* c_str() const
    {
        return Data;
    }
};

struct FName
{
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
    
    int ComparisonIndex;
    int Number;

    FORCEINLINE bool operator==(FName in_name) const
    {
        return this->ComparisonIndex == in_name.ComparisonIndex && this->Number == in_name.Number;
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