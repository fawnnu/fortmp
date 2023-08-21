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

    DEFINE_CLASS(L"/Game/Athena/Items/QuestInteractables/FortnightLetters/Prop_QuestInteractable_Letters_Parent.Prop_QuestInteractable_Letters_Parent_C");
};

class AProp_Quest_GniceGnomes_Parent_C : public ABuildingSMActor
{
public:
    DEFINE_PROPERTY(UFortQuestItemDefinition*, QuestItem);
    DEFINE_PROPERTY(FName, QuestBackendObjectiveName);

    DEFINE_CLASS(L"/Game/Athena/Items/QuestInteractables/GniceGnomes/Prop_Quest_GniceGnomes_Parent.Prop_Quest_GniceGnomes_Parent_C");
};

class AProp_QuestInteractable_RubberDuckies_Parent_C : public ABuildingSMActor
{
public:
    DEFINE_PROPERTY(UFortQuestItemDefinition*, QuestItem);
    DEFINE_PROPERTY(FName, QuestBackendObjectiveName);

    DEFINE_CLASS(L"/Game/Athena/Items/QuestInteractables/RubberDuckies/Prop_QuestInteractable_RubberDuckies_Parent.Prop_QuestInteractable_RubberDuckies_Parent_C");
};

class AProp_QuestActor_FloatingLightningBolt_Parent_C : public ABuildingSMActor
{
public:
    DEFINE_PROPERTY(UFortQuestItemDefinition*, QuestItem);
    DEFINE_PROPERTY(FName, QuestBackendObjectiveName);

    DEFINE_CLASS(L"/Game/Athena/Items/QuestInteractables/FloatingLightningBolts/Prop_QuestActor_FloatingLightningBolt_Parent.Prop_QuestActor_FloatingLightningBolt_Parent_C");
};

class AProp_QuestObject_Interactable_Parent_C : public ABuildingSMActor
{
public:
    DEFINE_PROPERTY(UFortQuestItemDefinition*, QuestItem);
    DEFINE_PROPERTY(FName, QuestBackendObjectiveName);
    
    DEFINE_CLASS(L"/Game/Athena/Items/QuestInteractables/Generic/Prop_QuestObject_Interactable_Parent.Prop_QuestObject_Interactable_Parent_C");
};

class AProp_QuestInteractable_Parent_C : public ABuildingSMActor
{
public:
    DEFINE_PROPERTY(UFortQuestItemDefinition*, QuestItem);
    DEFINE_PROPERTY(FName, QuestBackendObjectiveName);
};

class AProp_QuestInteractable_HiddenStar_Parent_C : public AProp_QuestInteractable_Parent_C
{
public:
    DEFINE_CLASS(L"/Game/Athena/Items/QuestInteractables/HiddenStars/Prop_QuestInteractable_HiddenStar_Parent.Prop_QuestInteractable_HiddenStar_Parent_C");
};

class ABA_QuestInteractable_DancePlatforms : public AActor
{
public:
    DEFINE_PROPERTY(TArray<AFortPlayerPawn*>, Platform1_Pawns);
    DEFINE_PROPERTY(TArray<AFortPlayerPawn*>, Platform2_Pawns);
    DEFINE_PROPERTY(TArray<AFortPlayerPawn*>, Platform3_Pawns);
    DEFINE_PROPERTY(TArray<AFortPlayerPawn*>, Platform4_Pawns);
    DEFINE_PROPERTY(FName, QuestObjBackendName);
    
    DEFINE_CLASS(L"/Game/Athena/Items/QuestInteractables/DancePlatforms/BA_QuestInteractable_DancePlatforms.BA_QuestInteractable_DancePlatforms_C");
};

class ABA_QuestInteractable_StormCenter_Single : public AActor
{
public:
    DEFINE_PROPERTY(TArray<APlayerState*>, PlayersWhoHaveTouchedThisCenter);
    DEFINE_CLASS(L"/Game/Athena/Items/QuestInteractables/StormCenter/BA_QuestInteractable_StormCenter_Single.BA_QuestInteractable_StormCenter_Single_C");    
};

class AAthena_SoccerGame_C : public AActor
{
public:
    DEFINE_PROPERTY(AFortPawn*, PlayerWhoScored);
    DEFINE_PROPERTY(FName, BackendObjName);
};

class AProp_MountainSummit_QuestInteractable_Parent : public AActor
{
public:
    DEFINE_CLASS(L"/Game/Athena/Items/QuestInteractables/MountainSummit/Prop_MountainSummit_QuestInteractable_Parent.Prop_MountainSummit_QuestInteractable_Parent_C");
    
    DEFINE_PROPERTY(UFortQuestItemDefinition*, QuestItem);
    DEFINE_PROPERTY(FName, QuestBackendObjectiveName);
};

class AProp_SkydiveRings_QuestInteractable : public AActor
{
public:
    DEFINE_CLASS(L"/Game/Athena/Items/QuestInteractables/SkydiveRings/Prop_SkydiveRings_QuestInteractable.Prop_SkydiveRings_QuestInteractable_C");
};