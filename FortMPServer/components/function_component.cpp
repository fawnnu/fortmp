#include "include.hpp"
#include "function_component.hpp"

function_component::function_component()
{
    if (GetEngineVersionEasy() < 423)
    {
        FuncMap_Offset = 0x108;
    }
    else if (GetEngineVersionEasy() < 425)
    {
        FuncMap_Offset = 0x100;
    }
    else if (GetEngineVersionEasy() < 426)
    {
        FuncMap_Offset = 0x148;
    }
    else
    {
        FuncMap_Offset = 0x130;
    }

    // Initialize functions
    std::vector<uintptr_t> calls = memory_manager::get_all_calls_in_function(reinterpret_cast<uintptr_t>(FindFunctionByName));

    INIT_POINTER(FuncMap_Add, calls[calls.size() - 1] + 5 + *reinterpret_cast<int32_t*>(calls[calls.size() - 1] + 1));

    INIT_POINTER(CreateLinkAndAddChildFunctionsToMap, memory_manager::find_pattern("E8 ? ? ? ? 8B 4E 40 48 8B 46 28 48 89 44 24 ? 89 4C 24 20", true, 1));
}

void function_component::create_custom_ufunction(const char* InName, UObject* InClass, void* NativeFunction)
{
    UFunction* NewFunction = static_cast<UFunction*>(UGameplayStatics::SpawnObject(FindObject(L"/Script/CoreUObject.Function"), InClass));

    // Create FName
    FName FunctionName(InName);
    
    // Update its name
    NewFunction->NamePrivate = FunctionName;

    // Set flags to FUNC_Native
    NewFunction->set_function_flags(0x00000400);

    // Bind
    NewFunction->set_native_function(NativeFunction);

    if (FuncMap_Add == nullptr && CreateLinkAndAddChildFunctionsToMap == nullptr)
    {
        spdlog::error(L"Failed to add newly created function to class: invalid Functions.");
        
        return;
    }

    // Add to class
    if (CreateLinkAndAddChildFunctionsToMap)
    {
        struct
        {
            UFunction* (*CreateFuncPtr)();
            const char* FuncNameUTF8;
        } ClassFunctionLinkInfo;

        LastFunction = NewFunction;

        ClassFunctionLinkInfo.CreateFuncPtr = GetLastFunction;
        ClassFunctionLinkInfo.FuncNameUTF8 = InName;

        CreateLinkAndAddChildFunctionsToMap(InClass, &ClassFunctionLinkInfo, 1);
    }
    else
    {
        UE::TMap<FName, UFunction*>& FuncMap = *reinterpret_cast<UE::TMap<FName, UFunction*>*>(reinterpret_cast<uintptr_t>(InClass) + FuncMap_Offset);

        int OutParam = 0;

        __int64 Pair[2];
        Pair[0] = reinterpret_cast<__int64>(&FunctionName);
        Pair[1] = reinterpret_cast<__int64>(&NewFunction);

        // Add
        FuncMap_Add(&FuncMap, &OutParam, reinterpret_cast<__int64**>(Pair), nullptr);
    }
    
    spdlog::info(L"Successfully created and registered new function: '{}'", UKismetSystemLibrary::GetPathName(NewFunction).c_str());
}


