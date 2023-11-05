#include "include.hpp"
#include "customproperties_component.hpp"

customproperties_component::customproperties_component()
{
    if (FMemory_Malloc)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        REGISTER_FUNCTION_DETOUR(customproperties_component_hooks::ConstructUProperty, memory_manager::find_pattern("40 55 53 57 41 54 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 48 8B 05 ? ? ? ? 48 33 C4 48 89 85"), customproperties_component_hooks::ConstructUProperty_Hook);
    
        DetourTransactionCommit();
    }
}

void customproperties_component_hooks::ConstructUProperty_Hook(UObject* NewClass, void*** PropertyArray, int& PropertyNum)
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