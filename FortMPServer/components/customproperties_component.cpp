#include "include.hpp"
#include "customproperties_component.hpp"

#include "director_client.hpp"
#include "player_component.hpp"

customproperties_component::customproperties_component()
{
    if (FMemory_Malloc && ConstructUScriptStruct)
    {
        if (GetEngineVersionEasy() < 423)
        {
            CoreOffsets::StructPropertyParamsOffset = offsetof(UE_419_422_TEMPLATE::FStructPropertyParams, Offset);   
            CREATE_CUSTOM_PROPERTY_CONTAINER("FortPlayerControllerAthena", "/Script/FortniteGame.FortPlayerControllerAthena", EraPlayerInfo, UE_419_422_TEMPLATE);
            CREATE_CUSTOM_PROPERTY_CONTAINER("BuildingContainer", "/Script/FortniteGame.BuildingContainer", EraContainerInfo, UE_419_422_TEMPLATE);
            CREATE_CUSTOM_PROPERTY_CONTAINER("FortPoiVolume", "/Script/FortniteGame.FortPoiVolume", EraPoiInfo, UE_419_422_TEMPLATE);
        }
        else
        {
            CoreOffsets::StructPropertyParamsOffset = offsetof(UE_423_425_TEMPLATE::FStructPropertyParams, Offset);
            CREATE_CUSTOM_PROPERTY_CONTAINER("FortPlayerControllerAthena", "/Script/FortniteGame.FortPlayerControllerAthena", EraPlayerInfo, UE_423_425_TEMPLATE);
            CREATE_CUSTOM_PROPERTY_CONTAINER("BuildingContainer", "/Script/FortniteGame.BuildingContainer", EraContainerInfo, UE_423_425_TEMPLATE);
            CREATE_CUSTOM_PROPERTY_CONTAINER("FortPoiVolume", "/Script/FortniteGame.FortPoiVolume", EraPoiInfo, UE_423_425_TEMPLATE);
        }
    }
    else
    {
        spdlog::critical(L"customproperties_component: failed to initialize. Closing...");
        
        // Unregister the server
        if (gameserver_client::Client)
        {
            gameserver_client::Client->SendLeave();
        }
        
        // Close director connection
        if (director_client::client)
        {
            director_client::client->Close();
        }
            
        // Request exit if the server has not been responsive for more than 5 minutes
        RequestExitWithStatus(true, 0);
    }
}

void customproperties_component::pre_init()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    INIT_POINTER(customproperties_component_hooks::ConstructUProperty, memory_manager::find_pattern("40 55 53 57 41 54 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 48 8B 05 ? ? ? ? 48 33 C4 48 89 85"));
    if (!customproperties_component_hooks::ConstructUProperty)
    {
        INIT_POINTER(customproperties_component_hooks::ConstructUProperty, memory_manager::find_pattern("48 89 5C 24 ? 55 56 57 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 45 33 C9 48 89 54 24 ? 41 8B D9"));
    }
        
    if (customproperties_component_hooks::ConstructUProperty)
    {
        DetourAttach(&reinterpret_cast<PVOID&>(customproperties_component_hooks::ConstructUProperty), customproperties_component_hooks::ConstructUProperty_Hook);
    }
    else
    {
        spdlog::critical("Failed to setup hook '{}', invalid function pointer provided. Closing....", "customproperties_component_hooks::ConstructUProperty");

        // Unregister the server
        if (gameserver_client::Client)
        {
            gameserver_client::Client->SendLeave();
        }
            
        // Close director connection
        if (director_client::client)
        {
            director_client::client->Close();
        }
            
        // Request exit if the server has not been responsive for more than 5 minutes
        RequestExitWithStatus(true, 0);
    }
        
    DetourTransactionCommit();
}

void customproperties_component::wait_for_initialization()
{
    do 
    {
        if (isUnrealInitialized)
        {
            break;
        }
        
        Sleep(1000 / 60);
    } while (true);
}

void customproperties_component_hooks::ConstructUProperty_Hook(UObject* NewClass, void*** PropertyArray, int& PropertyNum)
{
    // Wait for everything to be initialized; block the thread until then.
    customproperties_component::wait_for_initialization();
    
    // Get class name and apply any pending custom property
    if (NewClass)
    {
        std::vector<FCustomPropertyData> CustomPropertiesToApply;
        for (int i = customproperties_component::PendingCustomProperties.size() - 1; i >= 0; i--)
        {
            if (customproperties_component::PendingCustomProperties[i].ClassName == NewClass->GetName())
            {
                // Copy
                CustomPropertiesToApply.push_back(customproperties_component::PendingCustomProperties[i]);

                // Remove pending custom property
                customproperties_component::PendingCustomProperties.erase(customproperties_component::PendingCustomProperties.begin() + i);
            }
        }

        // Create a new PropertyArray
        if (CustomPropertiesToApply.size() > 0)
        {
            void** NewPropertyArray = static_cast<void**>(FMemory_Malloc((PropertyNum + CustomPropertiesToApply.size()) * 8, 8));
            ZeroMemory(NewPropertyArray, (PropertyNum + CustomPropertiesToApply.size()) * 8);
            memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(NewPropertyArray) + CustomPropertiesToApply.size() * 8), *PropertyArray, PropertyNum * 8);
            *PropertyArray = NewPropertyArray;
        }
        
        // Apply properties
        int LocalPropertyIndex = 0;
        for (auto CustomPropertyData : CustomPropertiesToApply)
        {
            PropertyNum++;

            (*PropertyArray)[LocalPropertyIndex++] = CustomPropertyData.ConstructData;

            // Process offsets for different kinds of structs
            if (CustomPropertyData.ConstructData)
            {
                *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(CustomPropertyData.ConstructData) + CoreOffsets::StructPropertyParamsOffset) = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(NewClass) + CoreOffsets::StructSize);
                *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(NewClass) + CoreOffsets::StructSize) += CustomPropertyData.PropertySize;
                
                spdlog::info(L"customproperties_component_hooks::ConstructUProperty_Hook [{}] updated class size [{:x}] for custom property creation [{}].", CustomPropertyData.ClassName.c_str(), *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(NewClass) + CoreOffsets::StructSize), CustomPropertyData.PropertyName.c_str());
            }
            
            spdlog::info(L"Applied property [{}] to {}. PropertyNum: {}", CustomPropertyData.PropertyName.c_str(), CustomPropertyData.ClassName.c_str(), PropertyNum);
        }
    }
    
    // Remove hook
    if (customproperties_component::PendingCustomProperties.size() == 0)
    {
        spdlog::info("Finished applying every custom property. Destroying ConstructUProperty_Hook.");

        // Detach
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourDetach(&reinterpret_cast<PVOID&>(ConstructUProperty), ConstructUProperty_Hook);
        
        DetourTransactionCommit();
    }
    
    ConstructUProperty(NewClass, PropertyArray, PropertyNum);
}
