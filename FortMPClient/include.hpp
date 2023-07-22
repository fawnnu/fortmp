#pragma once

#define WIN32_LEAN_AND_MEAN
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#define JM_XORSTR_DISABLE_AVX_INTRINSICS
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
//#define FMPLOG

#define XOR(STR) xorstr(STR).crypt_get()

#if !defined(NT_SUCCESS)
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

#if !defined(STATUS_SUCCESS)
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#endif

#pragma comment(lib, "openssl/libcrypto.lib")
#pragma comment(lib, "openssl/libssl.lib")

#include <Windows.h>
#include <Psapi.h>
#include <codecvt>
#include "winternl.h"
#include <TlHelp32.h>
#include <filesystem>

#include <spdlog/spdlog.h>
#include <Detours.h>

#include "utils/memory.hpp"
#include "utils/xorstr.hpp"
#include "nlohmann/json.hpp"
#include "toml++/toml.h"
#include "httplib/httplib.h"
#include "websocketpp/config/asio_client.hpp"
#include "websocketpp/client.hpp"

#include "game/unreal.hpp"
#include "game/sdk.hpp"

#include "utils/hook.hpp"
#include "utils/api_utils.hpp"
#include "utils/dll_utils.hpp"
#include "utils/sha1.h"
#include "utils/sha256.h"
#include "utils/md5.h"
#include "utils/base64.hpp"
#include "utils/hmac.hpp"
#include "utils/smbios.hpp"