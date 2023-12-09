#pragma once

class ABP_Athena_Event_Components_C : public AActor
{
public:
    DEFINE_CLASS(L"/Game/Athena/Events/BP_Athena_Event_Components.BP_Athena_Event_Components_C");

    DEFINE_PROPERTY(float, CrackProgression);
    DEFINE_PROPERTY(int, AreCracksVisible);
    DEFINE_PROPERTY(int, AreEndSequenceCrackVisible);
};

class Adomino_scripting_C : public AActor
{
public:

    DEFINE_CLASS(L"/Game/fortmp/content_complex/domino/blueprints/domino_scripting.domino_scripting_C")
    
    void StartEvent()
    {
        STATIC_FINDFUNCTION(StartEvent)

        Call(Function_StartEvent);
    }
};

class ABP_SnowSetup_C : public AActor
{
public:

    DEFINE_CLASS(L"/Game/Athena/Environments/Landscape/Blueprints/BP_SnowSetup.BP_SnowSetup_C");

    DEFINE_PROPERTY(float, Zone);
    
    void SetSnow(float NewParam)
    {
        STATIC_FINDFUNCTION(SetSnow)

        Call(Function_SetSnow, NewParam);
    }

    void SetFullSnowEd()
    {
        STATIC_FINDFUNCTION(SetSnow)

        Call(Function_SetSnow);
    }

    void OnRep_Snow_Amount()
    {
        STATIC_FINDFUNCTION(OnRep_Snow_Amount)

        Call(Function_OnRep_Snow_Amount);
    }
    
    static ABP_SnowSetup_C* get_snow_setup()
    {
        TArray<AActor*> island_scripter = UGameplayStatics::GetAllActorsOfClass(*GWorld, StaticClass());
                                
        for (auto actor : island_scripter)
        {
            return static_cast<ABP_SnowSetup_C*>(actor);
        }
        return nullptr;
    }
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

class ACUBE_Simple_C : public AActor
{
public:
    DEFINE_PROPERTY(float, CrackProgress);
    DEFINE_PROPERTY(float, DrainAmount); // Default = 0. Goes from 0.0 to 1.0f;
    DEFINE_PROPERTY(float, SnappedDrainAmount);
    DEFINE_PROPERTY(bool, DrainActive);

    DEFINE_PROPERTY(float, StartingDrainAmount);

    void BigDrip(bool Is_last_drip)
    {
        STATIC_FINDFUNCTION(BigDrip)

        Call(Function_BigDrip, Is_last_drip);
    }
    
    void OnRep_SpinStarted(bool a1, bool a2, bool a3)
    {
        STATIC_FINDFUNCTION(OnRep_CubeletGrowthstage)

        Call(Function_OnRep_CubeletGrowthstage, a1, a2, a3);
    }
    void OnRep_DrainAmount(float a1, int32 a1b, float a2, float a3, float a4, float a5)
    {
        STATIC_FINDFUNCTION(OnRep_CubeletGrowthstage)

        Call(Function_OnRep_CubeletGrowthstage, a1, a1b, a2, a3, a4, a5);
    }
    void OnRep_DrainActive(bool a1, class UObject* a2, bool a3, bool a4)
    {
        STATIC_FINDFUNCTION(OnRep_CubeletGrowthstage)

        Call(Function_OnRep_CubeletGrowthstage, a1, a2, a3, a4);
    }
    void OnRep_CrackProgress(bool a1, class UObject* a2)
    {
        STATIC_FINDFUNCTION(OnRep_CubeletGrowthstage)

        Call(Function_OnRep_CubeletGrowthstage, a1, a2);
    }
};
class ABP_IslandScripting_C : public AActor
{
public:
    // Default Island Position
    DEFINE_PROPERTY(FVector, IslandPosition);
    DEFINE_PROPERTY(bool, IsDeimosActive);
    DEFINE_PROPERTY(bool, UpdateMap);
    DEFINE_PROPERTY(bool, HasCubeTriggered);
    DEFINE_PROPERTY(float, CrackProgress);
    DEFINE_PROPERTY(int, MapIconState);
    DEFINE_PROPERTY(bool, DisableVortexGust);
    DEFINE_PROPERTY(int, LevelActorsDeletedInt);
    DEFINE_PROPERTY(ACUBE_Simple_C*, CubeBP);
    
    DEFINE_CLASS(L"/Game/Athena/Prototype/Blueprints/Island/BP_IslandScripting.BP_IslandScripting_C")

    void ButterflyStart()
    {
        STATIC_FINDFUNCTION(ButterflyStart)

        Call(Function_ButterflyStart);
    }
    
    void CubeEvent()
    {
        STATIC_FINDFUNCTION(CubeEvent)

        Call(Function_CubeEvent);
    }
    
    void OnRep_CubeletGrowthstage()
    {
        STATIC_FINDFUNCTION(OnRep_CubeletGrowthstage)

        Call(Function_OnRep_CubeletGrowthstage);
    }
    
    void OnRep_LevelActorsDeletedInt()
    {
        STATIC_FINDFUNCTION(OnRep_LevelActorsDeletedInt)

        Call(Function_OnRep_LevelActorsDeletedInt);
    }
    
    void OnRep_DisableVortexGust()
    {
        STATIC_FINDFUNCTION(OnRep_DisableVortexGust)

        Call(Function_OnRep_DisableVortexGust);
    }
    
    void OnRep_MapIconState()
    {
        STATIC_FINDFUNCTION(OnRep_MapIconState)

        Call(Function_OnRep_MapIconState);
    }
    
    void CubecrackLeadup()
    {
        STATIC_FINDFUNCTION(CubecrackLeadup)

        Call(Function_CubecrackLeadup);
    }
    
    void LoadDynamicLevels()
    {
        STATIC_FINDFUNCTION(LoadDynamicLevels)

        Call(Function_LoadDynamicLevels);
    }

    void TrySetIslandLocation()
    {
        STATIC_FINDFUNCTION(TrySetIslandLocation)

        Call(Function_TrySetIslandLocation);
    }

    void OnRep_UpdateMap()
    {
        STATIC_FINDFUNCTION(OnRep_UpdateMap)

        Call(Function_OnRep_UpdateMap, true);
    }

    void HideAndDestroyLevelActors()
    {
        STATIC_FINDFUNCTION(HideAndDestroyLevelActors)

        Call(Function_HideAndDestroyLevelActors, true);
    }
    
    static ABP_IslandScripting_C* get_island_scripting()
    {
        TArray<AActor*> island_scripter = UGameplayStatics::GetAllActorsOfClass(*GWorld, StaticClass());
                                
        for (auto actor : island_scripter)
        {
            return static_cast<ABP_IslandScripting_C*>(actor);
        }
        return nullptr;
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
    
    void SetupLake()
    {
        STATIC_FINDFUNCTION(SetupLake)

        Call(Function_SetupLake);
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