#include "include.hpp"
#include "shell_component.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

#include "loader_component.hpp"


era_command::era_command(std::string InCommandName, EraCommandCB InCommand)
{
    Command = InCommand;
    CommandName = InCommandName;

    shell_component::commands.push_back(this);
}

void era_command::Execute()
{
    Command();
}

shell_component::shell_component()
{
    // Register commands
    new era_command("safeshutdown", []()
    {
        loader_component::bPendingShutdown = true;
        console->info("Safe shutdown procedure initialized [bPendingShutdown: true]\n");
    });

    new era_command("help", []()
    {
        console->info("Commands list: 'safeshutdown'\n");
    });
    
    // Allocate console
    AllocConsole();
    FILE *fDummy;
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    console = spdlog::stdout_color_mt("console");
    console->set_pattern("%v");

    // First log
    console->info("FortMP shell software [NULLRHI: enabled]\n");
    
    // Capture input
    std::string InCommand;

    while (std::getline(std::cin, InCommand))
    {
        std::string Name = InCommand.find(" ") != std::string::npos ? InCommand.substr(0, InCommand.find(" ")) : InCommand;

        // Check if command is valid
        if (era_command* Command = find_command(Name))
        {
            Command->Execute();
        }
        else
        {
            console->error("Invalid command [{}].\nType 'help' to see the list of available commands\n", Name.c_str());
        }
    }
}

era_command* shell_component::find_command(std::string InName)
{
    if (const auto Iterator = std::find(commands.begin(), commands.end(), InName); Iterator != commands.end())
    {
        return *Iterator;
    }

    return nullptr;
}