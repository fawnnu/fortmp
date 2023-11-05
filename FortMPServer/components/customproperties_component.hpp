#pragma once

struct FCustomPropertyData
{
    FCustomPropertyData(std::wstring InClassName, std::wstring InPropertyName, void* InConstructData)
    {
        ClassName = InClassName;
        PropertyName = InPropertyName;
        ConstructData = InConstructData;
    }
    
    std::wstring ClassName;
    std::wstring PropertyName;
    void* ConstructData;
};

class customproperties_component
{
public:
    customproperties_component();

    static inline std::vector<FCustomPropertyData> PendingCustomProperties;
};

namespace customproperties_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, ConstructUProperty, (UObject* NewClass, void*** PropertyArray, int& PropertyNum))
}
