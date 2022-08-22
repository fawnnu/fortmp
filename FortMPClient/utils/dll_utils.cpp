#include "include.hpp"
#include "dll_utils.hpp"

using namespace std::string_view_literals;

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

const char* dll_utils::get_current_path()
{
    char dll_path[MAX_PATH];

    ZeroMemory(dll_path, sizeof dll_path);

    GetModuleFileNameA(reinterpret_cast<HINSTANCE>(&__ImageBase), dll_path, _countof(dll_path));

    const std::string dll_path_str(dll_path);
    
    return dll_path_str.substr(0, dll_path_str.find_last_of("/\\")).c_str();
}

toml::table dll_utils::get_current_config()
{
    std::string config_path(std::string(get_current_path()) + XOR("/Settings"));

    // Create directory if one doesn't exist
    std::filesystem::create_directories(config_path);
    
    config_path.append(XOR("/FortMPConfig.toml"));

    if (std::filesystem::exists(config_path))
    {
        return toml::parse_file(config_path);
    }

    static constexpr auto default_config = R"(
    [client]
    username="FortMP User"
    backend_address="127.0.0.1"
    backend_port=5555
    gameserver_address="erafn.org"
    gameserver_port=7777)";
    
    toml::table parse_result = toml::parse(default_config);

    // Create toml file automatically if one wasn't found
    std::ofstream config_file;

    config_file.open(config_path);

    config_file << parse_result;
    
    config_file.close();

    return parse_result;
}

std::string dll_utils::get_redirect_url()
{
    auto config = get_current_config();

    std::string redirect_url;
    redirect_url.append(XOR("http://"));
    redirect_url.append(config[XOR("client")][XOR("backend_address")].value_or(XOR("127.0.0.1")));
    redirect_url.append(XOR(":"));
    redirect_url.append(std::to_string(config[XOR("client")][XOR("backend_port")].value_or(5555)));

    return redirect_url;
}

std::string dll_utils::get_local_username()
{
    auto config = get_current_config();
    
    return config[XOR("client")][XOR("username")].value_or(XOR("FortMP User"));
}

std::string dll_utils::get_gameserver_addr()
{
    auto config = get_current_config();
    
    return config[XOR("client")][XOR("gameserver_address")].value_or(XOR("erafn.org"));
}

int dll_utils::get_gameserver_port()
{
    auto config = get_current_config();
    
    return config[XOR("client")][XOR("gameserver_port")].value_or(7777);
}