#pragma once
#include "sdk/FortniteGame/Athena/FortCreative.hpp"

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
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortClientSettingsRecord");

    DEFINE_PROPERTY(FString, LastKnownBestRegionId);
    DEFINE_PROPERTY(FString, SelectedRegionId);
};

class UCheatManager : public UObject
{
public:
    DEFINE_STATIC_CLASS(L"/Script/Engine.CheatManager");
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
        STATIC_FINDFUNCTION(Possess)
        
        Call(Function_Possess, InPawn);
    }

    void SetControlRotation(FRotator in_rotation)
    {
        STATIC_FINDFUNCTION(SetControlRotation)
        
        Call(Function_SetControlRotation, in_rotation);
    }

    AActor* GetViewTarget()
    {
        STATIC_FINDFUNCTION(GetViewTarget)
        
        return Call<AActor*>(Function_GetViewTarget);
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
    DEFINE_PROPERTY(UCheatManager*, CheatManager);
    
    void SwitchLevel(FString URL)
    {
        STATIC_FINDFUNCTION(SwitchLevel)
        
        Call(Function_SwitchLevel, URL);
    }

    void ClientGotoState(FName NewState)
    {
        STATIC_FINDFUNCTION(ClientGotoState)
        
        Call(Function_ClientGotoState, NewState);
    }

    void ClientSetViewTarget(AActor* Target)
    {
        STATIC_FINDFUNCTION(ClientSetViewTarget)
        
        Call(Function_ClientSetViewTarget, Target, FViewTargetTransitionParams{});
    }

    void ClientMessage(FString S)
    {
        STATIC_FINDFUNCTION(ClientMessage)

        struct
        {
            FString S;
            FName Type;
            float MsgLifeTime;
        } params;

        params.S = S;
        params.Type = FName(TEXT("None"));
        params.MsgLifeTime = 0.f;

        ProcessEvent(this, Function_ClientMessage, &params);
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
    DEFINE_PROPERTY(TArray<struct FFortUpdatedObjectiveStat>, UpdatedObjectiveStats);
    DEFINE_PROPERTY(AFortPlayerPawn*, MyFortPawn);
    DEFINE_PROPERTY(UFortPlayerControllerAthenaXPComponent*, XPComponent);
    DEFINE_PROPERTY(bool, bHasServerFinishedLoading);
    DEFINE_PROPERTY(class AFortQuickBars*, QuickBars);
    DEFINE_PROPERTY(class AFortQuickBars*, ClientQuickBars);
    DEFINE_PROPERTY(AFortInventory*, WorldInventory);
    DEFINE_PROPERTY(APawn*, PlayerToSpectateOnDeath);

    bool IsInAircraft()
    {
        STATIC_FINDFUNCTION(IsInAircraft)
        
        return Call<bool>(Function_IsInAircraft);
    }

    void ClientConfirmTargetData(uint16 UniqueId, bool bSuccess, TArray<uint8> HitReplaces)
    {
        STATIC_FINDFUNCTION(ClientConfirmTargetData)

        Call(Function_ClientConfirmTargetData, UniqueId, bSuccess, HitReplaces);
    }
    
    void ServerExecuteInventoryItem(FGuid ItemGuid)
    {
        STATIC_FINDFUNCTION(ServerExecuteInventoryItem)
        
        Call(Function_ServerExecuteInventoryItem, ItemGuid);
    }

    void ServerAttemptAircraftJump(FRotator ClientRotation)
    {
        STATIC_FINDFUNCTION(ServerAttemptAircraftJump)
        
        Call(Function_ServerAttemptAircraftJump, ClientRotation);
    }

    void ClientEquipItem(FGuid ItemGuid, bool bForceExecution)
    {
        STATIC_FINDFUNCTION(ClientEquipItem)

        if (Function_ClientEquipItem)
        {
            static bool bForceExecution_Exists = FindPropertyByName(Function_ClientEquipItem, TEXT("bForceExecution")) != nullptr;

            if (bForceExecution_Exists)
            {
                Call(Function_ClientEquipItem, ItemGuid, bForceExecution);
            }
            else
            {
                Call(Function_ClientEquipItem, ItemGuid);
            }
        }
    }

    void ClientReturnToMainMenu(FString in_reason)
    {
        STATIC_FINDFUNCTION(ClientReturnToMainMenu)
        
        Call(Function_ClientReturnToMainMenu, in_reason);
    }

    UFortItem* BP_GetInventoryItemWithGuid(FGuid ItemGuid)
    {
        STATIC_FINDFUNCTION(BP_GetInventoryItemWithGuid)
        
        return Call<UFortItem*>(Function_BP_GetInventoryItemWithGuid, ItemGuid);
    }

    UFortItem* K2_GetInventoryItemWithGuid(FGuid ItemGuid)
    {
        STATIC_FINDFUNCTION(K2_GetInventoryItemWithGuid)
        
        return Call<UFortItem*>(Function_K2_GetInventoryItemWithGuid, ItemGuid);
    }

    UFortItem* BP_FindExistingItemForDefinition(UFortItemDefinition* ItemDefinition, bool bInStorageVault = false)
    {
        STATIC_FINDFUNCTION(BP_FindExistingItemForDefinition)
        
        return Call<UFortItem*>(Function_BP_FindExistingItemForDefinition, ItemDefinition, bInStorageVault);
    }

    UFortItem* K2_FindExistingItemForDefinition(UFortItemDefinition* ItemDefinition, bool bInStorageVault = false)
    {
        STATIC_FINDFUNCTION(K2_FindExistingItemForDefinition)
        
        return Call<UFortItem*>(Function_K2_FindExistingItemForDefinition, ItemDefinition, bInStorageVault);
    }

    void ClientReportDamagedResourceBuilding(class ABuildingSMActor* BuildingSMActor, uint8_t PotentialResourceType, int PotentialResourceCount, bool bDestroyed, bool bJustHitWeakpoint)
    {
        STATIC_FINDFUNCTION(ClientReportDamagedResourceBuilding)
        
        Call(Function_ClientReportDamagedResourceBuilding, BuildingSMActor, PotentialResourceType, PotentialResourceCount, bDestroyed, bJustHitWeakpoint);
    }

    void ServerSuicide()
    {
        STATIC_FINDFUNCTION(ServerSuicide)
        
        Call(Function_ServerSuicide);
    }
};

struct FAthenaQuickChatActiveEntry
{
    DEFINE_STRUCT_PROPERTY(uint8_t, Index);
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.AthenaQuickChatActiveEntry");
};

class AFortPlayerControllerAthena : public AFortPlayerController
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortPlayerControllerAthena");

    FFortAthenaLoadout& CustomizationLoadout()
    {
        static void* native_property = FindPropertyByName(this->Class, TEXT("CustomizationLoadout")) ? FindPropertyByName(this->Class, TEXT("CustomizationLoadout")) : FindPropertyByName(this->Class, TEXT("CosmeticLoadoutPC"));                                          
        static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);   
        
        return *reinterpret_cast<FFortAthenaLoadout*>(__int64(this) + native_property_offset);                
    }
    
    DEFINE_PROPERTY(TScriptMulticastDelegate<AFortPlayerController>, OnStartedRespawn);

    DEFINE_PROPERTY(AFortAthenaCreativePortal*, OwnedPortal);
    DEFINE_PROPERTY(AFortVolume*, CreativePlotLinkedVolume);
    DEFINE_PROPERTY(UAthenaMarkerComponent*, MarkerComponent);
    DEFINE_PROPERTY(UAthenaResurrectionComponent*, ResurrectionComponent);

    void ClientReceiveSquadQuickChatMessage(FAthenaQuickChatActiveEntry& ChatEntry, AFortPlayerController* SenderPC)
    {
        STATIC_FINDFUNCTION(ClientReceiveSquadQuickChatMessage)

        // GET PARAMS SIZE
        int16_t parms_size = 0;
        std::vector<int> PropertyOffsets; // Index 0 = ChatEntry, Index 1 = SenderPC
            
        for (UObject* Property = *reinterpret_cast<UObject**>(__int64(Function_ClientReceiveSquadQuickChatMessage) + CoreOffsets::Children); Property; Property = *reinterpret_cast<UObject**>(__int64(Property) + CoreOffsets::Next))
        {
            int offset = *reinterpret_cast<int*>(__int64(Property) + CoreOffsets::Offset_Internal);

            PropertyOffsets.push_back(offset);
            
            parms_size = offset + // Offset_Internal
                *reinterpret_cast<int*>(__int64(Property) + CoreOffsets::ElementSize); // ElementSize
        }

        // Allocate params
        PVOID Params = alloca(parms_size);
        ZeroMemory(Params, parms_size);

        // memcpy ChatEntry
        memcpy(reinterpret_cast<FAthenaQuickChatActiveEntry*>(reinterpret_cast<uintptr_t>(Params) + PropertyOffsets[0]), &ChatEntry, FAthenaQuickChatActiveEntry::GetStructSize());

        // Set SenderPC
        *reinterpret_cast<AFortPlayerController**>(reinterpret_cast<uintptr_t>(Params) + PropertyOffsets[1]) = SenderPC;

        // ProcessEvent
        ProcessEvent(this, Function_ClientReceiveSquadQuickChatMessage, Params);
    }
    
    void OnRep_CreativePlotLinkedVolume()
    {
        STATIC_FINDFUNCTION(OnRep_CreativePlotLinkedVolume)
        
        Call(Function_OnRep_CreativePlotLinkedVolume);
    }

    void SpectateOnDeath()
    {
        STATIC_FINDFUNCTION(SpectateOnDeath)
        
        Call(Function_SpectateOnDeath);
    }

    void PreLobbyIslandTeleport()
    {
        STATIC_FINDFUNCTION(PreLobbyIslandTeleport)
        
        Call(Function_PreLobbyIslandTeleport);
    }

    void PostLobbyIslandTeleport(AFortPlayerPawn* FortPlayerPawn)
    {
        STATIC_FINDFUNCTION(PostLobbyIslandTeleport)
        
        Call(Function_PostLobbyIslandTeleport, FortPlayerPawn);
    }
};

struct FFortPlayerDeathReport
{
    DEFINE_STRUCT_PROPERTY(AFortPlayerStateAthena*, KillerPlayerState);
    DEFINE_STRUCT_PROPERTY(AFortPawn*, KillerPawn);
    DEFINE_STRUCT_PROPERTY(AActor*, DamageCauser);
    DEFINE_STRUCT_PROPERTY(FGameplayTagContainer, Tags);
    DEFINE_STRUCT_PROPERTY(float, LethalDamage);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.FortPlayerDeathReport");
};

class UAthenaResurrectionComponent : public UObject
{
public:
    DEFINE_PROPERTY(TArray<FUniqueNetIdRepl>, PlayerIdsForResurrection);
};

class AAthena_PlayerController_C : public AFortPlayerControllerAthena
{
public:
    DEFINE_STATIC_CLASS(L"/Game/Athena/Athena_PlayerController.Athena_PlayerController_C");
};

class AFortAthenaAIBotController : public AController
{
public:
    DEFINE_PROPERTY(FFortAthenaLoadout, CosmeticLoadoutBC);
    DEFINE_PROPERTY(class UFortAthenaAIBotInventoryItems*, StartupInventory);
    DEFINE_PROPERTY(class UFortAthenaAIBotCustomizationData*, BotData);
    DEFINE_PROPERTY(AFortInventory*, Inventory);
    DEFINE_PROPERTY(class AFortGameModeAthena*, CachedGameMode);
    DEFINE_PROPERTY(class UFortServerBotManagerAthena*, CachedBotManager);
    DEFINE_PROPERTY(class AFortPlayerPawnAthena*, PlayerBotPawn);
    DEFINE_PROPERTY(class AFortAthenaMutator_Bots*, CachedBotMutator);
};