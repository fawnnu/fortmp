#include "include.hpp"
#include "fort_mutators_component.hpp"

#include "function_component.hpp"
#include "inventory_component.hpp"

fort_mutators_component::fort_mutators_component()
{
    AFortPlayerController* Default__FortPlayerController = FindObject<AFortPlayerController>(L"/Script/FortniteGame.Default__FortPlayerController");

    if (const uintptr_t BeginPlayAddress = memory_manager::find_function_by_string(L"MusicManager class default object is null as FortPlayerController::BeginPlay is called"))
    {
        int BeginPlayIndex = 0;
    
        for (int i = 0; i < INT_MAX; i++)
        {
            if (Default__FortPlayerController->VTable[i] == reinterpret_cast<LPVOID>(BeginPlayAddress))
            {
                BeginPlayIndex = i;
            
                break;
            }
        }

    	if (AFortAthenaMutator_Barrier::StaticClass())
    	{
    		// Hook begin play for athena mutators
    		REGISTER_VTABLE_HOOK(CreateDefaultObject<AFortAthenaMutator_Barrier>()->VTable, BeginPlayIndex, fort_mutators_component_hooks::AFortAthenaMutator_Barrier_BeginPlay);	
    	}
    	
    	// Getaway mode re-implementation
        if (const AFortAthenaExitCraftSpawner* DefaultObject = CreateDefaultObject<AFortAthenaExitCraftSpawner>())
        {
            REGISTER_VTABLE_HOOK(DefaultObject->VTable, BeginPlayIndex, fort_mutators_component_hooks::AFortAthenaExitCraftSpawner_BeginPlay);

            static UFunction* FunctionObject = FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaExitCraftSpawner.StartExitCraftSpawnTimer");

            if(FunctionObject)
            {
                auto start = memory_manager::scan_for(reinterpret_cast<uintptr_t>(FunctionObject->get_native_function()), {0xE9}, 0, false);
                
                auto result = (((uintptr_t)start + 5) + *(int32_t*)((uintptr_t)start + 1));
                
                auto stripped_leas = memory_manager::get_all_stripped_leas_in_function(result);

                for (int i = 0; i < stripped_leas.size(); ++i)
                {
                    uintptr_t func = stripped_leas[i];
                    
                    memory_manager::lea_hook(func, reinterpret_cast<uintptr_t>(ExitCraftOnTimerFinished));
                }
            }
        }
    }
    else
    {
    	spdlog::error(L"Failed to hook fort_mutators_component BeginPlay. Invalid BeginPlayAddress");
    }
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
	REGISTER_UFUNCTION_DETOUR(fort_mutators_component_hooks::StartItemAwardTimers, L"/Script/FortniteGame.FortGameplayAbilityAthena_PeriodicItemGrant.StartItemAwardTimers", fort_mutators_component_hooks::StartItemAwardTimers_Hook);
    DetourTransactionCommit();

	if (UFortGameplayAbilityAthena_PeriodicItemGrant::StaticClass())
	{
		REGISTER_CUSTOM_UFUNCTION(PeriodicWoodGrant, UFortGameplayAbilityAthena_PeriodicItemGrant::StaticClass())
		REGISTER_CUSTOM_UFUNCTION(PeriodicMetalGrant, UFortGameplayAbilityAthena_PeriodicItemGrant::StaticClass())
		REGISTER_CUSTOM_UFUNCTION(PeriodicStoneGrant, UFortGameplayAbilityAthena_PeriodicItemGrant::StaticClass())
	}
}

void fort_mutators_component::PeriodicWoodGrant(UFortGameplayAbilityAthena_PeriodicItemGrant* Ability,
 	FFrame& StackFrame)
 {
	static UFortItemDefinition* WoodItemData = FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
 	auto player_pawn = Ability->GetActivatingPawn();
 
 	if (player_pawn)
 	{
 		if (auto controller = player_pawn->Controller())
 		{
 			if (auto inventory_item = inventory_component::get_inventory_item_by_definition(controller, WoodItemData))
 			{
 				if (inventory_item->Cast<UFortWorldItem>()->ItemEntry().Count() >= WoodItemData->GetMaxStackSize())
 				{
 					return;
 				}
 			}
 			inventory_component::add_item(controller, WoodItemData, 1, 0, {}, false);
 		}
 	}
 }

void fort_mutators_component::PeriodicStoneGrant(UFortGameplayAbilityAthena_PeriodicItemGrant* Ability,
	FFrame& StackFrame)
{
	static UFortItemDefinition* StoneItemData = FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
	auto player_pawn = Ability->GetActivatingPawn();
 
	if (player_pawn)
	{
		if (auto controller = player_pawn->Controller())
		{
			if (auto inventory_item = inventory_component::get_inventory_item_by_definition(controller, StoneItemData))
			{
				if (inventory_item->Cast<UFortWorldItem>()->ItemEntry().Count() >= StoneItemData->GetMaxStackSize())
				{
					return;
				}
			}
			inventory_component::add_item(controller, StoneItemData, 1, 0, {}, false);
		}
	}
}

void fort_mutators_component::PeriodicMetalGrant(UFortGameplayAbilityAthena_PeriodicItemGrant* Ability,
	FFrame& StackFrame)
{
	static UFortItemDefinition* MetalItemData = FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/MetalItemData.MetalItemData");
	auto player_pawn = Ability->GetActivatingPawn();
 
	if (player_pawn)
	{
		if (auto controller = player_pawn->Controller())
		{
			if (auto inventory_item = inventory_component::get_inventory_item_by_definition(controller, MetalItemData))
			{
				if (inventory_item->Cast<UFortWorldItem>()->ItemEntry().Count() >= MetalItemData->GetMaxStackSize())
				{
					return;
				}
			}
			inventory_component::add_item(controller, MetalItemData, 1, 0, {}, false);
		}
	}
}

void fort_mutators_component_hooks::StartItemAwardTimers_Hook(UFortGameplayAbilityAthena_PeriodicItemGrant* Ability, FFrame& StackFrame)
{
	StartItemAwardTimers(Ability, StackFrame);

	/*static void* native_property = FindPropertyByName(Ability->Class, L"ItemsToGrant");
	static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);
	auto ItemsToGrant = *reinterpret_cast<UE::TMap<FActiveItemGrantInfo, FScalableFloat*>*>(__int64(Ability) + native_property_offset);

	for (auto item_to_grant : ItemsToGrant)
	{
		if (item_to_grant.Key().Item())
		{
			UKismetSystemLibrary::K2_SetTimer(Ability, L"PeriodicItemGrant", item_to_grant.Value()->GetValueAtLevel(0, nullptr), true);
		}
	}*/

	// Temporary solution
	if (fort_mutators_component::get_mutator_of_class(AFortAthenaMutator_Fill::StaticClass()))
	{
		UKismetSystemLibrary::K2_SetTimer(Ability, L"PeriodicWoodGrant", 1.0f, true);
		UKismetSystemLibrary::K2_SetTimer(Ability, L"PeriodicStoneGrant", 1.5f, true);
		UKismetSystemLibrary::K2_SetTimer(Ability, L"PeriodicMetalGrant", 2.0f, true);	
	}
}

// Food Fight Mode hooks
void fort_mutators_component_hooks::AFortAthenaMutator_Barrier_BeginPlay_Hook(AFortAthenaMutator_Barrier* Target)
{
    // Spawn the barrier big base wall
    FActorSpawnParameters spawn_parameters = {};
	
    FVector spawn_location = FVector((*GWorld)->GameState()->FlightPathSelectionCenter().X, (*GWorld)->GameState()->FlightPathSelectionCenter().Y, -5000);
    FRotator spawn_rotation = (*GWorld)->GameState()->FlightPathMidLine().FlightStartRotation();
	
    Target->BigBaseWall() = static_cast<AAthenaBigBaseWall*>(SpawnActor(*GWorld, Target->BigBaseWallClass(), &spawn_location, &spawn_rotation, spawn_parameters));
	
    AFortAthenaMutator_Barrier_BeginPlay(Target);
}

// Getaway mode hooks
void fort_mutators_component_hooks::AFortAthenaExitCraftSpawner_BeginPlay_Hook(AFortAthenaExitCraftSpawner* Target)
{
    AFortAthenaExitCraftSpawner_BeginPlay(Target);

    if (Target)
    {
        // Start the timer to spawn ExitCrafts
        Target->StartExitCraftSpawnTimer();
        
        spdlog::info(L"ExitCraft {} has just started a spawn timer.", Target->GetName().c_str());
    }
}

void fort_mutators_component::ExitCraftOnTimerFinished(AFortAthenaExitCraftSpawner* ExitCraftSpawner, FFrame& StackFrame)
{
    if (ExitCraftSpawner)
    {
        spdlog::info(L"Exit Craft Spawner timer finished, spawning!");
        
        AFortGameModeAthena* game_mode = (*GWorld)->AuthorityGameMode();
        AFortGameStateAthena* game_state = (*GWorld)->GameState();
        
	    if (!game_mode) return;
	    if (!game_state) return;

        AFortAthenaMutator_Heist* heist_mutator = nullptr;

        TArray<AActor*> out_actors = UGameplayStatics::GetAllActorsOfClass(*GWorld, AFortAthenaMutator_Heist::StaticClass());

        for (AActor* out_actor : out_actors)
        {
            heist_mutator = static_cast<AFortAthenaMutator_Heist*>(out_actor);
        }

        if (!heist_mutator)
        {
            spdlog::error(L"Couldn't find Heist mutator!");
            return;
        }

        FHeistExitCraftData* CraftData = heist_mutator->FindCraftData(ExitCraftSpawner);
        if (!CraftData)
        {
            spdlog::error(L"CraftData was not found!");
            return;
        }
  
        heist_mutator->NumSpawnedExitCrafts()++;
        heist_mutator->NumUnspawnedExitCrafts()--;
  
        UFortAthenaExitCraftInfo* spawn_info = ExitCraftSpawner->ExitCraftInfo();
  
        UObject* van_class = ExitCraftSpawner->ExitCraftInfo()->ExitCaftClass();
  
        FVector location = ExitCraftSpawner->K2_GetActorLocation();
  
        FVector start_location = location;
        start_location.Z += spawn_info->ExitCraftInfo().ExitCraftZOffset().GetValueAtLevel(0, nullptr);
	  
        FRotator rotation = ExitCraftSpawner->K2_GetActorRotation();
        
        FActorSpawnParameters spawn_parameters = {};

        AFortAthenaExitCraft* van = static_cast<AFortAthenaExitCraft*>(SpawnActor(*GWorld, van_class, &start_location, &rotation, spawn_parameters));
        if (!van) return;

        van->ExitCraftInfo() = spawn_info;

        van->CurrentState() = 1;
        //van->OnNewState(1);

        van->CraftSpecialActorID() = ExitCraftSpawner->SpawnerSpecialActorID();
  
        CraftData->SpawnedExitCraft() = van;
        CraftData->ExitCraftState() = 2;
        CraftData->SpawnTime() = (*GWorld)->TimeSeconds();
        CraftData->bIsUsed() = false;
        CraftData->bHasDeparted() = false;
  
        heist_mutator->OnExitCraftSpawned(van, ExitCraftSpawner);
  
        FVector TargetLocation = location;
        TargetLocation.Z += spawn_info->ExitCraftInfo().ExitCraftTargetZOffset().GetValueAtLevel(0, nullptr);
  
        van->LocationLerpData().PositionLerp_Start() = start_location;
        van->LocationLerpData().PositionLerp_End() = TargetLocation;
        van->LocationLerpData().PositionLerp_Target() = TargetLocation;
        van->LocationLerpData().TotalLerpTime() = spawn_info->ExitCraftInfo().ExitCraftTimeToHoverLocation().GetValueAtLevel(0, nullptr);
  
        van->OnRep_LocationLerpData();
        
        auto SpecialActorData = game_state->SpecialActorData();
        FSpecialActorRepDataArray* actorRepList = &SpecialActorData->SpecialActorRepList();
        auto SpecialActorList = TArrayManager<FSpecialActorRepData>(&actorRepList->SpecialActorList(), FSpecialActorRepData::GetStructSize());

	    int SpawnerIndex = -1;
	    FSpecialActorRepData* SpawnerRepData = nullptr;

        for (int i = 0; i < SpecialActorList.Num(); ++i)
        {
            FSpecialActorRepData* RepData = &SpecialActorList[i];
            if (!RepData) continue;
    
            if (RepData->SpecialActor() != ExitCraftSpawner) continue;
    
            SpawnerIndex = i;
            SpawnerRepData = RepData;
            break;
        }
        
	    FSpecialActorRepData* VanRepData = (FSpecialActorRepData*)FMemory_Realloc(0,FSpecialActorRepData::GetStructSize(),0);
	    memcpy(VanRepData,SpawnerRepData,FSpecialActorRepData::GetStructSize());
    
	    VanRepData->MostRecentArrayReplicationKey = -1;
	    VanRepData->ReplicationID = -1;
	    VanRepData->ReplicationKey = -1;
    
	    VanRepData->SpecialActorID() = van->CraftSpecialActorID();
	    VanRepData->CategoryTag() = heist_mutator->ExitCraftTag();
	    VanRepData->SpecialActor() = van;
    
	    memcpy(&VanRepData->MainIconBrush(), &spawn_info->CraftMinimapIconBrush(),FSlateBrush::GetStructSize());
	    VanRepData->MinimapScale() = spawn_info->CraftMinimapIconScale();
    
	    memcpy(&VanRepData->CompassIconBrush(),&spawn_info->CraftCompassIconBrush(),FSlateBrush::GetStructSize());
	    VanRepData->CompassScale() = spawn_info->CraftCompassIconScale();
    
	    VanRepData->AddedToClientTime() = reinterpret_cast<UWorld*>(GWorld)->TimeSeconds();
    
	    VanRepData->LastLocationReplicationTime() = 0.0f;
	    VanRepData->PrevLocationReplicatedTime() = 0.0f;
    
	    VanRepData->LastYawReplicationTime() = 0.0f;
	    VanRepData->PrevYawReplicatedTime() = 0.0f;
    
	    VanRepData->LastRepLocation() = FVector{0,0,0};
	    VanRepData->PrevRepLocation() = FVector{0,0,0};
	    VanRepData->LerpStartLocation() = start_location;
    
	    VanRepData->bActorIsRelevant() = true;
    
	    SpecialActorList.Add(VanRepData);
    
	    FSpecialActorRepData* RefoundVanData = nullptr;
	    for (int i = 0; i < SpecialActorList.Num(); ++i)
	    {
	    	FSpecialActorRepData* RepData = &SpecialActorList[i];
    
	    	if (RepData->SpecialActor() == van)
	    	{
	    		RefoundVanData = RepData;
	    		break;
	    	}
	    }

        actorRepList->MarkItemDirty(*RefoundVanData);
        
	    memcpy(VanRepData,RefoundVanData,FSpecialActorRepData::GetStructSize());
    
	    heist_mutator->OnSpecialActorAdded(*SpawnerRepData);
        heist_mutator->OnSpecialActorRemoved(*SpawnerRepData);
    
        static void* special_actor_list = FindPropertyByName(FSpecialActorRepDataArray::GetClass(), L"SpecialActorList");
            
	    SpecialActorList.Remove(SpawnerIndex, special_actor_list);
        
	    actorRepList->MarkArrayDirty();
    
	    FMemory_Free(VanRepData);
    }
}

AFortGameplayMutator* fort_mutators_component::get_mutator_of_class(UObject* mutator_class)
{
	if (mutator_class)
	{
		TArray<AActor*> out_actors = UGameplayStatics::GetAllActorsOfClass(*GWorld, mutator_class);

		for (AActor* out_actor : out_actors)
		{
			return static_cast<AFortGameplayMutator*>(out_actor);
		}	
	}
	// spdlog::error(L"Invalid mutator class/expected mutator was not found in fort_mutators_component::get_mutator_of_class");
	return nullptr;
}

void fort_mutators_component::on_game_phase_updated(uint8_t GamePhase)
{
	if (auto mutator = get_mutator_of_class(AFortAthenaMutator_Barrier::StaticClass()))
	{
		static UEnum* GamePhaseEnum = FindObject<UEnum>(TEXT("/Script/FortniteGame.EAthenaGamePhase"));
		uint8_t SafeZones = GamePhaseEnum->GetEnumIndex(TEXT("SafeZones"));
		uint8_t Aircraft = GamePhaseEnum->GetEnumIndex(TEXT("Aircraft"));
		AFortAthenaMutator_Barrier* barrier_mutator = static_cast<AFortAthenaMutator_Barrier*>(mutator);
		
		if (GamePhase == SafeZones)
		{
			barrier_mutator->BigBaseWall()->TimeUntilWallComesDown() = barrier_mutator->TimeToBringDownWall().GetValueAtLevel(0, nullptr);
			spdlog::info(L"Updating TimeUntilWallComesDown on AFortAthenaMutator_Barrier::BigBaseWall actor.");
		}
		if (GamePhase == Aircraft)
		{
			spdlog::info(L"Aircraft Phase on AFortAthenaMutator_Barrier, spawning Objective Flags");

			barrier_mutator->SpawnBarrierObjective(0);
			barrier_mutator->SpawnBarrierObjective(1);
		}
	}
}
