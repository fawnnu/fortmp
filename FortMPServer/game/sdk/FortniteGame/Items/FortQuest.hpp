#pragma once

struct FFortMcpQuestObjectiveInfo
{
    DEFINE_STRUCT_PROPERTY(FName, BackendName)
    DEFINE_STRUCT_PROPERTY(FCurveTableRowHandle, ObjectiveStatHandle);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortMcpQuestObjectiveInfo")
};

class UFortQuestItemDefinition : public UFortItemDefinition
{
public:
    DEFINE_PROPERTY(TArray<FFortMcpQuestObjectiveInfo>, Objectives);
};

struct FFortUpdatedObjectiveStat
{
    DEFINE_STRUCT_PROPERTY(UFortQuestItemDefinition*, Quest);
    DEFINE_STRUCT_PROPERTY(FName, BackendName);
    DEFINE_STRUCT_PROPERTY(int, StatValue);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortUpdatedObjectiveStat")
};

class UFortQuestManager : public UObject
{
public:
    
};

class UFortAsyncAction_SendQuestStatEvent : public UObject
{
public:
    static UFortAsyncAction_SendQuestStatEvent* SendClientStatEvent(UFortQuestManager* QuestManager, FCurveTableRowHandle& ObjectiveStat, int Count)
    {
        static UFortAsyncAction_SendQuestStatEvent* Default__FortAsyncAction_SendQuestStatEvent = FindObject<UFortAsyncAction_SendQuestStatEvent>(L"/Script/FortniteGame.Default__FortAsyncAction_SendQuestStatEvent");
        
        STATIC_FINDFUNCTIONOBJECT(L"/Script/FortniteGame.FortAsyncAction_SendQuestStatEvent.SendClientStatEvent")

        struct
        {
            UFortQuestManager* QuestManager;
            FCurveTableRowHandle ObjectiveStat;
            int Count;
            UFortAsyncAction_SendQuestStatEvent* ReturnValue;
        } FunctionParameters;

        FunctionParameters.QuestManager = QuestManager;
        FunctionParameters.ObjectiveStat = ObjectiveStat;
        FunctionParameters.Count = Count;

        spdlog::info(L"Sending SendClientStatEvent [{}, {}, {}, {}]", QuestManager->GetName().c_str(), ObjectiveStat.CurveTable->GetName().c_str(), ObjectiveStat.RowName.ToString().c_str(), Count);

        ProcessEvent(Default__FortAsyncAction_SendQuestStatEvent, FunctionObject, &FunctionParameters);

        return FunctionParameters.ReturnValue;
    }
};

class AProp_QuestInteractable_Letter_Parent_C : public ABuildingSMActor
{
public:
    DEFINE_PROPERTY(UFortQuestItemDefinition*, QuestItem);
    DEFINE_PROPERTY(FName, QuestBackendObjectiveName);

    DEFINE_STATIC_CLASS(L"/Game/Athena/Items/QuestInteractables/FortnightLetters/Prop_QuestInteractable_Letters_Parent.Prop_QuestInteractable_Letters_Parent_C");
};

class AProp_QuestInteractable_RubberDuckies_Parent_C : public ABuildingSMActor
{
public:
    DEFINE_PROPERTY(UFortQuestItemDefinition*, QuestItem);
    DEFINE_PROPERTY(FName, QuestBackendObjectiveName);

    DEFINE_STATIC_CLASS(L"/Game/Athena/Items/QuestInteractables/RubberDuckies/Prop_QuestInteractable_RubberDuckies_Parent.Prop_QuestInteractable_RubberDuckies_Parent_C");
};

class AProp_QuestObject_Interactable_Parent_C : public ABuildingSMActor
{
public:
    DEFINE_PROPERTY(UFortQuestItemDefinition*, QuestItem);
    DEFINE_PROPERTY(FName, QuestBackendObjectiveName);
    
    DEFINE_STATIC_CLASS(L"/Game/Athena/Items/QuestInteractables/Generic/Prop_QuestObject_Interactable_Parent.Prop_QuestObject_Interactable_Parent_C");
};