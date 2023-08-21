#pragma once

// Existing mutator definitions
struct siphon_mutator_params
{
    AFortPlayerController* Target;
    AFortPlayerController* Victim;
};

// typedefs
typedef void(*ExecuteCallback)(void* Params);

class era_mutator
{
public:
    era_mutator() = default;
    era_mutator(std::string in_mutator_name, ExecuteCallback in_callback);
    
    std::string mutator_name;
    ExecuteCallback callback;
};

class cube_mutator : public era_mutator
{
public:
    cube_mutator() = default;
    cube_mutator(std::string in_mutator_name, ExecuteCallback in_callback) :
        era_mutator(in_mutator_name, in_callback)
    {
    }

    inline static int step;
};

// Overload operator
FORCEINLINE bool operator==(era_mutator* Mutator, std::string Name)
{
    return Mutator->mutator_name == Name;
}

class era_mutators_component
{
public:
    era_mutators_component();

    // Parse loaded_mutators
    static void parse_loaded_mutators();

    // Execute
    static void Execute(std::string Name, void* Params);

    // Mutators list
    static inline std::vector<era_mutator*> era_mutators;

    // Active list of mutators
    static inline std::vector<std::string> enabled_era_mutators;
};
