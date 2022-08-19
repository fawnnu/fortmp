#pragma once

#define WIN32_LEAN_AND_MEAN
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#define JM_XORSTR_DISABLE_AVX_INTRINSICS
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_

#define XOR(STR) xorstr(STR).crypt_get()

#include <Windows.h>
#include <Psapi.h>

#include <spdlog/spdlog.h>
#include <Detours.h>

#include "utils/memory.hpp"
#include "utils/xorstr.hpp"
#include "nlohmann/json.hpp"
#include "httplib/httplib.h"
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"

#include "game/unreal.hpp"
#include "game/sdk.hpp"

#include "utils/hook.hpp"
#include "utils/api_utils.hpp"