#include "include.hpp"
#include "blueprint_component.hpp"

#include "function_component.hpp"

blueprint_component::blueprint_component()
{
    REGISTER_CUSTOM_UFUNCTION_WITH_PARAMS(LogFMP, UFortKismetLibrary::StaticClass(), 0x10);
}

void blueprint_component::LogFMP(UObject* Self, FFrame& StackFrame)
{
    // Read params from bytecode
    FString InString;
    StackFrame.Step(StackFrame.Object, &InString);

    // Bump Code one last time
    StackFrame.Code++;
    
    // Redirect this function's input to spdlog
    spdlog::info(L"BlueprintLog: {}", InString.c_str());
}

