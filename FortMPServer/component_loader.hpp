#pragma once

#include <vector>

class component
{
public:
    component() = default;
    ~component() = default;

    std::string component_name;
};

class component_loader
{
public:
    component_loader();

    void load_component(component* component_to_load);
    void unload_component(component* component_to_unload);
    
private:
    std::vector<component*> components;
};