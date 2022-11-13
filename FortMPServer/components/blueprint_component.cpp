#include "include.hpp"
#include "blueprint_component.hpp"

#include "function_component.hpp"

blueprint_component::blueprint_component()
{
    REGISTER_CUSTOM_UFUNCTION(LogFMP, UFortKismetLibrary::StaticClass());
    INIT_CUSTOM_FUNC_PARAMS(LogFMP, UFortKismetLibrary::StaticClass(), 0x10);

    REGISTER_CUSTOM_UFUNCTION(BP_FindObject, UFortKismetLibrary::StaticClass());
    INIT_CUSTOM_FUNC_PARAMS(BP_FindObject, UFortKismetLibrary::StaticClass(), 0x18);
    INIT_CUSTOM_FUNC_RET_OFFSET(BP_FindObject, UFortKismetLibrary::StaticClass(), 0x10);
}

void blueprint_component::LogFMP(UObject* Self, FFrame& StackFrame)
{
    GET_CUSTOM_FUNCTION_BP_PARAM(FString, InString);

    INIT_CUSTOM_FUNCTION();
    
    // Redirect this function's input to spdlog
    spdlog::info(L"BlueprintLog: {}", InString.c_str());
}

void blueprint_component::BP_FindObject(UObject* Self, FFrame& StackFrame, void* ReturnValue)
{
    GET_CUSTOM_FUNCTION_BP_PARAM(FString, InObjectName);

    INIT_CUSTOM_FUNCTION();

    UObject* FoundObject = FindObject(InObjectName.c_str());

    *reinterpret_cast<UObject**>(ReturnValue) = FoundObject;
}

