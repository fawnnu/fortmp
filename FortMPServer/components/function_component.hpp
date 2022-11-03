#pragma once

/*
 * FMP Custom function docs:
 *
 * FMP Custom function is a way for Server Mods (Custom Gamemode Blueprints) to call c++ code
 *
 * To create custom functions, create a normal C++ function first (Must have 2 params: Self Object and FFrame, FFrame's parameter name MUST be StackFrame).
 * Then, call the REGISTER_CUSTOM_UFUNCTION() macro, passing in the C++ function pointer and a class object.
 * e.g. REGISTER_CUSTOM_UFUNCTION(UpdateWeaponLoadedAmmo, AFortWeapon::StaticClass());
 *
 * Then, go into the C++ function implementation, and call the INIT_CUSTOM_FUNCTION() macro before executing any of your code.
 *
 * If your custom function uses parameters, do the same but register the function with REGISTER_CUSTOM_UFUNCTION_WITH_PARAMS, With the last param being the size of parameters
 * e.g. A function with an FString parameter only will have 0x10 of ParmsSize, due to the size of FString being 0x10
 *
 * To read parameters in a custom function with parameters, go to the C++ function implementation and:
 * - If your function will be called by a blueprint, use GET_CUSTOM_FUNCTION_BP_PARAM. NOTE: This macro HAS to be called for EVERY function parameter, even ones you won't use
 * - If your function is called by ProcessEvent (FortMP, Delegates), read the parameters from StackFrame.Locals
 *
 * Overall, custom functions implementation looks as follows:
 *
 * void CustomFunction(UObject* ThisObject, FFrame& StackFrame)
 * {
 *  // Read params...
 *
 *  INIT_CUSTOM_FUNCTION();
 *
 *  // Code
 * }
 */

#define REGISTER_CUSTOM_UFUNCTION(Function, Class)\
    {\
        function_component::create_custom_ufunction(#Function, Class, Function);\
    }

#define REGISTER_CUSTOM_UFUNCTION_WITH_PARAMS(Function, Class, ParmsSize)\
    {\
        function_component::create_custom_ufunction(#Function, Class, Function, ParmsSize);\
    }

#define GET_CUSTOM_FUNCTION_BP_PARAM(ParameterType, ParameterName)\
    ParameterType ParameterName;\
    StackFrame.Step(StackFrame.Object, &ParameterName);\

#define INIT_CUSTOM_FUNCTION()\
    if (StackFrame.Code)\
    {\
        StackFrame.Code++;\
    }

class function_component
{
public:
    function_component();

    static void create_custom_ufunction(const char* InName, UObject* InClass, void* NativeFunction, int ParmsSize = 0);

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
