#include "include.hpp"
#include "bots_component.hpp"

#include "pickup_component.hpp"
#include "looting_component.hpp"

bots_component::bots_component()
{
    REGISTER_PROCESS_EVENT_DETOUR(L"/Script/FortniteGame.FortAthenaAIBotController.OnPossesedPawnDied", bots_component_hooks::OnPossesedPawnDied_Hook);
}

void bots_component::initialize_data()
{
    if (AFortAthenaMutator_Bots::StaticClass())
    {
        mutator_bots = SpawnDefaultActor<AFortAthenaMutator_Bots>(*GWorld, AFortAthenaMutator_Bots::StaticClass());

        if (mutator_bots)
        {
            if (STATIC_HASPROPERTY((*GWorld)->AuthorityGameMode(), ServerBotManager))
            {
                (*GWorld)->AuthorityGameMode()->ServerBotManager() = static_cast<UFortServerBotManagerAthena*>(UGameplayStatics::SpawnObject(UFortServerBotManagerAthena::StaticClass(), (*GWorld)));

                if ((*GWorld)->AuthorityGameMode()->ServerBotManager())
                {
                    STATIC_HASPROPERTY((*GWorld)->AuthorityGameMode()->ServerBotManager(), CachedBotMutator);
                    STATIC_HASPROPERTY((*GWorld)->AuthorityGameMode()->ServerBotManager(), CachedGameMode);
                    STATIC_HASPROPERTY((*GWorld)->AuthorityGameMode()->ServerBotManager(), CachedGameState);

                    if (bExists_CachedBotMutator &&
                        bExists_CachedGameMode &&
                        bExists_CachedGameState)
                    {
                        (*GWorld)->AuthorityGameMode()->ServerBotManager()->CachedBotMutator() = mutator_bots;
                        (*GWorld)->AuthorityGameMode()->ServerBotManager()->CachedGameMode() = (*GWorld)->AuthorityGameMode();
                        (*GWorld)->AuthorityGameMode()->ServerBotManager()->CachedGameState() = (*GWorld)->GameState();
                    
                        // Basic agent spawning example
                        /*FVector spawn_loc = FVector(0, 0, 0);
                        FRotator spawn_rot = {};
                        
                        spawn_agent(static_cast<UFortAthenaAIBotCustomizationData*>(FindObject(L"/Game/Athena/AI/MANG/BotData/BotData_MANG_POI_Agency.BotData_MANG_POI_Agency", true, false)), spawn_loc, spawn_rot);*/
                    }
                }
            }
        }
    }
}

AFortPlayerPawnAthena* bots_component::spawn_agent(UFortAthenaAIBotCustomizationData* customization_data, FVector spawn_location, FRotator spawn_rotation)
{
    if (mutator_bots && customization_data)
    {
        FActorSpawnParameters spawn_parameters = {};
        
        AFortPlayerPawnAthena* bot = mutator_bots->SpawnBot(customization_data->PawnClass(), nullptr, spawn_location, spawn_rotation, true);
        AFortAthenaAIBotController* controller = static_cast<AFortAthenaAIBotController*>(UAIBlueprintHelperLibrary::GetAIController(bot));

        controller->PlayerBotPawn() = bot;

        // Cache
        controller->CachedBotManager() = (*GWorld)->AuthorityGameMode()->ServerBotManager();
        controller->CachedBotMutator() = mutator_bots;
        controller->CachedGameMode() = (*GWorld)->AuthorityGameMode();

        // Inventory
        if(controller->Inventory() == nullptr)
        {
            controller->Inventory() = SpawnDefaultActor<AFortInventory>(*GWorld, FindObject(L"/Script/FortniteGame.FortInventory"));   
        }
        
        controller->StartupInventory() = customization_data->StartupInventory();
        
        for (UFortItemDefinition* item_definition : controller->StartupInventory()->Items())
        {
            auto replicated_entries = TArrayManager<FFortItemEntry>(&controller->Inventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());
            auto item_instances = TArrayManager<UFortWorldItem*>(&controller->Inventory()->Inventory().ItemInstances(), sizeof(UFortWorldItem*));

            // Create a world item from the item definition
            UFortWorldItem* world_item = reinterpret_cast<UFortWorldItem*>(item_definition->CreateTemporaryItemInstanceBP(1, 1));

            if (world_item == nullptr)
            {
                return nullptr;
            }
    
            // Add to inventory
            replicated_entries.Add(&world_item->ItemEntry());
    
            // Update inventory locally
            controller->Inventory()->HandleInventoryLocalUpdate();
    
            // Mark for replication
            controller->Inventory()->Inventory().MarkItemDirty(replicated_entries[replicated_entries.Num() - 1]);
        }

        // Custom visualization
        controller->PlayerBotPawn()->PlayerState()->HeroType() = customization_data->CharacterCustomization()->CustomizationLoadout().Character()->HeroDefinition();
        
        for(auto soft_character_part : customization_data->CharacterCustomization()->CustomizationLoadout().Character()->HeroDefinition()->Specializations()[0]->CharacterParts())
        {
            auto character_part = soft_character_part.Get();
            ServerChoosePart_Implementation(bot, character_part->CharacterPartType(), character_part);
        }
        controller->PlayerBotPawn()->PlayerState()->OnRep_CharacterData();
        
        return bot;
    }
    else
    {
        return nullptr;
    }
}

void bots_component_hooks::OnPossesedPawnDied_Hook(UObject* Object, UObject* Function, void* Params)
{
    auto ai_bot_controller = static_cast<AFortAthenaAIBotController*>(Object);

    if (ai_bot_controller->Inventory())
    {
        auto replicated_entries = TArrayManager<FFortItemEntry>(&ai_bot_controller->Inventory()->Inventory().ReplicatedEntries(), FFortItemEntry::GetStructSize());

        static UEnum* EFortInventoryFilter = FindObject<UEnum>(L"/Script/FortniteGame.EFortInventoryFilter");
    
        for (int i = replicated_entries.Num() - 1; i >= 0; i--)
        {
            FFortItemEntry& item_entry = replicated_entries[i];

            if (UFortItemDefinition* ItemDefinition = item_entry.ItemDefinition())
            {
                if (ItemDefinition->FilterOverride() == EFortInventoryFilter->GetEnumIndex(L"Invisible") || item_entry.Count() < 1)
                {
                    continue;
                }
        
                pickup_component::spawn_pickup(item_entry.ItemDefinition(), ai_bot_controller->PlayerBotPawn()->K2_GetActorLocation(), item_entry.Count(), looting_component::get_default_loaded_ammo(item_entry.ItemDefinition()));
            }
        }
    }
}
