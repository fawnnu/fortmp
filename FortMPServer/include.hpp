#pragma once

#define WIN32_LEAN_AND_MEAN
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_

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

#ifdef _STAGING
#define WS_URL std::string("wss://staging.ws.erafn.org") 
#define API_URL std::string("https://staging.api.v1.internal.erafn.org")
#elif
#define WS_URL std::string("wss://ws.erafn.org") 
#define API_URL std::string("https://api.v1.internal.erafn.org") 
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
#include "websocketpp/config/asio_client.hpp"
#include "websocketpp/client.hpp"
#include "components/gameserver_client.hpp"

#include "fstream"
#include "utils/memory.hpp"
#include "utils/dll_utils.hpp"

#include "game/unreal.hpp"
#include "game/sdk.hpp"

#include "utils/hook.hpp"

#include "utils/Containers.hpp"