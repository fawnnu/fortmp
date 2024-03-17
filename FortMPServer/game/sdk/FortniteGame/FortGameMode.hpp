#pragma once
#include "../FortGameState.hpp"
#include "../../Engine/World.hpp"
#include "sdk/FortniteGame/Athena/FortCreative.hpp"
#include "sdk/Engine/Kismet/GameplayStatics.hpp"

class AGameSession : public UObject
{
public:
    DEFINE_PROPERTY(int, MaxPlayers);
};

class AFortGameSessionDedicated : public AGameSession
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortGameSessionDedicated")
};

class AFortGameSessionDedicatedAthena : public AFortGameSessionDedicated
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortGameSessionDedicatedAthena")
};

class AGameModeBase : public AActor
{
public:
    DEFINE_PROPERTY(AGameSession*, GameSession);
    DEFINE_PROPERTY(UObject*, GameSessionClass);
    DEFINE_PROPERTY(UObject*, PlayerControllerClass);
    
    class AFortPlayerPawn* SpawnDefaultPawnAtTransform(class APlayerController* NewPlayer, FTransform SpawnTransform)
    {
        STATIC_FINDFUNCTION(SpawnDefaultPawnAtTransform)
        
        return Call<AFortPlayerPawn*>(Function_SpawnDefaultPawnAtTransform, NewPlayer, SpawnTransform);
    }

    AActor* ChoosePlayerStart(APlayerController* NewPlayer)
    {
        STATIC_FINDFUNCTION(ChoosePlayerStart)
        
        return Call<AActor*>(Function_ChoosePlayerStart, NewPlayer);
    }

    void RestartPlayer(class AController* NewPlayer)
    {
        STATIC_FINDFUNCTION(RestartPlayer)
        
        Call(Function_RestartPlayer, NewPlayer);
    }

    void RestartPlayerAtPlayerStart(class AController* NewPlayer, class AActor* StartSpot)
    {
        STATIC_FINDFUNCTION(RestartPlayerAtPlayerStart)
        
        Call(Function_RestartPlayerAtPlayerStart, NewPlayer, StartSpot);
    }

    void RestartPlayerAtTransform(class AController* NewPlayer, FTransform Transform)
    {
        STATIC_FINDFUNCTION(RestartPlayerAtTransform)
        
        Call(Function_RestartPlayerAtTransform, NewPlayer, Transform);
    }
};

class AGameMode : public AGameModeBase
{
public:
    DEFINE_PROPERTY(UObject*, DefaultPawnClass);

    void StartMatch()
    {
        STATIC_FINDFUNCTION(StartMatch)
        
        Call(Function_StartMatch);
    }
};

class AFortGameMode : public AGameMode
{
public:
    DEFINE_PROPERTY(TArray<AFortPlayerController*>, ConnectedPlayerControllers);
    DEFINE_PROPERTY(AGameSession*, FortGameSession);
    DEFINE_PROPERTY(float, AbilityRefireHitchCooldown);
    DEFINE_PROPERTY_BY_OFFSET(float, LastHitch, CoreOffsets::LastHitch);

    static AActor* GetClosestPlayerStart(FVector Location)
    {
        TArray<AActor*> FortPlayerStartArray = UGameplayStatics::GetAllActorsOfClass(*GWorld, AFortPlayerStart::StaticClass());

        float ClosestDist = FLT_MAX;
        AFortPlayerStart* ClosestPlayerStart = nullptr;
        
        for (auto actor : FortPlayerStartArray)
        {
            auto FortPlayerStart = actor->Cast<AFortPlayerStart>();
            if (!FortPlayerStart) continue;

            auto FortPlayerStartLocation = FortPlayerStart->K2_GetActorLocation();
            if (!FortPlayerStartLocation.X && !FortPlayerStartLocation.Y && !FortPlayerStartLocation.Z) continue;

            int Dist = float(sqrtf(powf(FortPlayerStartLocation.X - Location.X, 2.0) + powf(FortPlayerStartLocation.Y - Location.Y, 2.0) + powf(FortPlayerStartLocation.Z - Location.Z, 2.0))) / 100.f;
            if (Dist < ClosestDist)
            {
                ClosestDist = Dist;
                ClosestPlayerStart = FortPlayerStart;
            }
        }
        
        return ClosestPlayerStart;
    }
};

class AFortGameModeZone : public AFortGameMode
{
public:
    DEFINE_PROPERTY(TArray<struct FItemAndCount>, StartingItems);
};

class AFortGameModeAthena : public AFortGameModeZone
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortGameModeAthena")

    DEFINE_PROPERTY(UObject*, LiveSpectatorPlayerControllerClass);
    DEFINE_PROPERTY(UObject*, LiveBroadcastPlayerControllerClass);
    
    DEFINE_PROPERTY(bool, bDisableGCOnServerDuringMatch);
    
    DEFINE_PROPERTY(float, WarmupCountdownDuration);
    DEFINE_PROPERTY(int, WarmupRequiredPlayerCount);
    DEFINE_PROPERTY(float, WarmupEarlyCountdownDuration);
    DEFINE_PROPERTY(float, EndGameKickPlayersDelay);
    
    DEFINE_PROPERTY(int, SafeZonePhase);
    
    DEFINE_PROPERTY(bool, bMapInfoInitialized);
    DEFINE_PROPERTY(class AFortSafeZoneIndicator*, SafeZoneIndicator);
    DEFINE_PROPERTY(TArray<FVector>, SafeZoneLocations);
    
    DEFINE_PROPERTY(class UFortServerBotManagerAthena*, ServerBotManager);

    DEFINE_PROPERTY(TArray<AFortPlayerController*>, AlivePlayers);

    DEFINE_PROPERTY(bool, bSquadPlay);

    bool& bMatchHasStarted()
    {
        static void* native_property = FindPropertyByName(this->Class, L"WarmupRequiredPlayerCount");                                          
        static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);   
        
        return *reinterpret_cast<bool*>(__int64(this) + native_property_offset - 4);                                        
    }

    class AFortGameplayMutator* GetMutatorByClass(AActor* ContextActor, UObject* MutatorClass)
    {
        STATIC_FINDFUNCTION(GetMutatorByClass)

        return Call<AFortGameplayMutator*>(Function_GetMutatorByClass, ContextActor, MutatorClass);
    }

    bool ShouldSpawnVehicle(class AFortAthenaVehicle* VehicleClass)
    {
        STATIC_FINDFUNCTION(ShouldSpawnVehicle)

        return Call<bool>(Function_ShouldSpawnVehicle, VehicleClass);
    }
    // Custom event
    void OnServerReady()
    {
        STATIC_FINDFUNCTION(OnServerReady)
        
        Call(Function_OnServerReady);
    }

    bool IsMapInfoInitialized()
    {
        STATIC_FINDFUNCTION(IsMapInfoInitialized)
        
        return Call<bool>(Function_IsMapInfoInitialized);
    }
    
    void SwapButterflyEventPOIs()
    {
        STATIC_FINDFUNCTION(SwapButterflyEventPOIs)
        
        Call(Function_SwapButterflyEventPOIs);
    }
    
    // Custom event
    void OnServerMapInfoReady()
    {
        STATIC_FINDFUNCTION(OnServerMapInfoReady)
        
        Call(Function_OnServerMapInfoReady);
    }

    void OnPlayerJoined(APlayerController* NewPlayer)
    {
        if (UFunction* TargetFunction = static_cast<UFunction*>(FindFunctionByName(this->Class, L"OnPlayerJoined", 1)))
        {
            struct
            {
                APlayerController* NewPlayer;
            } Params;

            Params.NewPlayer = NewPlayer;

            ProcessEvent(this, TargetFunction, &Params);
        }
    }

    bool& OnUpdateStorm()
    {
        if (UFunction* TargetFunction = static_cast<UFunction*>(FindFunctionByName(this->Class, L"OnUpdateStorm", 1)))
        {
            struct
            {
                bool ReturnValue;
            } Params;

            ProcessEvent(this, TargetFunction, &Params);

            return Params.ReturnValue;
        }
    }
    
    UFortPlaylistAthena*& OnSetPlaylist()
    {
        if (UFunction* TargetFunction = static_cast<UFunction*>(FindFunctionByName(this->Class, L"OnSetPlaylist", 1)))
        {
            struct
            {
                UFortPlaylistAthena* ReturnValue;
            } Params;

            ProcessEvent(this, TargetFunction, &Params);

            return Params.ReturnValue;
        }
    }
    
    void OnEnterAircraft(APlayerController* TargetPlayer)
    {
        if (UFunction* TargetFunction = static_cast<UFunction*>(FindFunctionByName(this->Class, L"OnEnterAircraft", 1)))
        {
            struct
            {
                APlayerController* NewPlayer;
            } Params;

            Params.NewPlayer = TargetPlayer;

            ProcessEvent(this, TargetFunction, &Params);
        }
    }
    
    void OnExitAircraft(APlayerController* TargetPlayer)
    {
        if (UFunction* TargetFunction = static_cast<UFunction*>(FindFunctionByName(this->Class, L"OnExitAircraft", 1)))
        {
            struct
            {
                APlayerController* NewPlayer;
            } Params;

            Params.NewPlayer = TargetPlayer;

            ProcessEvent(this, TargetFunction, &Params);
        }
    }
    
    void OnPawnFinishedSpawning(class AFortPawn* TargetPawn)
    {
        if (UFunction* TargetFunction = static_cast<UFunction*>(FindFunctionByName(this->Class, L"OnPawnFinishedSpawning", 1)))
        {
            struct
            {
                AFortPawn* TargetPawn;
            } Params;

            Params.TargetPawn = TargetPawn;

            ProcessEvent(this, TargetFunction, &Params);
        }
    }
};

class AFortAthenaVehicleSpawner : public AActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaVehicleSpawner")

    DEFINE_PROPERTY(TSoftObjectPtr<class UFortVehicleItemDefinition>, FortVehicleItemDef);
    DEFINE_PROPERTY(AFortAthenaVehicle*, SpawnedVehicle);
    
    void SpawnVehicle()
    {
        STATIC_FINDFUNCTION(SpawnVehicle)

        Call(Function_SpawnVehicle);
    }

    UObject* GetVehicleClass()
    {
        STATIC_FINDFUNCTION(GetVehicleClass)

        return Call<UObject*>(Function_GetVehicleClass);
    }
};