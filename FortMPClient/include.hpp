#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <Psapi.h>

#include <spdlog/spdlog.h>
#include <Detours.h>

#include "utils/memory.hpp"

#include "game/unreal.hpp"
#include "game/sdk.hpp"

#include "utils/hook.hpp"