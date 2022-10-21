#pragma once
#include "FortCreative.hpp"

class UNetConnection;
class UAthenaResurrectionComponent;

struct FViewTargetTransitionParams
{
    uint8_t data[0x10];
};

class UFortRegisteredPlayerInfo : public UObject
{
public:
    DEFINE_PROPERTY(FString, PlayerName);
};

class UFortClientSettingsRecord : public UObject
{
public:
};

class ULocalPlayer : public UPlayer
{
public:
};

class UFortLocalPlayer : public ULocalPlayer
{
public:
    DEFINE_PROPERTY(UFortClientSettingsRecord*, ClientSettingsRecord);
};

class AController : public AActor
{
public:
    DEFINE_PROPERTY(UNetConnection*, NetConnection);
    
    void Possess(AFortPlayerPawn* InPawn)
    {
        Call(L"Possess", InPawn);
    }

    void SetControlRotation(FRotator in_rotation)
    {
        Call(L"SetControlRotation", in_rotation);
    }

    AActor* GetViewTarget()
    {
        return Call<AActor*>(L"GetViewTarget");
    }
};

class APlayerCameraManager : public AActor
{
public:
    DEFINE_PROPERTY(float, ViewRollMin);
    DEFINE_PROPERTY(float, ViewRollMax);
};

class APlayerController : public AController
{
public:
    DEFINE_PROPERTY(UObject*, AcknowledgedPawn);
    DEFINE_PROPERTY(AFortPlayerStateAthena*, PlayerState);
    DEFINE_PROPERTY(APlayerCameraManager*, PlayerCameraManager);

    void SwitchLevel(FString URL)
    {
        Call(L"SwitchLevel", URL);
    }

    void ClientGotoState(FName NewState)
    {
        Call(L"ClientGotoState", NewState);
    }

    void ClientSetViewTarget(AActor* Target)
    {
        Call(L"ClientSetViewTarget", Target, FViewTargetTransitionParams{});
    }
};

class UFortPlayerControllerAthenaXPComponent : public UObject
{
public:
    DEFINE_PROPERTY(bool, bRegisteredWithQuestManager);
};

class AFortInventory;

class AFortPlayerController : public APlayerController
{
public:
    DEFINE_BIT_PROPERTY(bool, bIsDisconnecting);
    DEFINE_PROPERTY(AFortPlayerPawn*, Pawn);
    DEFINE_PROPERTY(UFortRegisteredPlayerInfo*, MyPlayerInfo);

    DEFINE_PROPERTY(UFortPlayerControllerAthenaXPComponent*, XPComponent);
    
    DEFINE_PROPERTY(bool, bHasServerFinishedLoading);

    DEFINE_PROPERTY(class AFortQuickBars*, QuickBars);
    DEFINE_PROPERTY(class AFortQuickBars*, ClientQuickBars);

    DEFINE_PROPERTY(AFortInventory*, WorldInventory);

    DEFINE_PROPERTY(APawn*, PlayerToSpectateOnDeath);

    bool IsInAircraft()
    {
        return Call<bool>(L"IsInAircraft");
    }

    void ServerExecuteInventoryItem(FGuid ItemGuid)
    {
        Call(L"ServerExecuteInventoryItem", ItemGuid);
    }

    void ServerAttemptAircraftJump(FRotator ClientRotation)
    {
        Call(L"ServerAttemptAircraftJump", ClientRotation);
    }

    void ClientEquipItem(FGuid ItemGuid, bool bForceExecution)
    {
        Call(L"ClientEquipItem", ItemGuid, bForceExecution);
    }

    void ClientReturnToMainMenu(FString in_reason)
    {
        Call(L"ClientReturnToMainMenu", in_reason);
    }

    UFortItem* BP_GetInventoryItemWithGuid(FGuid ItemGuid)
    {
        return Call<UFortItem*>(L"BP_GetInventoryItemWithGuid", ItemGuid);
    }

    UFortItem* K2_GetInventoryItemWithGuid(FGuid ItemGuid)
    {
        return Call<UFortItem*>(L"K2_GetInventoryItemWithGuid", ItemGuid);
    }

    UFortItem* BP_FindExistingItemForDefinition(UFortItemDefinition* ItemDefinition, bool bInStorageVault = false)
    {
        return Call<UFortItem*>(L"BP_FindExistingItemForDefinition", ItemDefinition, bInStorageVault);
    }

    UFortItem* K2_FindExistingItemForDefinition(UFortItemDefinition* ItemDefinition, bool bInStorageVault = false)
    {
        return Call<UFortItem*>(L"K2_FindExistingItemForDefinition", ItemDefinition, bInStorageVault);
    }

    void ClientReportDamagedResourceBuilding(class ABuildingSMActor* BuildingSMActor, uint8_t PotentialResourceType, int PotentialResourceCount, bool bDestroyed, bool bJustHitWeakpoint)
    {
        Call(L"ClientReportDamagedResourceBuilding", BuildingSMActor, PotentialResourceType, PotentialResourceCount, bDestroyed, bJustHitWeakpoint);
    }
};

class AFortPlayerControllerAthena : public AFortPlayerController
{
public:
    DEFINE_PROPERTY(TScriptMulticastDelegate<AFortPlayerController>, OnStartedRespawn);

    DEFINE_PROPERTY(AFortAthenaCreativePortal*, OwnedPortal);
    DEFINE_PROPERTY(AFortVolume*, CreativePlotLinkedVolume);
    
    DEFINE_PROPERTY(UAthenaResurrectionComponent*, ResurrectionComponent);

    void SpectateOnDeath()
    {
        Call(L"SpectateOnDeath");
    }

    void PreLobbyIslandTeleport()
    {
        Call(L"PreLobbyIslandTeleport");
    }

    void PostLobbyIslandTeleport(AFortPlayerPawn* FortPlayerPawn)
    {
        Call(L"PostLobbyIslandTeleport", FortPlayerPawn);
    }
};

struct FFortPlayerDeathReport
{
    DEFINE_STRUCT_PROPERTY(AFortPlayerStateAthena*, KillerPlayerState);
    DEFINE_STRUCT_PROPERTY(AFortPawn*, KillerPawn);
    DEFINE_STRUCT_PROPERTY(AActor*, DamageCauser);
    DEFINE_STRUCT_PROPERTY(FGameplayTagContainer, Tags);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortPlayerDeathReport");
};

class UAthenaResurrectionComponent : public UObject
{
public:
    DEFINE_PROPERTY(TArray<FUniqueNetIdRepl>, PlayerIdsForResurrection);
};