#pragma once

class blueprint_component
{
public:
    blueprint_component();

    // Wrapper UFunction to call spdlog on blueprints
    static void LogFMP(UObject* Self, FFrame& StackFrame);

    // Wrapper UFunction to find objects in blueprints
    static void BP_FindObject(UObject* Self, FFrame& StackFrame, void* ReturnValue);
};

/*
 * A Disassembler for blueprint functions, which outputs Json files
 */
class BlueprintDisasm
{
public:
    BlueprintDisasm() = default;
    BlueprintDisasm(UFunction* InTargetScript);

    // Functions
    nlohmann::ordered_json DisasmExpression(int& ScriptIndex);
    int32 ReadInt(int32& ScriptIndex);
    uint64 ReadQword(int32& ScriptIndex);
    uint8 ReadByte(int32& ScriptIndex);
    FString ReadName(int32& ScriptIndex);
    uint16 ReadWord(int32& ScriptIndex);
    float ReadFloat(int32& ScriptIndex);
    int ReadSkipCount(int32& ScriptIndex);
    FString ReadString8(int32& ScriptIndex);
    FString ReadString16(int32& ScriptIndex);
    FString ReadString(int32& ScriptIndex);
    
    UFunction* TargetScript;
    std::vector<uint8> Script;

    static inline std::map<std::string, nlohmann::ordered_json> DisassembledBlueprintFunctions;
};
