#pragma once

struct FCustomPropertyData
{
    FCustomPropertyData(std::wstring InClassName, std::wstring InFullClassPath, std::wstring InPropertyName, void* InConstructData, SIZE_T InPropertySize)
    {
        ClassName = InClassName;
        FullClassPath = InFullClassPath;
        PropertyName = InPropertyName;
        ConstructData = InConstructData;
        PropertySize = InPropertySize;
    }
    
    std::wstring ClassName;
    std::wstring FullClassPath;
    std::wstring PropertyName;
    void* ConstructData;
    SIZE_T PropertySize;
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
    DEFINE_FUNCTION_HOOK(void, GetPrivateStaticClassBody, (const TCHAR* PackageName, const TCHAR* Name, UObject*& ReturnClass, void(*RegisterNativeFunc)(), uint32 InSize, uintptr_t InClassFlags,uintptr_t InClassCastFlags, const TCHAR* InConfigName, PVOID InClassConstructor, PVOID InClassVTableHelperCtorCaller,PVOID InClassAddReferencedObjects,PVOID InSuperClassFn,PVOID InWithinClassFn,bool bIsDynamic /*= false*/))
}
