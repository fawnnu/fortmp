#pragma once

struct FEnvQueryResult
{
    TWeakObjectPtr<UObject>& Owner()
    {
        static void* native_property = FindPropertyByName(GetClass(), "QueryID");
        static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);

        return *reinterpret_cast<TWeakObjectPtr<UObject>*>(__int64(this) + native_property_offset + 4);
    }

    DEFINE_STRUCT_PROPERTY(int, QueryID);
    
    DEFINE_STRUCT_CLASS(L"/Script/AIModule.EnvQueryResult");
};

class UEnvQuery : public UObject
{
public:
    
};

class UEnvQueryInstanceBlueprintWrapper : public UObject
{
public:
    DEFINE_PROPERTY(UObject*, ItemType);
    DEFINE_PROPERTY(TScriptMulticastDelegate<UEnvQueryInstanceBlueprintWrapper>, OnQueryFinishedEvent)
    DEFINE_PROPERTY_BY_OFFSET(UE::TSharedPtr<FEnvQueryResult>, QueryResult, 0x38);
    
    TArray<FVector> GetResultsAsLocations()
    {
        static UFunction* function = FindObject<UFunction>(L"/Script/AIModule.EnvQueryInstanceBlueprintWrapper.GetResultsAsLocations");
        
        struct params
        {
            // Return value
            TArray<FVector> ReturnValue;
        } function_params;
        
        ProcessEvent(this, function, &function_params);

        return function_params.ReturnValue;
    }
};

class UEnvQueryManager : public UObject
{
public:
    static UEnvQueryInstanceBlueprintWrapper* RunEQSQuery(UWorld* WorldContextObject, UEnvQuery* QueryTemplate, UObject* Querier, uint8_t RunMode, UObject* WrapperClass)
    {
        static UEnvQueryManager* EnvQueryManager__Default = FindObject<UEnvQueryManager>(L"/Script/AIModule.Default__EnvQueryManager");

        STATIC_FINDFUNCTIONOBJECT(L"/Script/AIModule.EnvQueryManager.RunEQSQuery")
        
        return EnvQueryManager__Default->Call<UEnvQueryInstanceBlueprintWrapper*>(FunctionObject, WorldContextObject, QueryTemplate, Querier, RunMode, WrapperClass);
    }
};