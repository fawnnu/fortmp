# FortMPServer

Building
------

To build FortMP, you will need:
1. MSVC v143 and c++20 for FortMPClient
2. MSVC v143 and c++20 for FortMPServer

All the libraries and dependancies are included in the solution.

Hosting a Dedicated Server
------

To host a dedicated server, you will need `FortMPLauncher.exe` and its config file. This launcher will patch the newly started Fortnite process to allow any commandline argument.

1. Make sure to have `FortMPLauncher.exe` and `FortMPServer.dll` in the same folder.
2. Set `ServerPath` in `launcher_config.toml` to the path of the Fortnite executable.
3. Open the Launcher.

# Features

Custom Mutators
------

FortMP features a few mutators, that being:

 - "rocket" - Allows the rocket event to be started through backend
 - "cube" - Allows the backend to control cube movement
 - "cube_spawn" - Spawns the cube (if an event is created on backend, it takes the spawn step from there)
 - "birthday_bus" - Changes the bus cosmetic to birthday bus (only works on the birthday version)
 - "winter_bus" - Changes the bus cosmetic to winter bus (only works on the winter version)
 - "siphon" - Enables the siphon mode on kill