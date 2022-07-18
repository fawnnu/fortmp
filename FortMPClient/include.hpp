#pragma once

#define WIN32_LEAN_AND_MEAN
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT

#define XOR(STR) xorstr(STR).crypt_get()

#include <Windows.h>
#include <Psapi.h>

#include <spdlog/spdlog.h>
#include <Detours.h>

#include "utils/memory.hpp"
#include "utils/xorstr.hpp"

#include "game/unreal.hpp"
#include "game/sdk.hpp"

#include "utils/hook.hpp"