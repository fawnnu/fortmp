#include "include.hpp"
#include "gui_manager.hpp"

#include "config_manager.hpp"
#include "shell_component.hpp"
#include "../gui/renderer.hpp"

gui_manager::gui_manager()
{
    auto config = config_manager::get_server_config();

    if (!config["nullrhi"].get<bool>())
    {
        CreateThread(nullptr, 0, [](LPVOID) -> DWORD
        {
            new renderer();
            
            return 0;    
        }, nullptr, 0, nullptr);
    }
    else
    {
        CreateThread(nullptr, 0, [](LPVOID) -> DWORD
        {
            new shell_component();

            return 0;
        }, nullptr, 0, nullptr);
    }
}
