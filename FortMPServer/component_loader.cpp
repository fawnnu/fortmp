#include "include.hpp"
#include "component_loader.hpp"

#include "components/server_component.hpp"

component_loader::component_loader()
{
    // Load game components
    load_component(new server_component());
}

void component_loader::load_component(component* component_to_load)
{
    if (component_to_load)
    {
        // Add to component list
        components.push_back(component_to_load);
        spdlog::info("Loaded server component: {}", component_to_load->component_name.c_str());
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

component* component_loader::find_component_by_name(std::string in_component_name)
{
    for (component* component_it : components)
    {
        if (component_it->component_name == in_component_name)
        {
            return component_it;
        }
    }

    return nullptr;
}
