#pragma once

class ABP_Athena_Event_Components_C : public AActor
{
public:
    DEFINE_CLASS(L"/Game/Athena/Events/BP_Athena_Event_Components.BP_Athena_Event_Components_C");

    DEFINE_PROPERTY(float, CrackProgression);
    DEFINE_PROPERTY(int, AreCracksVisible);
    DEFINE_PROPERTY(int, AreEndSequenceCrackVisible);
};

class ABP_GeodeScripting_C : public AActor
{
public:

    DEFINE_CLASS(L"/Game/Athena/Maps/Test/Events/BP_GeodeScripting.BP_GeodeScripting_C")
    
    void LaunchSequence()
    {
        STATIC_FINDFUNCTION(LaunchSequence)

        Call(Function_LaunchSequence);
    }

    void Event_TurnOffGodRay()
    {
        STATIC_FINDFUNCTION(Event_TurnOffGodRay)

        Call(Function_Event_TurnOffGodRay);
    }
    
    void Event_TurnOnGodRay()
    {
        STATIC_FINDFUNCTION(Event_TurnOnGodRay)

        Call(Function_Event_TurnOnGodRay);
    }
};

class ACUBE_C : public AActor
{
public:

    DEFINE_CLASS(L"/Game/Athena/Prototype/Blueprints/Cube/CUBE.CUBE_C")

    DEFINE_PROPERTY(int, PathIndex);
    DEFINE_PROPERTY(int, CurrentIndex);
    DEFINE_PROPERTY(int, StartingIndex);

    // Use this if you're gonna force the cube to move forward.
    void NextStepDynamic()
    {
        TArray<AActor*> cube_scripter = UGameplayStatics::GetAllActorsOfClass((*GWorld), ACUBE_C::StaticClass());
        for (auto actor : cube_scripter)
        {
            ACUBE_C* cube = static_cast<ACUBE_C*>(actor);
            cube->Next(cube->PathIndex() + 1);
        }
    }
    
    void Show()
    {
        STATIC_FINDFUNCTION(Show)

        Call(Function_Show);
    }

    void SpawnCube()
    {
        STATIC_FINDFUNCTION(SpawnCube)

        Call(Function_SpawnCube);
    }

    void Next(int NewParam)
    {
        STATIC_FINDFUNCTION(Next)

        Call(Function_Next, NewParam);
    }

    void New(int NewParam)
    {
        STATIC_FINDFUNCTION(New)

        Call(Function_New, NewParam);
    }
    
    void RollForward()
    {
        STATIC_FINDFUNCTION(Rollforward)

        Call(Function_Rollforward);
    }

    void OnRep_PathIndex()
    {
        STATIC_FINDFUNCTION(OnRep_PathIndex)

        Call(Function_OnRep_PathIndex);
    }

    void OnRep_StartingIndex()
    {
        STATIC_FINDFUNCTION(OnRep_StartingIndex)

        Call(Function_OnRep_StartingIndex);
    }

    void IncrementIndex()
    {
        STATIC_FINDFUNCTION(IncrementIndex)

        Call(Function_IncrementIndex);
    }
};