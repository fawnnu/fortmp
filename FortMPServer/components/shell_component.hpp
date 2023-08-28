#pragma once

typedef void(*EraCommandCB)();

class era_command
{
public:
    era_command(std::string InCommandName, EraCommandCB InCommand);
    
    void Execute();
    EraCommandCB Command;
    std::string CommandName;
};

class shell_component
{
public:
    shell_component();

    static era_command* find_command(std::string Name);
    static inline std::vector<era_command*> commands;
    static inline std::shared_ptr<spdlog::logger> console;
};

FORCEINLINE bool operator==(era_command* Command, std::string Name)
{
    return Command->CommandName == Name;
}

// Commands 
