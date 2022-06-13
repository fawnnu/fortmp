#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <Psapi.h>

// Remove 'max' macro
#ifdef max
#undef max
#endif

// Remove 'min' macro
#ifdef min
#undef min
#endif

#pragma warning(pop)
#pragma warning(disable: 4100)

#pragma comment(lib, "opengl32.lib")

#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#include <spdlog/spdlog.h>

#include "component_loader.hpp"
#include "engine_manager.hpp"
#include "memory_manager.hpp"
#include "hook_manager.hpp"

//SDK
#include "sdk/basic.hpp"
#include "sdk/coreuobject.hpp"
#include "sdk/world.hpp"
#include "sdk/engine.hpp"
#include "sdk/gameplaystatics.hpp"
#include "sdk/playercontroller.hpp"
#include "sdk/replicationdriver.hpp"
#include "sdk/netdriver.hpp"
#include "sdk/gamemode.hpp"