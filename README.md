# FortMP

Building
------

To build FortMP, you will need:
1. MSVC v142 and c++17 for FortMPClient
2. MSVC v143 and c++20 for FortMPServer

All the libraries and dependancies are included in the solution.

Hosting a Dedicated Server
------

To host a dedicated server, you will need `FortMPLauncher.exe` and its config file. This launcher will patch the newly started Fortnite process to allow any commandline argument.

1. Be sure to have `FortMPLauncher.exe` and `FortMPServer.dll` in the same folder.
2. Set `ServerPath` in `launcher_config.toml` to the path of the Fortnite executable.
3. Open the Launcher.