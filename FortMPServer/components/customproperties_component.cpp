#include "include.hpp"
#include "customproperties_component.hpp"

customproperties_component::customproperties_component()
{
    // Register custom properties
    PendingCustomProperties.push_back(FCustomPropertyData(L"FortPlayerControllerAthena", nullptr));
    
    INIT_POINTER(FName_ToString, memory_manager::find_pattern("E8 ? ? ? ? F3 41 0F 10 06 48 8D 15 ? ? ? ? F3 0F 59 05 ? ? ? ? 48 8B CF", true, 1));
    INIT_POINTER(FName_FName, memory_manager::find_function_by_string(L"EnteringMap", { 0xE9 }, 1, false));
    INIT_POINTER(FMemory_Free, memory_manager::find_pattern("E8 ? ? ? ? 48 8B 0F 48 89 0B 4C 89 37 8B 47 08 89 43 08 8B 47 0C 89 43 0C 4C 89 77 08", true, 1));

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    REGISTER_FUNCTION_DETOUR(customproperties_component_hooks::ConstructUProperty, memory_manager::find_pattern("40 55 53 57 41 54 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 48 8B 05 ? ? ? ? 48 33 C4 48 89 85"), customproperties_component_hooks::ConstructUProperty_Hook);
    
    DetourTransactionCommit();
}

void customproperties_component_hooks::ConstructUProperty_Hook(UObject* NewClass, void** PropertyArray, int& PropertyNum)
{
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
        
        // Apply properties
        for (auto CustomPropertyData : CustomPropertiesToApply)
        {
            spdlog::info(L"Applied property to {}. PropertyNum: {}", CustomPropertyData.ClassName.c_str(), PropertyNum);
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