#pragma once

//TArray Implementation
template <class T>
struct TArray
{
    friend struct FString;
    
public:
    TArray() = default;
    
    int Num() const
    {
        return Count;
    }
    
private:
    T* Data;
    int Count;
    int Max;
};

//FString implementation
struct FString : private TArray<wchar_t>
{
public:
    FString() = default;

    FString(const wchar_t* InString)
    {
        Max = Count = *InString ? std::wcslen(InString) + 1 : 0;

        if (Count)
        {
            Data = const_cast<wchar_t*>(InString);
        }
    }

    const wchar_t* c_str() const
    {
        return Data;
    }
    
private:
};

//FName implementation
struct FName
{
public:
    
    FName() = default;

    FORCEINLINE bool operator==(FName name) const
    {
        return this->ComparisonIndex == name.ComparisonIndex && this->Number == name.Number;
    }

    std::wstring ToString() 
    {
        FString temp;
        global_engine_manager->fname_to_string(this, temp);

        std::wstring ret(temp.c_str());

        global_engine_manager->free_internal((void*)temp.c_str());

        return ret;
    }
    
private:
    int ComparisonIndex;
    int Number;
};