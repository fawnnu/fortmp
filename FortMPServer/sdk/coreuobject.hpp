#pragma once

class UObject
{
public:
    void** VTableObject;
    int ObjectFlags;
    int InternalIndex;
    UObject* Class;
    FName NamePrivate;
    UObject* Outer;

    std::wstring GetName()
    {
        return NamePrivate.ToString();
    }

    std::wstring GetFullName()
    {
        std::wstring result;
        
        for (UObject* outer = Outer; outer; outer = outer->Outer)
        {
            result = outer->GetName() + L"." + result;
        }

        result = result + this->GetName();

        return result;
    }

    bool IsA(UObject* ComparisonClass) const
    {
        return Class == ComparisonClass;
    }
    
private:
    
};