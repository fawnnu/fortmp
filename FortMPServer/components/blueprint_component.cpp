#include "include.hpp"
#include "blueprint_component.hpp"

#include "function_component.hpp"

blueprint_component::blueprint_component()
{
    REGISTER_CUSTOM_UFUNCTION_WITH_PARAMS(LogFMP, UFortKismetLibrary::StaticClass(), 0x10);
}

void blueprint_component::LogFMP(UObject* Self, FFrame& StackFrame)
{
    GET_CUSTOM_FUNCTION_BP_PARAM(FString, InString);

    INIT_CUSTOM_FUNCTION();
    
    // Redirect this function's input to spdlog
    spdlog::info(L"BlueprintLog: {}", InString.c_str());
}

