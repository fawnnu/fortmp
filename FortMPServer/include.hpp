#pragma once

#define WIN32_LEAN_AND_MEAN
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT

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

#include <spdlog/spdlog.h>
#include <Detours.h>

#include "game/unreal.hpp"
#include "game/sdk.hpp"

#include "utils/memory.hpp"
#include "utils/hook.hpp"