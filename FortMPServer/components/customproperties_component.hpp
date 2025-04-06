#pragma once

#define CREATE_CUSTOM_PROPERTY_CONTAINER(Class, FullClassPath, ContainerName, Template)\
auto Construct##ContainerName = []() -> UObject*\
        {\
            static UObject* ContainerName = nullptr;\
            if (!ContainerName)\
            {\
                Template::FStructParams StructParams;\
                ZeroMemory(&StructParams, sizeof Template::FStructParams);\
                StructParams.OuterFunc = []\
                {\
                    return FindObject(TEXT("/Script/FortniteGame"));  \
                };\
                StructParams.SuperFunc = []() -> UObject*\
                {\
                    return nullptr;\
                };\
                StructParams.StructOpsFunc = nullptr;\
                StructParams.NameUTF8 = #ContainerName;\
                StructParams.ObjectFlags = RF_Public | RF_Transient | RF_MarkAsRootSet;\
                StructParams.StructFlags = 0;\
                StructParams.SizeOf = sizeof F##ContainerName;\
                StructParams.AlignOf = alignof(F##ContainerName);\
                StructParams.PropertyArray = nullptr;\
                StructParams.NumProperties = 0;\
                \
                ConstructUScriptStruct(&ContainerName, &StructParams);\
            }\
            \
            return ContainerName;\
        };\
        \
Template::FStructPropertyParams* ContainerName##Property = new Template::FStructPropertyParams;\
ZeroMemory(ContainerName##Property, sizeof Template::FStructPropertyParams);\
ContainerName##Property->Type = EPropertyClass::Struct;\
ContainerName##Property->NameUTF8 = #ContainerName;\
ContainerName##Property->ObjectFlags = RF_Public | RF_Transient | RF_MarkAsRootSet;\
ContainerName##Property->PropertyFlags = CPF_Edit | CPF_IsPlainOldData;\
ContainerName##Property->ArrayDim = 1;\
ContainerName##Property->ScriptStructFunc = Construct##ContainerName;\
PendingCustomProperties.push_back(FCustomPropertyData(TEXT(Class), TEXT(FullClassPath), TEXT(#ContainerName), ContainerName##Property, sizeof F##ContainerName));


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
    static void pre_init();
    static void wait_for_initialization();
    
    static inline std::vector<FCustomPropertyData> PendingCustomProperties;
};

namespace customproperties_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, ConstructUProperty, (UObject* NewClass, void*** PropertyArray, int& PropertyNum))
    DEFINE_FUNCTION_HOOK(void, GetPrivateStaticClassBody, (const TCHAR* PackageName, const TCHAR* Name, UObject*& ReturnClass, void(*RegisterNativeFunc)(), uint32 InSize, uintptr_t InClassFlags,uintptr_t InClassCastFlags, const TCHAR* InConfigName, PVOID InClassConstructor, PVOID InClassVTableHelperCtorCaller,PVOID InClassAddReferencedObjects,PVOID InSuperClassFn,PVOID InWithinClassFn,bool bIsDynamic /*= false*/))
}