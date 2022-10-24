#pragma once

#define REGISTER_CUSTOM_UFUNCTION(Function, Class)\
    {\
        function_component::create_custom_ufunction(#Function, Class, Function);\
    }

class function_component
{
public:
    function_component();

    static void create_custom_ufunction(const char* InName, UObject* InClass, void* NativeFunction);

private:
    inline static DWORD FuncMap_Offset;

    inline static UFunction* LastFunction;

    static UFunction* GetLastFunction()
    {
        return LastFunction;
    }

    inline static void (*FuncMap_Add)(void* FuncMap, int* OutParam, __int64** Pair, void*);

    inline static void (*CreateLinkAndAddChildFunctionsToMap)(UObject* Class, void* Functions, uint32_t NumFunctions);
};
