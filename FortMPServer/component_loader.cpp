#include "include.hpp"
#include "component_loader.hpp"

#include "components/fort_inventory_component.hpp"

component_loader::component_loader()
{
    // Load game components
    //load_component(new fort_inventory_component());
}

void component_loader::load_component(component* component_to_load)
{
    if (component_to_load)
    {
        // Add to component list
        components.push_back(component_to_load);
    }
    else
    {
        // No component exists
        spdlog::error("Failed to load component: {}",
            component_to_load->component_name.c_str());
    }
}

void component_loader::unload_component(component* component_to_unload)
{
    if (component_to_unload)
    {
        for (component* component : components)
        {
            if (component->component_name == component_to_unload->component_name)
            {
                // Disable all hooks
                
                
                // Remove from component list
                components.erase(std::remove(components.begin(), components.end(), component), components.end());

                spdlog::info("Unloaded component: {}",
                    component_to_unload->component_name.c_str());
            }
        }
    }
    else
    {
        // No component exists
        spdlog::error("Failed to unload component: {}",
            component_to_unload->component_name.c_str());
    }
}