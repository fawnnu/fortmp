#include "include.hpp"
#include "blueprint_component.hpp"

#include "function_component.hpp"

using namespace nlohmann;

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

    UObject* FoundObject = FindObject(InObjectName.c_str(), false, true);

    *reinterpret_cast<UObject**>(ReturnValue) = FoundObject;
}

BlueprintDisasm::BlueprintDisasm(UFunction* InTargetScript)
{
    // Set function
    TargetScript = InTargetScript;

    // Script info
    std::wstring ScriptName = InTargetScript->GetName().c_str();
    TO_STRING(ScriptName, sScriptName)

    // Class
    std::wstring ClassName = InTargetScript->Outer->GetName().c_str();
    TO_STRING(ClassName, sClassName);
    
    // TArray -> std::vector
    TArray<uint8>& BytecodeScript = *reinterpret_cast<TArray<uint8>*>(reinterpret_cast<uintptr_t>(TargetScript) + CoreOffsets::Script);
    Script = std::vector(BytecodeScript.begin(), BytecodeScript.end());
    ordered_json Output{};
    int Index = 0;

    // Log
    spdlog::info("Processing bytecode [Length: {}]", BytecodeScript.Num());

    // Loop through the Script
    while(Index < Script.size())
    {
        const int IndexCopy = Index; 
        ordered_json InstructionInfo{
            {"InstructionIndex", IndexCopy},
            {"InstructionData", DisasmExpression(Index)}
        };
        Output[sScriptName].push_back(InstructionInfo);
    }

    // Output to file...
    std::filesystem::create_directories(dll_utils::get_current_path() + std::string("/Exports/").append(sClassName));
    std::ofstream OutputStream;
    std::string Filename = "dump-" + sScriptName + ".json";
    OutputStream.open(dll_utils::get_current_path() + std::string("/Exports/").append(sClassName).append("/").append(Filename));
    OutputStream << std::setw(4) << Output << std::endl;
    OutputStream.close();

    // Save to local map
    DisassembledBlueprintFunctions[sScriptName] = Output;
}

ordered_json BlueprintDisasm::DisasmExpression(int& ScriptIndex)
{
    ordered_json ReturnValue{};
    EExprToken Opcode = static_cast<EExprToken>(ReadByte(ScriptIndex));

    switch (Opcode)
    {
    case EX_PrimitiveCast:
        {
            ReturnValue["InstructionType"] = "PrimitiveCast";
            uint8_t ConversionType = ReadByte(ScriptIndex);
            ReturnValue["CastType"] = ConversionType;
            ReturnValue["Expression"] = DisasmExpression(ScriptIndex);
            break;
        }
    case EX_SetSet:
        {
            ReturnValue["InstructionType"] = "SetSet";
            ReturnValue["LeftSideExpression"] = DisasmExpression(ScriptIndex);
            while (Script[ScriptIndex] != EX_EndSet) {
                ReturnValue["Values"].push_back(DisasmExpression(ScriptIndex));
            }
            ScriptIndex++;
            break;
        }
    case EX_SetConst:
        {
            UProperty* InnerProp = reinterpret_cast<UProperty*>(ReadQword(ScriptIndex));
            ReturnValue["InstructionType"] = "SetConst";
            ReturnValue["InnerProperty"] = InnerProp->Class->sGetName();
            while (Script[ScriptIndex] != EX_EndSetConst) {
                ReturnValue["Values"].push_back(DisasmExpression(ScriptIndex));
            }
            ScriptIndex++;
            break;
        }
    case EX_SetMap:
        {
            ReturnValue["InstructionType"] = "SetMap";
            ReturnValue["LeftSideExpression"] = DisasmExpression(ScriptIndex);
            while (Script[ScriptIndex] != EX_EndMap)
            {
                ordered_json Value{
                            {"key", DisasmExpression(ScriptIndex)},
                            {"value", DisasmExpression(ScriptIndex)}
                };

                ReturnValue["Values"].push_back(Value);
            }
            ScriptIndex++;
            break;
        }
    case EX_MapConst:
        {
            ReturnValue["InstructionType"] = "MapConst";

            UProperty* KeyProp = reinterpret_cast<UProperty*>(ReadQword(ScriptIndex));
            UProperty* ValueProp = reinterpret_cast<UProperty*>(ReadQword(ScriptIndex));
            ReturnValue["KeyProperty"] = KeyProp->Class->sGetName().c_str();
            ReturnValue["ValueProperty"] = ValueProp->Class->sGetName().c_str();
            while (Script[ScriptIndex] != EX_EndMapConst)
            {
                ordered_json Pair;
                Pair["Key"] = DisasmExpression(ScriptIndex);
                Pair["Value"] = DisasmExpression(ScriptIndex);
                ReturnValue["Values"].push_back(Pair);
            }
            ScriptIndex++;
            break;
        }
    case EX_TextConst:
        {
            ReturnValue["InstructionType"] = "TextConst";

            switch (static_cast<EBlueprintTextLiteralType>(Script[ScriptIndex++]))
            {
            case Empty:
                {
                    ReturnValue["TextLiteralType"] = "Empty";
                    break;
                }
            case LocalizedText:
                {
                    ReturnValue["TextLiteralType"] = "LocalizedText";
                    std::wstring SourceString = ReadString(ScriptIndex).c_str();
                    std::wstring KeyString = ReadString(ScriptIndex).c_str();
                    std::wstring Namespace = ReadString(ScriptIndex).c_str();
                    TO_STRING(SourceString, sSourceString);
                    TO_STRING(KeyString, sKeyString);
                    TO_STRING(Namespace, sNamespace);
                    ReturnValue["SourceString"] = sSourceString;
                    ReturnValue["LocalizationKey"] = sKeyString;
                    ReturnValue["LocalizationNamespace"] = sNamespace;
                    break;
                }
            case InvariantText:
                {
                    ReturnValue["TextLiteralType"] = "InvariantText";
                    std::wstring SourceString = ReadString(ScriptIndex).c_str();
                    TO_STRING(SourceString, sSourceString)
                    ReturnValue["SourceString"] = sSourceString;
                    break;
                }

            case LiteralString:
                {
                    ReturnValue["TextLiteralType"] = "LiteralString";
                    std::wstring SourceString = ReadString(ScriptIndex).c_str();
                    TO_STRING(SourceString, sSourceString)
                    ReturnValue["SourceString"] = sSourceString;
                    break;
                }
            case StringTableEntry:
                {
                    ReturnValue["TextLiteralType"] = "StringTableEntry";
					
                    ReadQword(ScriptIndex);
                    std::wstring TableIdString = ReadString(ScriptIndex).c_str();
                    std::wstring KeyString = ReadString(ScriptIndex).c_str();
                    TO_STRING(TableIdString, sTableIdString)
                    TO_STRING(KeyString, sKeyString)

                    ReturnValue["TableId"] = sTableIdString;
                    ReturnValue["TableKey"] = sKeyString;
                    break;
                }
            default:
                {
                    ReturnValue["TextLiteralType"] = "Unknown text type.";
                    break;
                }
            }
            break;
        }
    case EX_ObjToInterfaceCast:
        {
            ReturnValue["InstructionType"] = "ObjToInterfaceCast";
            UObject* InterfaceClass = reinterpret_cast<UObject*>(ReadQword(ScriptIndex));

            ReturnValue["InterfaceClass"] = InterfaceClass->sGetName().c_str();
            ReturnValue["Expression"] = DisasmExpression(ScriptIndex);

            break;
        }
    case EX_CrossInterfaceCast:
        {
            ReturnValue["InstructionType"] = "CrossInterfaceCast";
            UObject* InterfaceClass = reinterpret_cast<UObject*>(ReadQword(ScriptIndex));

            ReturnValue["InterfaceClass"] = InterfaceClass->sGetName().c_str();
            ReturnValue["Expression"] = DisasmExpression(ScriptIndex);

            break;
        }
    case EX_InterfaceToObjCast:
        {
            ReturnValue["InstructionType"] = "InterfaceToObjCast";
            UObject* ObjectClass = reinterpret_cast<UObject*>(ReadQword(ScriptIndex));

            ReturnValue["ObjectClass"] = ObjectClass->sGetName().c_str();
            ReturnValue["Expression"] = DisasmExpression(ScriptIndex);

            break; 
        }
    case EX_Let:
        {
            ReturnValue["InstructionType"] = "Let";

            // Property
            ReadQword(ScriptIndex);

            ReturnValue["Variable"] = DisasmExpression(ScriptIndex);
            ReturnValue["Expression"] = DisasmExpression(ScriptIndex);
            
            break;
        }
    case EX_LetObj:
        {
            ReturnValue["InstructionType"] = "LetObj";

            ReturnValue["Variable"] = DisasmExpression(ScriptIndex);
            ReturnValue["Expression"] = DisasmExpression(ScriptIndex);
            
            break;
        }
    case EX_LetWeakObjPtr:
        {
            ReturnValue["InstructionType"] = "LetWeakObjPtr";

            ReturnValue["Variable"] = DisasmExpression(ScriptIndex);
            ReturnValue["Expression"] = DisasmExpression(ScriptIndex);
            
            break;
        }
    case EX_LetBool:
        {
            ReturnValue["InstructionType"] = "LetBool";

            ReturnValue["Variable"] = DisasmExpression(ScriptIndex);
            ReturnValue["Expression"] = DisasmExpression(ScriptIndex);
            
            break;
        }
    case EX_LetValueOnPersistentFrame:
        {
            ReturnValue["InstructionType"] = "LetValueOnPersistentFrame";

            UProperty* Property = reinterpret_cast<UProperty*>(ReadQword(ScriptIndex));
            ReturnValue["PropertyType"] = Property->Class->sGetName().c_str();
            ReturnValue["PropertyName"] = Property->sGetName().c_str();
            ReturnValue["Expression"] = DisasmExpression(ScriptIndex);
            
            break;
        }
    case EX_StructMemberContext:
        {
            ReturnValue["InstructionType"] = "StructMemberContext";

            UProperty* Property = reinterpret_cast<UProperty*>(ReadQword(ScriptIndex));
            ReturnValue["PropertyType"] = Property->Class->sGetName().c_str();
            ReturnValue["PropertyName"] = Property->sGetName().c_str();
            ReturnValue["StructExpression"] = DisasmExpression(ScriptIndex);
            
            break;
        }
    case EX_LetDelegate:
        {
            ReturnValue["InstructionType"] = "LetDelegate";

            ReturnValue["Variable"] = DisasmExpression(ScriptIndex);
            ReturnValue["Expression"] = DisasmExpression(ScriptIndex);
            
            break;
        }
    case EX_LetMulticastDelegate:
        {
            ReturnValue["InstructionType"] = "LetMulticastDelegate";

            ReturnValue["Variable"] = DisasmExpression(ScriptIndex);
            ReturnValue["Expression"] = DisasmExpression(ScriptIndex);
            
            break;
        }
    case EX_ComputedJump:
        {
            ReturnValue["InstructionType"] = "ComputedJump";
            ReturnValue["OffsetExpression"] = DisasmExpression(ScriptIndex);
            break;
        }
    case EX_Jump:
        {
            ReturnValue["InstructionType"] = "Jump";
            ReturnValue["Offset"] = ReadSkipCount(ScriptIndex);
            break;
        }
    case EX_LocalVariable:
        {
            ReturnValue["InstructionType"] = "LocalVariable";

            UProperty* Property = reinterpret_cast<UProperty*>(ReadQword(ScriptIndex));
            if (!memory_manager::is_bad_read_pointer(Property) && Property && Property->Class)
            {
                ReturnValue["VariableType"] = Property->Class->sGetName().c_str();
                ReturnValue["VariableName"] = Property->sGetName().c_str();
            }
            break;
        }
    case EX_DefaultVariable:
        {
            ReturnValue["InstructionType"] = "DefaultVariable";

            UProperty* Property = reinterpret_cast<UProperty*>(ReadQword(ScriptIndex));
            if (!memory_manager::is_bad_read_pointer(Property) && Property && Property->Class)
            {
                ReturnValue["VariableType"] = Property->Class->sGetName().c_str();
                ReturnValue["VariableName"] = Property->sGetName().c_str();
            }
            break;
        }
    case EX_InstanceVariable:
        {
            ReturnValue["InstructionType"] = "InstanceVariable";

            UProperty* Property = reinterpret_cast<UProperty*>(ReadQword(ScriptIndex));
            if (!memory_manager::is_bad_read_pointer(Property) && Property && Property->Class)
            {
                ReturnValue["VariableType"] = Property->Class->sGetName().c_str();
                ReturnValue["VariableName"] = Property->sGetName().c_str();
            }
            break;
        }
    case EX_LocalOutVariable:
        {
            ReturnValue["InstructionType"] = "LocalOutVariable";

            UProperty* Property = reinterpret_cast<UProperty*>(ReadQword(ScriptIndex));
            if (!memory_manager::is_bad_read_pointer(Property) && Property && Property->Class)
            {
                ReturnValue["VariableType"] = Property->Class->sGetName().c_str();
                ReturnValue["VariableName"] = Property->sGetName().c_str();
            }
            break;
        }
    case EX_InterfaceContext:
        {
            ReturnValue["InstructionType"] = "InterfaceContext";
            ReturnValue["Expression"] = DisasmExpression(ScriptIndex);
            break;
        }
    case EX_DeprecatedOp4A:
        {
            ReturnValue["InstructionType"] = "DeprecatedOp4A";
            break;
        }
    case EX_Nothing:
        {
            ReturnValue["InstructionType"] = "Nothing";
            break;
        }
    case EX_EndOfScript:
        {
            ReturnValue["InstructionType"] = "EndOfScript";
            break;
        }
    case EX_IntZero:
        {
            ReturnValue["InstructionType"] = "IntZero";
            break;
        }
    case EX_IntOne:
        {
            ReturnValue["InstructionType"] = "IntOne";
            break;
        }
    case EX_True:
        {
            ReturnValue["InstructionType"] = "True";
            break;
        }
    case EX_False:
        {
            ReturnValue["InstructionType"] = "False";
            break;
        }
    case EX_NoObject:
        {
            ReturnValue["InstructionType"] = "NoObject";
            break;
        }
    case EX_NoInterface:
        {
            ReturnValue["InstructionType"] = "NoInterface";
            break;
        }
    case EX_Self:
        {
            ReturnValue["InstructionType"] = "Self";
            break;
        }
    case EX_Return:
        {
            ReturnValue["InstructionType"] = "Return";
            ReturnValue["Expression"] = DisasmExpression(ScriptIndex);
            break;
        }
    case EX_CallMath:
        {
            ReturnValue["InstructionType"] = "CallMath";

            UFunction* StackNode = reinterpret_cast<UFunction*>(ReadQword(ScriptIndex));
            ReturnValue["Function"] = StackNode->sGetName().c_str();

            while (Script[ScriptIndex] != EX_EndFunctionParms)
            {
                ordered_json ParameterStatement = DisasmExpression(ScriptIndex);
                ReturnValue["Parameters"].push_back(ParameterStatement);
            }
            ScriptIndex++;
            break;
        }
    case EX_LocalFinalFunction:
    case EX_FinalFunction:
        {
            ReturnValue["InstructionType"] = "FinalFunction";
            UFunction* StackNode = reinterpret_cast<UFunction*>(ReadQword(ScriptIndex));

            ReturnValue["Function"] = StackNode->sGetName().c_str();
            while (Script[ScriptIndex] != EX_EndFunctionParms)
            {
                ordered_json ParameterStatement = DisasmExpression(ScriptIndex);
                ReturnValue["Parameters"].push_back(ParameterStatement);
            }
            ScriptIndex++;
            break;
        }
    case EX_CallMulticastDelegate:
        {
            ReturnValue["InstructionType"] = "CallMulticastDelegate";
            UFunction* StackNode = reinterpret_cast<UFunction*>(ReadQword(ScriptIndex));

            ReturnValue["Function"] = StackNode->sGetName().c_str();
            ReturnValue["Delegate"] = DisasmExpression(ScriptIndex);
            while (Script[ScriptIndex] != EX_EndFunctionParms)
            {
                ordered_json ParameterStatement = DisasmExpression(ScriptIndex);
                ReturnValue["Parameters"].push_back(ParameterStatement);
            }
            ScriptIndex++;
            break;
        }
    case EX_LocalVirtualFunction:
    case EX_VirtualFunction:
        {
            ReturnValue["InstructionType"] = "VirtualFunction";
            FString FunctionName = ReadName(ScriptIndex);
            std::wstring wFunctionName = FunctionName.c_str();
            TO_STRING(wFunctionName, sFunctionName);

            ReturnValue["FunctionName"] = sFunctionName;
            while (Script[ScriptIndex] != EX_EndFunctionParms)
            {
                ordered_json ParameterStatement = DisasmExpression(ScriptIndex);
                ReturnValue["Parameters"].push_back(ParameterStatement);
            }
            ScriptIndex++;
            break;
        }
    case EX_ClassContext:
    case EX_Context:
    case EX_Context_FailSilent:
        {
            if (Opcode == EX_ClassContext)
            {
                ReturnValue["InstructionType"] = "ClassContext";
            }
            else if (Opcode == EX_Context)
            {
                ReturnValue["InstructionType"] = "Context";
            }
            else
            {
                ReturnValue["InstructionType"] = "Context_FailSilent";
            }

            ReturnValue["Context"] = DisasmExpression(ScriptIndex);
            ReturnValue["SkipOffsetForNull"] = ReadSkipCount(ScriptIndex);

            UProperty* Field = reinterpret_cast<UProperty*>(ReadQword(ScriptIndex));
            if (Field)
            {
                ReturnValue["RValuePropertyType"] = Field->Class->sGetName().c_str();
                ReturnValue["RValuePropertyName"] = Field->sGetName().c_str();
            }
            
            ReturnValue["Expression"] = DisasmExpression(ScriptIndex);
            break;
        }
    case EX_IntConst:
        {
            int ConstValue = ReadInt(ScriptIndex);
            ReturnValue["InstructionType"] = "IntConst";
            ReturnValue["Value"] = ConstValue;
            break;
        }
    case EX_SkipOffsetConst:
        {
            int ConstValue = ReadSkipCount(ScriptIndex);
            ReturnValue["InstructionType"] = "SkipOffsetConst";
            ReturnValue["Value"] = ConstValue;
            break;
        }
    case EX_FloatConst:
        {
            float ConstValue = ReadFloat(ScriptIndex);
            ReturnValue["InstructionType"] = "FloatConst";
            ReturnValue["Value"] = ConstValue;
            break;
        }
    case EX_StringConst:
        {
            FString ConstValue = ReadString8(ScriptIndex);
            ReturnValue["InstructionType"] = "StringConst";
            std::wstring wConstValue = ConstValue.c_str();
            TO_STRING(wConstValue, sConstValue);
            ReturnValue["Value"] = sConstValue;
            break;
        }
    case EX_UnicodeStringConst:
        {
            FString ConstValue = ReadString16(ScriptIndex);
            ReturnValue["InstructionType"] = "UnicodeStringConst";
            std::wstring wConstValue = ConstValue.c_str();
            TO_STRING(wConstValue, sConstValue);
            ReturnValue["Value"] = sConstValue;
            break;
        }
    case EX_ObjectConst:
        {
            ReturnValue["InstructionType"] = "ObjectConst";
            UObject* Pointer = reinterpret_cast<UObject*>(ReadQword(ScriptIndex));
            ReturnValue["Object"] = Pointer->sGetName().c_str();
            break;
        }
    case EX_SoftObjectConst:
        {
            ReturnValue["InstructionType"] = "SoftObjectConst";
            ReturnValue["Value"] = DisasmExpression(ScriptIndex);
            break;
        }
    case EX_NameConst:
        {
            ReturnValue["InstructionType"] = "NameConst";
            FString ConstValue = ReadName(ScriptIndex);
            std::wstring wConstValue = ConstValue.c_str();
            TO_STRING(wConstValue, sConstValue);
            ReturnValue["Value"] = sConstValue;
            break;
        }
    case EX_RotationConst:
        {
            ReturnValue["InstructionType"] = "RotationConst";
            ReturnValue["Pitch"] = ReadFloat(ScriptIndex);
            ReturnValue["Yaw"] = ReadFloat(ScriptIndex);
            ReturnValue["Roll"] = ReadFloat(ScriptIndex);
            break;
        }
    case EX_VectorConst:
        {
            ReturnValue["InstructionType"] = "VectorConst";
            ReturnValue["X"] = ReadFloat(ScriptIndex);
            ReturnValue["Y"] = ReadFloat(ScriptIndex);
            ReturnValue["Z"] = ReadFloat(ScriptIndex);
            break;
        }
    case EX_TransformConst:
        {
            ReturnValue["InstructionType"] = "TransformConst";
            ReturnValue["Rotation"]["X"] = ReadFloat(ScriptIndex);
            ReturnValue["Rotation"]["Y"] = ReadFloat(ScriptIndex);
            ReturnValue["Rotation"]["Z"] = ReadFloat(ScriptIndex);
            ReturnValue["Rotation"]["W"] = ReadFloat(ScriptIndex);
            ReturnValue["Translation"]["X"] = ReadFloat(ScriptIndex);
            ReturnValue["Translation"]["Y"] = ReadFloat(ScriptIndex);
            ReturnValue["Translation"]["Z"] = ReadFloat(ScriptIndex);
            ReturnValue["Scale"]["X"] = ReadFloat(ScriptIndex);
            ReturnValue["Scale"]["Y"] = ReadFloat(ScriptIndex);
            ReturnValue["Scale"]["Z"] = ReadFloat(ScriptIndex);
            break;
        }
    case EX_StructConst:
        {
            ReturnValue["InstructionType"] = "StructConst";
            UObject* Struct = reinterpret_cast<UObject*>(ReadQword(ScriptIndex));
            ReturnValue["Struct"] = Struct->sGetName().c_str();
            ReturnValue["StructSize"] = ReadInt(ScriptIndex);
            for (UObject* Property = *reinterpret_cast<UObject**>(reinterpret_cast<__int64>(Struct) + CoreOffsets::Children); Property; Property = *reinterpret_cast<UObject**>(reinterpret_cast<__int64>(Property) + CoreOffsets::Next))
            {
                if (Property->IsA(UProperty::StaticClass()))
                {
                    uint64 PropertyFlags = *reinterpret_cast<uint64*>(reinterpret_cast<uintptr_t>(Property) + CoreOffsets::PropertyFlags);
                    if (PropertyFlags & CPF_Transient || PropertyFlags & CPF_EditorOnly)
                    {
                        continue;
                    }
                    
                    ordered_json PropertyValue{
                            {Property->sGetName(), DisasmExpression(ScriptIndex)}
                    };
                    ReturnValue["Properties"].push_back(PropertyValue);
                }
            }
            while (Script[ScriptIndex] != EX_EndStructConst)
            {
                DisasmExpression(ScriptIndex);
            }
            ScriptIndex++;
            break;
        }
    case EX_SetArray:
        {
            ReturnValue["InstructionType"] = "SetArray";
            ReturnValue["LeftSideExpression"] = DisasmExpression(ScriptIndex);
            while (Script[ScriptIndex] != EX_EndArray)
            {
                ReturnValue["Values"].push_back(DisasmExpression(ScriptIndex));
            }
            ScriptIndex++;
            break;
        }
    case EX_ArrayConst:
        {
            UProperty* InnerProp = reinterpret_cast<UProperty*>(ReadQword(ScriptIndex));
            ReturnValue["InstructionType"] = "ArrayConst";
            ReturnValue["InnerProperty"] = InnerProp->Class->sGetName().c_str();
            ReturnValue["Elements"] = ReadInt(ScriptIndex);
            while (Script[ScriptIndex] != EX_EndArrayConst)
            {
                ReturnValue["Values"].push_back(DisasmExpression(ScriptIndex));
            }
            ScriptIndex++;
            break;
        }
    case EX_ByteConst:
        {
            ReturnValue["InstructionType"] = "ByteConst";
            ReturnValue["Value"] = ReadByte(ScriptIndex);
            break;
        }
    case EX_IntConstByte:
        {
            ReturnValue["InstructionType"] = "IntConstByte";
            ReturnValue["Value"] = ReadByte(ScriptIndex);
            break;
        }
    case EX_Int64Const:
        {
            ReturnValue["InstructionType"] = "Int64Const";
            ReturnValue["Value"] = ReadQword(ScriptIndex);
            break;
        }
    case EX_UInt64Const:
        {
            ReturnValue["InstructionType"] = "UInt64Const";
            ReturnValue["Value"] = ReadQword(ScriptIndex);
            break;
        }
    case EX_MetaCast:
        {
            ReturnValue["InstructionType"] = "MetaCast";
            UObject* Class = reinterpret_cast<UObject*>(ReadQword(ScriptIndex));
            ReturnValue["Class"] = Class->sGetName().c_str();
            ReturnValue["Expression"] = DisasmExpression(ScriptIndex);
            break;
        }
    case EX_DynamicCast:
        {
            ReturnValue["InstructionType"] = "DynamicCast";
            UObject* Class = reinterpret_cast<UObject*>(ReadQword(ScriptIndex));
            ReturnValue["Class"] = Class->sGetName().c_str();
            ReturnValue["Expression"] = DisasmExpression(ScriptIndex);
            break;
        }
    case EX_JumpIfNot:
        {
            ReturnValue["InstructionType"] = "JumpIfNot";
            ReturnValue["Offset"] = ReadSkipCount(ScriptIndex);
            ReturnValue["Condition"] = DisasmExpression(ScriptIndex);
            break;
        }
    case EX_Assert:
        {
            ReturnValue["InstructionType"] = "Assert";
            uint16 LineNumber = ReadWord(ScriptIndex);
            uint8 InDebugMode = ReadByte(ScriptIndex);
            
            ReturnValue["LineNumber"] = LineNumber;
            ReturnValue["Debug"] = static_cast<bool>(InDebugMode);
            ReturnValue["Expression"] = DisasmExpression(ScriptIndex);
            break;
        }
    case EX_InstanceDelegate:
        {
            ReturnValue["InstructionType"] = "InstanceDelegate";
            FString FuncName = ReadName(ScriptIndex);
            std::wstring wFuncName = FuncName.c_str();
            TO_STRING(wFuncName, sFuncName);
            ReturnValue["FunctionName"] = sFuncName;
            break;
        }
    case EX_AddMulticastDelegate:
        {
            ReturnValue["InstructionType"] = "AddMulticastDelegate";
            ReturnValue["MulticastDelegate"] = DisasmExpression(ScriptIndex);
            ReturnValue["Delegate"] = DisasmExpression(ScriptIndex);
            break;
        }
    case EX_RemoveMulticastDelegate:
        {
            ReturnValue["InstructionType"] = "RemoveMulticastDelegate";
            ReturnValue["MulticastDelegate"] = DisasmExpression(ScriptIndex);
            ReturnValue["Delegate"] = DisasmExpression(ScriptIndex);
            break;
        }
    case EX_ClearMulticastDelegate:
        {
            ReturnValue["InstructionType"] = "ClearMulticastDelegate";
            ReturnValue["MulticastDelegate"] = DisasmExpression(ScriptIndex);
            break;
        }
    case EX_BindDelegate:
        {
            ReturnValue["InstructionType"] = "BindDelegate";
            FString FuncName = ReadName(ScriptIndex);
            std::wstring wFuncName = FuncName.c_str();
            TO_STRING(wFuncName, sFuncName);
            ReturnValue["FunctionName"] = sFuncName;
            ReturnValue["Delegate"] = DisasmExpression(ScriptIndex);
            ReturnValue["Object"] = DisasmExpression(ScriptIndex);
            break;
        }
    case EX_PushExecutionFlow:
        {
            ReturnValue["InstructionType"] = "PushExecutionFlow";
            ReturnValue["Offset"] = ReadSkipCount(ScriptIndex);
            break;
        }
    case EX_PopExecutionFlow:
        {
            ReturnValue["InstructionType"] = "PopExecutionFlow";
            break;
        }
    case EX_PopExecutionFlowIfNot:
        {
            ReturnValue["InstructionType"] = "PopExecutionFlowIfNot";
            ReturnValue["Condition"] = DisasmExpression(ScriptIndex);
            break;
        }
    case EX_Breakpoint:
        {
            ReturnValue["InstructionType"] = "BreakPoint";
            break;
        }
    case EX_WireTracepoint:
        {
            ReturnValue["InstructionType"] = "WireTracepoint";
            break;
        }
    case EX_InstrumentationEvent:
        {
            ReturnValue["InstructionType"] = "InstrumentationEvent";
            // Ignore
            ReadByte(ScriptIndex);
            ReadName(ScriptIndex);
            break;
        }
    case EX_Tracepoint:
        {
            ReturnValue["InstructionType"] = "Tracepoint";
            break;
        }
    case EX_SwitchValue:
        {
            ReturnValue["InstructionType"] = "SwitchValue";
            
            uint16 NumCases = ReadWord(ScriptIndex);
            uint16 AfterSkip = ReadSkipCount(ScriptIndex);
            ReturnValue["Expression"] = DisasmExpression(ScriptIndex);
            ReturnValue["AfterSkip"] = AfterSkip;
            for (uint16 i = 0; i < NumCases; i++)
            {
                ordered_json Case;
                Case["CaseValue"] = DisasmExpression(ScriptIndex);
                Case["OffsetToNextCase"] = ReadSkipCount(ScriptIndex);
                Case["CaseResult"] = DisasmExpression(ScriptIndex);
                ReturnValue["Cases"].push_back(Case);
            }
            ReturnValue["DefaultResult"] = DisasmExpression(ScriptIndex);
            break;
        }
    case EX_ArrayGetByRef:
        {
            ReturnValue["InstructionType"] = "ArrayGetByRef";
            ReturnValue["ArrayExpression"] = DisasmExpression(ScriptIndex);
            ReturnValue["IndexExpression"] = DisasmExpression(ScriptIndex);
            break;
        }
    default:
        {
            ReturnValue["InstructionType"] = std::string("Unknown opcode: ").append(std::to_string(Opcode));
            spdlog::error("BlueprintDisasm::DisasmExpression: Unsupported opcode {}", static_cast<int>(Opcode));
            break;
        }
    }
    
    return ReturnValue;
}

uint8 BlueprintDisasm::ReadByte(int32& ScriptIndex)
{
    uint8 Value = Script[ScriptIndex]; ++ScriptIndex;
    return Value;
}

uint16 BlueprintDisasm::ReadWord(int32& ScriptIndex)
{
    uint16 Value = Script[ScriptIndex]; ++ScriptIndex;
    Value = Value | static_cast<uint16>(Script[ScriptIndex]) << 8; ++ScriptIndex;
    return Value;
}

int32 BlueprintDisasm::ReadInt(int32& ScriptIndex)
{
    int32 Value = Script[ScriptIndex]; ++ScriptIndex;
    Value = Value | static_cast<int32>(Script[ScriptIndex]) << 8; ++ScriptIndex;
    Value = Value | static_cast<int32>(Script[ScriptIndex]) << 16; ++ScriptIndex;
    Value = Value | static_cast<int32>(Script[ScriptIndex]) << 24; ++ScriptIndex;
    return Value;
}

uint64 BlueprintDisasm::ReadQword(int32& ScriptIndex)
{
    uint64 Value = Script[ScriptIndex]; ++ScriptIndex;
    Value = Value | static_cast<uint64>(Script[ScriptIndex]) << 8; ++ScriptIndex;
    Value = Value | static_cast<uint64>(Script[ScriptIndex]) << 16; ++ScriptIndex;
    Value = Value | static_cast<uint64>(Script[ScriptIndex]) << 24; ++ScriptIndex;
    Value = Value | static_cast<uint64>(Script[ScriptIndex]) << 32; ++ScriptIndex;
    Value = Value | static_cast<uint64>(Script[ScriptIndex]) << 40; ++ScriptIndex;
    Value = Value | static_cast<uint64>(Script[ScriptIndex]) << 48; ++ScriptIndex;
    Value = Value | static_cast<uint64>(Script[ScriptIndex]) << 56; ++ScriptIndex;
    return Value;
}

float BlueprintDisasm::ReadFloat(int32& ScriptIndex)
{
    union { float f; int32 i; } Result;
    Result.i = ReadInt(ScriptIndex);
    return Result.f;
}

FString BlueprintDisasm::ReadName(int32& ScriptIndex)
{
    FScriptName Value = *reinterpret_cast<FScriptName*>(Script.data() + ScriptIndex);
    ScriptIndex += sizeof FScriptName;
    
    return Value.ToName().ToString();
}

int BlueprintDisasm::ReadSkipCount(int32& ScriptIndex)
{
    return ReadInt(ScriptIndex);
}

FString BlueprintDisasm::ReadString8(int32& ScriptIndex)
{
    std::string Result;
    
    do {
        Result += static_cast<char>(ReadByte(ScriptIndex));
    }
    while (Script[ScriptIndex-1] != 0);

    TO_WSTRING(Result, wResult);

    return wResult;
}

FString BlueprintDisasm::ReadString16(int32& ScriptIndex)
{
    std::wstring Result;
    
    do {
        Result += ReadByte(ScriptIndex);
    }
    while (Script[ScriptIndex-1] != 0);

    return Result;
}

FString BlueprintDisasm::ReadString(int32& ScriptIndex)
{
    switch (static_cast<EExprToken>(Script[ScriptIndex++]))
    {
    case EX_StringConst:
        return ReadString8(ScriptIndex);
    case EX_UnicodeStringConst:
        return ReadString16(ScriptIndex);
    default:
        return TEXT("BlueprintDisasm::ReadString: Unexpected opcode.");
    }
}
