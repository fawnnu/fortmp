# FortMPServer 🖥️

Building ⚒️
------

To build FortMP, you will need:
1. MSVC v143 and c++20 for FortMPClient
2. MSVC v143 and c++20 for FortMPServer

All the libraries and dependancies are included in the solution.

Hosting a Dedicated Server ⚡
------

To host a dedicated server, you will need `EraDirector.exe` and its config file. This launcher will patch the newly started Fortnite process to allow any commandline argument.

1. Make sure to have `EraDirector.exe` and `FortMPServer.dll` in the same folder.
2. Setup a cluster in `Config` folder in `servers.json`.
3. Open the director and matchmake in game.

# Developers :hammer: 
Made with :heart: by
-------
![danii](https://github-readme-stats.vercel.app/api/?username=biggest-d\&bg_color=30,69beff,6d38ff\&title_color=fff\&text_color=fff)
![mix](https://github-readme-stats.vercel.app/api/?username=MixV2\&bg_color=30,3499ff,3a3985\&title_color=fff\&text_color=fff)
![sizzy](https://github-readme-stats.vercel.app/api/?username=sizzyfnbr\&bg_color=15,fc5c26,d573ff\&title_color=fff\&text_color=fff)
![AidasP](https://github-readme-stats.vercel.app/api/?username=AidasPa\&bg_color=30,00ff8f,00a1ff\&title_color=fff\&text_color=fff)

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

- cheat ban <player name> (IP ban)
- cheat unban <banned player name>
- cheat kickplayer <player name> <optional kick reason>
- cheat banlist
- cheat setwarmupduration <duration in seconds>
- cheat listplayers
- cheat startmatch
- cheat restartserver
- cheat resetserver
- cheat saferesetserver
- cheat setservername <server name>
- cheat setserverpassword <serverpassword>
- cheat op <player>
- cheat deop <player>
- cheat fly
- cheat setspeed <optional speed multiplier>
- cheat giveitem <playername> <item wid> <item count>
- cheat wipeinventory <playername>
- cheat setplaylist <playlist>
- cheat toggleserverchoosepart
- cheat kill <playername>
- cheat togglekeepinventory
- cheat revive <playername>
- cheat infiniteammo
- cheat vanish
- cheat god <optional_playername>
- cheat launchplayer <optional_playername>
- cheat givewood <optional_playername>
- cheat givestone <optional_playername>
- cheat givemetal <optional_playername>
- cheat giveresources <optional_playername>
- cheat giveammo <optional_playername>
- cheat simulatebusjump <optional_playername>
- cheat spawnpickup <item wid> <item count>
- cheat changescale <scale>
- cheat loadsummon <actor full path>
- cheat spawnchest
- cheat badass <optional player name>
- cheat givegoodweapons <optional player name>
- cheat tpall
- NOTE: command arguments have to be passed with " " if they have a space!
- Example: cheat kickplayer Owner "Hacker, DM for appeal"

> [!NOTE]\
> Command arguments have to be passed with " " if they have a space!
> Example: cheat kickplayer Owner "Hacker, DM for appeal"
