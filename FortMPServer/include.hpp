#pragma once

#define WIN32_LEAN_AND_MEAN
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#define SILENCE_WEBHOOKS

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
#pragma comment(lib, "openssl/libcrypto.lib")
#pragma comment(lib, "openssl/libssl.lib")

#include <spdlog/spdlog.h>
#include <Detours.h>
#include <nlohmann/json.hpp>
#include <httplib/httplib.h>

#include "fstream"
#include "utils/memory.hpp"
#include "utils/dll_utils.hpp"

#include "game/unreal.hpp"
#include "game/sdk.hpp"

#include "utils/hook.hpp"

#include "utils/Containers.hpp"